/* cREXX License (MIT). llama.rexx persistent model and VM ownership. */
#include "bridge.h"
#include "package.hpp"
#include "generation_utf8.h"
#include "llama.h"
#include "ggml-backend.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace {
namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;
using rxllama_package::json;
constexpr int64_t MiB = 1024 * 1024;
constexpr const char *engine_id = "5266f24da75dc449bd56cbed7addb9c8e4a6a73e";
struct Failure : std::runtime_error { int code; Failure(int c, const std::string &s): std::runtime_error(s), code(c) {} };
void require(bool yes, const char *why, int code = -1) { if (!yes) throw Failure(code, why); }
struct Config {
    std::map<std::string, int64_t> ints {
        {"memory_bytes", 4096 * MiB}, {"vram_bytes", 4096 * MiB}, {"max_sessions", 8},
        {"threads", 0}, {"batch_threads", 0}, {"context_tokens", 512}, {"request_rows", 8},
        {"request_tokens", 4096}, {"request_bytes", MiB}, {"batch_tokens", 0},
        {"output_tokens", 32}, {"seed", 1234}};
    std::map<std::string, std::string> texts {{"hardware_mode", "auto"}, {"backend", "auto"},
        {"devices", ""}, {"sampler", "greedy"}};
    std::map<std::string, double> floats {{"temperature", 0.0}};
    int64_t i(const char *k) const { return ints.at(k); }
    const std::string &s(const char *k) const { return texts.at(k); }
    void validate() const {
        require(i("context_tokens") <= 8192 && i("request_rows") <= 8 && i("max_sessions") <= 64, "unsupported context/row/session bound");
        require(i("context_tokens") * i("request_rows") <= 65536, "total context bound exceeded");
        require(i("threads") <= 256 && i("batch_threads") <= 256, "thread limit exceeded");
        require(i("batch_tokens") <= 4096 && i("request_tokens") <= 65536, "batch/token bound exceeded");
        require(i("request_tokens") <= i("context_tokens") * i("request_rows"), "request token budget exceeds session capacity");
        require(i("request_bytes") <= 64 * MiB && i("output_tokens") <= i("context_tokens"), "request/output budget invalid");
        require(s("hardware_mode") != "cpu" || (s("backend") == "auto" || s("backend") == "cpu"), "CPU mode conflicts with backend");
        require(s("hardware_mode") != "required-gpu" || s("backend") != "cpu", "required GPU conflicts with CPU backend");
        require((s("hardware_mode") != "cpu" && s("backend") != "cpu") || s("devices").empty(), "CPU selection conflicts with an explicit GPU device");
    }
};
struct Device { ggml_backend_dev_t native; std::string name, backend, description, id, exclusion; size_t available, total; bool gpu; };
std::mutex engine_mutex;
std::vector<Device> inventory;
std::vector<ggml_backend_reg_t> loaded_backends;
// llama.cpp retains backend DSOs for process lifetime because backend threads
// can outlive the registry. Retain feature probes on the same basis; unloading
// and reopening a Mach-O bundle also re-registers its ASan globals (SAN-009).
std::vector<void *> probe_leases;
bool engine_started = false;
std::string discovery_note;
thread_local std::string *placement_log = nullptr;
void inference_log(ggml_log_level, const char *line, void *) {
    // Keep only bounded allocation/offload diagnostics, never metadata/prompts.
    if (!placement_log || !line) return;
    if ((std::strstr(line, "offloaded ") || std::strstr(line, "buffer size")) && placement_log->size() < 4096)
        placement_log->append(line, std::min<size_t>(std::strlen(line), 4096 - placement_log->size()));
}
fs::path engine_directory() {
#ifdef _WIN32
    HMODULE module = nullptr;
    require(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(&rxllama_vm_create), &module) != 0, "cannot locate inference runtime");
    std::vector<wchar_t> path(32768);
    auto n = GetModuleFileNameW(module, path.data(), DWORD(path.size()));
    require(n && n < path.size(), "cannot locate inference runtime");
    return fs::path(std::wstring(path.data(), n)).parent_path();
#else
    Dl_info info{};
    require(dladdr(reinterpret_cast<void *>(&rxllama_vm_create), &info) && info.dli_fname, "cannot locate inference runtime");
    return fs::canonical(info.dli_fname).parent_path();
#endif
}
struct EngineCleanup {
    ~EngineCleanup() {
        if (engine_started) llama_backend_free();
        llama_log_set(nullptr, nullptr); ggml_log_set(nullptr, nullptr);
        // Backends follow upstream process-lifetime DSO ownership; do not unload.
    }
};
int cpu_score(const fs::path &path) {
#ifdef _WIN32
    auto library = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!library) return 0;
    auto score = reinterpret_cast<int (*)()>(GetProcAddress(library, "ggml_backend_score"));
    int value = score ? score() : 1; probe_leases.push_back(library); return value;
#else
    auto library = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!library) return 0;
    auto score = reinterpret_cast<int (*)()>(dlsym(library, "ggml_backend_score"));
    int value = score ? score() : 1; probe_leases.push_back(library); return value;
#endif
}
void initialize_engine() {
    std::lock_guard<std::mutex> lock(engine_mutex);
    if (engine_started) return;
    static EngineCleanup cleanup;
    llama_log_set(inference_log, nullptr);
    ggml_log_set(inference_log, nullptr);
    const auto dir = engine_directory();
    auto manifest = rxllama_package::read(dir / "rxllama.runtime.json");
    require(manifest.at("engine") == engine_id, "inference runtime identity mismatch");
    // Each successful load is retained until process/library shutdown. No
    // environment search or ggml_backend_load_all is ever used.
    json selected = json::array();
    json best_cpu; int best_score = 0;
    size_t cpu_count = 0;
    for (const auto &entry : manifest.at("backends"))
        if (entry.at("backend").get<std::string>().rfind("cpu", 0) == 0) ++cpu_count;
    for (const auto &entry : manifest.at("backends")) {
        const auto name = entry.at("backend").get<std::string>();
        if (name.rfind("cpu", 0) != 0) { selected.push_back(entry); continue; }
        try {
            auto path = rxllama_package::verified(dir, entry);
            int score = cpu_count == 1 ? 1 : cpu_score(path);
            if (score > best_score) { best_cpu = entry; best_score = score; }
        } catch (const std::exception &e) { discovery_note += name + ": " + e.what() + "; "; }
    }
    require(best_score > 0, "no usable verified CPU backend variant", -6);
    selected.insert(selected.begin(), best_cpu);
    for (auto &entry : selected) {
        const auto name = entry.at("backend").get<std::string>();
        try {
            auto file = rxllama_package::verified(dir, entry);
            auto reg = ggml_backend_load(file.u8string().c_str());
            if (!reg) throw Failure(-6, "backend/driver unavailable");
            loaded_backends.push_back(reg);
        } catch (const std::exception &e) {
            discovery_note += name + ": " + e.what() + "; ";
        }
    }
    std::set<std::string> physical;
    for (auto reg : loaded_backends) for (size_t i = 0; i < ggml_backend_reg_dev_count(reg); ++i) {
        auto dev = ggml_backend_reg_dev_get(reg, i);
        ggml_backend_dev_props p{}; ggml_backend_dev_get_props(dev, &p);
        std::string backend = ggml_backend_reg_name(reg);
        std::transform(backend.begin(), backend.end(), backend.begin(), [](unsigned char c){ return char(std::tolower(c)); });
        if (backend == "mtl") backend = "metal";
        bool gpu = p.type == GGML_BACKEND_DEVICE_TYPE_GPU || p.type == GGML_BACKEND_DEVICE_TYPE_IGPU;
        Device d{dev, p.name, backend, p.description, p.device_id ? p.device_id : "", "", p.memory_free, p.memory_total, gpu};
        if (gpu && !d.id.empty() && !physical.insert(d.id).second) d.exclusion = "duplicate physical device; earlier packaged backend preferred";
        inventory.push_back(std::move(d));
    }
    require(std::any_of(inventory.begin(), inventory.end(), [](const Device &d){return !d.gpu && d.backend == "cpu";}), "packaged CPU backend unavailable", -6);

    llama_backend_init(); engine_started = true;
}
struct SharedModel {
    std::mutex mutex;
    std::thread loader;
    std::atomic<bool> cancel{false};
    std::atomic<unsigned> owners{0};
    std::atomic<unsigned> peak_owners{1};
    std::string state = "loading", error, id, hash, profile, backend, device, selection, placement;
    int actual_gpu_layers = 0;
    llama_model *model = nullptr;
    double load_ms = 0;
    int64_t weight_reserve = 0, session_reserve = 0;
    bool gpu = false;
    ~SharedModel() { cancel = true; if (loader.joinable()) loader.join(); if (model) llama_model_free(model); }
};
std::map<std::string, std::weak_ptr<SharedModel>> models;
std::atomic<uint64_t> next_model{1};
std::atomic<unsigned> active_loaders{0};
enum Kind { config_kind, runtime_kind, model_kind, session_kind, request_kind };
struct EmbeddingRow { std::string text; std::vector<llama_token> tokens; };
struct GenerationRow {
    std::string text, partial, finish;
    std::vector<llama_token> tokens;
    size_t read_tokens = 0;
    llama_token next = 0;
    int position = 0;
};
struct GenerationBatch {
    llama_batch value;
    explicit GenerationBatch(int capacity): value(llama_batch_init(capacity, 0, 1)) {}
    ~GenerationBatch() { llama_batch_free(value); }
    void add(llama_token token, int position, int row, bool output) {
        int n = value.n_tokens++; value.token[n] = token; value.pos[n] = position;
        value.n_seq_id[n] = 1; value.seq_id[n][0] = row; value.logits[n] = output;
    }
};
struct Request {
    std::string state = "building", error;
    std::vector<EmbeddingRow> rows;
    std::vector<double> values;
    int64_t bytes = 0, tokens = 0, decode_calls = 0;
    double submit_ms = 0, process_ms = 0;
    bool generation = false;
    std::vector<GenerationRow> generated;
    std::unique_ptr<GenerationBatch> batch;
    size_t prefill_row = 0, prefill_pos = 0, decode_row = 0;
    int64_t output_bytes = 0, last_work_tokens = 0, maximum_work_tokens = 0;
    int64_t prefill_calls = 0, token_calls = 0;
};
struct Resource {
    uint64_t id = 0, parent = 0;
    Kind kind;
    std::atomic<unsigned> references{1};
    bool closed = false, cancelled = false, prepared = false;
    Config config;
    std::string selection;
    std::shared_ptr<SharedModel> model;
    llama_context *context = nullptr;
    int64_t reserved = 0, vram_reserved = 0, sessions = 0, charge = 0, vram_charge = 0;
    double prepare_ms = 0;
    std::vector<llama_token> warm_tokens;
    size_t warm_offset = 0;
    uint64_t active_request = 0;
    int64_t completed_requests = 0;
    std::unique_ptr<Request> request;
    explicit Resource(Kind k): kind(k) {}
    ~Resource() { if (context) llama_free(context); }
};
struct VM {
    uint64_t id, next = 1;
    std::map<uint64_t, std::unique_ptr<Resource>> resources;
    std::string text, operation, message;
    int error = 0;
    bool probes_enabled = false;
    rxllama_glue_probes probes{};
    explicit VM(uint64_t n): id(n) {
#ifdef CREXX_LLAMA_TESTING
        const char *enabled = std::getenv("CREXX_LLAMA_GLUE_PROBES");
        probes_enabled = enabled && std::strcmp(enabled, "1") == 0;
#endif
    }
};
std::mutex vm_mutex;
std::map<uint64_t, VM *> vms;
uint64_t next_vm = 1;
Resource &get(VM &vm, rxllama_token t, int kind = -1, bool allow_closed = false) {
    require(t.owner == vm.id && t.id != 0, "foreign or invalid handle", -2);
    auto it = vm.resources.find(t.id);
    require(it != vm.resources.end(), "stale handle", -2);
    auto &r = *it->second;
    require(kind < 0 || r.kind == kind, "wrong handle kind", -4);
    require(allow_closed || !r.closed, "closed handle", -3);
    return r;
}
Resource &make(VM &vm, Kind kind, rxllama_result &out) {
    auto p = std::make_unique<Resource>(kind); p->id = vm.next++;
    auto &r = *p;
    { std::lock_guard<std::mutex> lock(vm_mutex); vm.resources.emplace(r.id, std::move(p)); }
    out.handle = {vm.id, r.id}; return r;
}
bool has_children(VM &vm, Resource &r) {
    for (auto &pair : vm.resources) if (!pair.second->closed && pair.second->parent == r.id) return true;
    return false;
}
Resource &runtime(VM &vm, Resource &r) {
    if (r.kind == runtime_kind) return r;
    return runtime(vm, *vm.resources.at(r.parent));
}
void close(VM &vm, Resource &r) {
    if (r.closed) return;
    require(!has_children(vm, r), "resource has live children", -7);
    if (r.context) { llama_synchronize(r.context); llama_free(r.context); r.context = nullptr; }
    if (r.kind == request_kind) {
        auto &session = *vm.resources.at(r.parent);
        if (session.active_request == r.id) session.active_request = 0;
        runtime(vm, r).reserved -= r.charge; r.charge = 0;
        r.request.reset();
    }
    if (r.kind == session_kind) r.model.reset();
    if (r.kind == session_kind) { auto &rt = runtime(vm, r); rt.sessions--; rt.reserved -= r.charge; rt.vram_reserved -= r.vram_charge; }
    if (r.kind == model_kind && r.model) {
        bool transferred = false;
        if (r.charge) for (auto &entry : vm.resources) {
            auto &peer = *entry.second;
            if (&peer != &r && !peer.closed && peer.kind == model_kind && peer.parent == r.parent && peer.model == r.model) {
                peer.charge += r.charge; peer.vram_charge += r.vram_charge; transferred = true; break;
            }
        }
        if (!transferred) { runtime(vm, r).reserved -= r.charge; runtime(vm, r).vram_reserved -= r.vram_charge; }
        r.charge = 0; r.vram_charge = 0;
        { std::lock_guard<std::mutex> lock(engine_mutex); if (--r.model->owners == 0) r.model->cancel = true; }
        r.model.reset(); // Join and model free here, outside the RXPA payload lock.
    }
    r.closed = true;
}
void collect(VM &vm) {
    bool again;
    do {
        again = false;
        for (auto it = vm.resources.rbegin(); it != vm.resources.rend(); ++it) {
            auto &r = *it->second;
            if (!r.closed && r.references == 0 && !has_children(vm, r)) { close(vm, r); again = true; }
        }
        std::lock_guard<std::mutex> lock(vm_mutex);
        for (auto it = vm.resources.begin(); it != vm.resources.end();) {
            if (it->second->closed && it->second->references == 0 && !has_children(vm, *it->second)) it = vm.resources.erase(it);
            else ++it;
        }
    } while (again);
}
std::string model_state(Resource &r) {
    if (r.closed) return "closed";
    if (r.cancelled) return "cancelled";
    std::lock_guard<std::mutex> lock(r.model->mutex); return r.model->state;
}
int64_t context_reservation(const Config &c, bool embedding) {
    // Deliberately conservative admission envelope from STEP-02, not a claim
    // that upstream exposes a hard allocator cap. Includes graph/scratch/KV.
    int64_t scale = c.i("context_tokens") * c.i("request_rows");
    return embedding ? 128 * MiB + scale * 32768 : 128 * MiB + scale * 65536;
}
void reserve(Resource &rt, int64_t bytes) {
    require(bytes > 0 && bytes <= rt.config.i("memory_bytes") - rt.reserved, "RAM reservation exceeds runtime budget", -8);
    rt.reserved += bytes;
}
std::shared_ptr<SharedModel> open_model(const Config &c, const fs::path &path, const std::string &hash, const std::string &profile, const std::function<void(const SharedModel *)> &admit, int64_t remaining_vram, std::string &selection) {
    require(hash.size() == 64 && hash.find_first_not_of("0123456789abcdef") == std::string::npos, "expected lowercase SHA-256");
    require(profile == "bge-small-en-v1.5" || profile == "smollm2-360m-instruct", "unqualified model profile");
    bool embedding = profile == "bge-small-en-v1.5";
    int64_t weights = embedding ? 160 * MiB : 800 * MiB;
    int64_t scratch = context_reservation(c, embedding);
    std::lock_guard<std::mutex> lock(engine_mutex);
    const Device *chosen = nullptr;
    selection = "explicit CPU";
    if (c.s("hardware_mode") != "cpu" && c.s("backend") != "cpu") {
        for (const auto &d : inventory) {
            if (!d.gpu || (!d.exclusion.empty() && c.s("backend") == "auto")) continue;
            if (c.s("backend") != "auto" && c.s("backend") != d.backend) continue;
            if (!c.s("devices").empty() && c.s("devices") != d.name && c.s("devices") != d.id) continue;
            size_t available, total; ggml_backend_dev_memory(d.native, &available, &total);
            auto budget = std::min<int64_t>(int64_t(available), std::min(c.i("vram_bytes"), remaining_vram));
            if (weights + scratch <= budget) { chosen = &d; break; }
        }
        if (chosen) selection = "full GPU offload within conservative weight/context reservation; STEP-02 CPU/Metal profile";
        else {
            require(c.s("hardware_mode") != "required-gpu" && c.s("backend") == "auto" && c.s("devices").empty(), "required GPU/backend/device unavailable or exceeds budget", -6);
            selection = "CPU fallback: no eligible packaged GPU within memory budget; " + discovery_note;
        }
    }
    auto key = hash + ":" + profile + ":" + engine_id + ":" + (chosen ? chosen->backend + ":" + chosen->name : "cpu");
    for (auto it = models.begin(); it != models.end();) { if (it->second.expired()) it = models.erase(it); else ++it; }
    if (auto existing = models[key].lock()) {
        std::lock_guard<std::mutex> state(existing->mutex);
        if (!existing->cancel && existing->state != "failed") {
            admit(existing.get()); auto owners = ++existing->owners;
            if (owners > existing->peak_owners) existing->peak_owners = owners;
            return existing;
        }
    }
    auto p = std::make_shared<SharedModel>();
    p->owners = 1; p->hash = hash; p->profile = profile; p->id = std::to_string(next_model++);
    p->gpu = chosen; p->backend = chosen ? chosen->backend : "cpu"; p->device = chosen ? chosen->name : "CPU";
    p->selection = selection; p->weight_reserve = weights; p->session_reserve = scratch;
    auto dev = chosen ? chosen->native : nullptr;
    auto raw = p.get();
    admit(raw);
    require(active_loaders < 4, "model loader admission saturated", -8);
    ++active_loaders;
    try { p->loader = std::thread([raw, path, dev, embedding] {
        struct LoaderSlot { ~LoaderSlot() { --active_loaders; } } slot;
        auto started = Clock::now();
        try {
            // Hash the actual bytes before passing the same canonical path to
            // upstream. Models are application-owned immutable provisioning.
            require(rxllama_package::hash(path, &raw->cancel) == raw->hash, "model artifact SHA-256 mismatch");
            const char *expected = embedding ? "f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999" : "48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201";
            require(raw->hash == expected, "artifact is not the pinned model profile");
            require(!raw->cancel, "model load cancelled");
            auto params = llama_model_default_params();
            ggml_backend_dev_t devices[] = {dev, nullptr};
            params.devices = devices; params.n_gpu_layers = dev ? 999 : 0;
            params.load_mode = LLAMA_LOAD_MODE_AUTO; params.progress_callback = [](float, void *v){return !static_cast<SharedModel *>(v)->cancel.load();};
            params.progress_callback_user_data = raw;
            std::string placement;
            placement_log = &placement;
            auto model = llama_model_load_from_file(path.u8string().c_str(), params);
            placement_log = nullptr;
            require(model != nullptr, "upstream model load failed");
            if (llama_model_n_embd_out(model) != (embedding ? 384 : 960)) { llama_model_free(model); throw Failure(-1, "model dimension differs from profile"); }
            std::lock_guard<std::mutex> state(raw->mutex);
            raw->model = model; raw->placement = placement;
            auto offload = placement.find("offloaded ");
            if (offload != std::string::npos) raw->actual_gpu_layers = std::atoi(placement.c_str() + offload + 10);
            require(!raw->gpu || raw->actual_gpu_layers > 0, "required GPU did not offload model layers");
            raw->load_ms = std::chrono::duration<double, std::milli>(Clock::now() - started).count();
            raw->state = raw->cancel ? "cancelled" : "ready";
        } catch (const std::exception &e) {
            placement_log = nullptr;
            std::lock_guard<std::mutex> state(raw->mutex); raw->error = e.what(); raw->state = raw->cancel ? "cancelled" : "failed";
        } catch (...) { std::lock_guard<std::mutex> state(raw->mutex); raw->error = "unexpected native load failure"; raw->state = "failed"; }
    }); } catch (...) { --active_loaders; throw; }
    models[key] = p; return p;
}
void session_open(VM &vm, Resource &m, const Config &config, const std::string &capability, rxllama_result &out) {
    require(model_state(m) == "ready", "model is not ready", -7);
    bool embedding = m.model->profile == "bge-small-en-v1.5";
    require(capability == (embedding ? "embedding" : "generation"), "model capability mismatch");
    require(!embedding || config.i("context_tokens") <= 512, "BGE context exceeds 512 tokens");
    require(config.i("context_tokens") >= 256 && config.i("context_tokens") % 256 == 0, "context must be a supported multiple of 256 tokens");
    require(config.s("hardware_mode") != "cpu" || !m.model->gpu, "session CPU setting conflicts with loaded model");
    require(config.s("hardware_mode") != "required-gpu" || m.model->gpu, "session GPU setting conflicts with loaded model");
    require(config.s("backend") == "auto" || config.s("backend") == m.model->backend, "session backend conflicts with loaded model");
    require(config.s("devices").empty() || std::any_of(inventory.begin(), inventory.end(), [&](const Device &d) {
        return d.gpu && d.backend == m.model->backend && d.name == m.model->device &&
               (config.s("devices") == d.name || (!d.id.empty() && config.s("devices") == d.id));
    }), "session device conflicts with loaded model");
    require(config.i("batch_tokens") == 0 || (embedding ? config.i("batch_tokens") >= config.i("request_tokens") : config.i("batch_tokens") <= 128), "unsupported physical batch budget");
    auto &rt = runtime(vm, m);
    require(rt.sessions < rt.config.i("max_sessions"), "session admission saturated", -8);
    Config snapshot = config;
    auto selection = m.selection;
    auto charge = context_reservation(config, embedding);
    if (m.model->gpu) require(charge <= std::min(rt.config.i("vram_bytes"), config.i("vram_bytes")) - rt.vram_reserved, "VRAM session reservation exceeds budget", -8);
    reserve(rt, charge);
    try {
        auto &r = make(vm, session_kind, out); r.parent = m.id; r.config = std::move(snapshot); r.charge = charge; rt.sessions++;
        r.selection = std::move(selection);
        r.model = m.model;
        if (r.model->gpu) { r.vram_charge = charge; rt.vram_reserved += charge; }
        // Context retains immutable weights; parent enforces close ordering.
    } catch (...) { rt.reserved -= charge; throw; }
}
void prepare(Resource &r, int64_t work) {
    require(work > 0, "preparation work budget must be positive");
    if (r.prepared) return;
    auto begin = Clock::now();
    bool embedding = r.model->profile == "bge-small-en-v1.5";
    if (!r.context) {
        auto p = llama_context_default_params();
        p.n_ctx = uint32_t(r.config.i("context_tokens") * r.config.i("request_rows"));
        p.n_seq_max = uint32_t(r.config.i("request_rows"));
        p.n_batch = embedding ? uint32_t(r.config.i("request_tokens")) : 512;
        p.n_ubatch = embedding ? p.n_batch : 128;
        if (r.config.i("batch_tokens")) p.n_ubatch = uint32_t(r.config.i("batch_tokens"));
        p.n_threads = int32_t(r.config.i("threads"));
        if (!p.n_threads) p.n_threads = embedding || r.model->gpu ? 2 : 4;
        p.n_threads_batch = r.config.i("batch_threads") ? int32_t(r.config.i("batch_threads")) : p.n_threads;
        p.embeddings = embedding; p.pooling_type = embedding ? LLAMA_POOLING_TYPE_CLS : LLAMA_POOLING_TYPE_NONE;
        if (!embedding) { p.n_outputs_max = p.n_seq_max; p.n_outputs_max_per_seq = 1; }
        p.offload_kqv = r.model->gpu; p.op_offload = r.model->gpu; p.no_perf = false;
        r.context = llama_init_from_model(r.model->model, p);
        require(r.context != nullptr, "upstream context allocation failed", -8);
        require(llama_n_ctx_seq(r.context) == r.config.i("context_tokens"), "upstream changed the requested context size", -6);
        r.prepare_ms += std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
        return; // Context/graph allocation is an explicit preparation unit.
    }
    if (r.warm_tokens.empty()) {
        const auto vocab = llama_model_get_vocab(r.model->model);
        const std::string text = "Warm up.";
        int n = -llama_tokenize(vocab, text.data(), int(text.size()), nullptr, 0, true, false);
        require(n > 0 && n <= 512, "preparation tokenizer failed");
        r.warm_tokens.resize(size_t(n));
        require(llama_tokenize(vocab, text.data(), int(text.size()), r.warm_tokens.data(), n, true, false) == n, "preparation tokenization failed");
    }
    auto remaining = r.warm_tokens.size() - r.warm_offset;
    auto count = embedding ? remaining : std::min<size_t>(remaining, size_t(std::min<int64_t>(work, 128)));
    auto batch = llama_batch_init(int(count), 0, 1);
    for (size_t i = 0; i < count; ++i) {
        auto position = r.warm_offset + i;
        batch.token[i] = r.warm_tokens[position]; batch.pos[i] = int(position); batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0; batch.logits[i] = embedding || position + 1 == r.warm_tokens.size();
    }
    batch.n_tokens = int(count);
    auto status = llama_decode(r.context, batch); llama_batch_free(batch);
    llama_synchronize(r.context);
    require(status == 0, "preparation inference failed", -6);
    r.warm_offset += count;
    if (r.warm_offset < r.warm_tokens.size()) {
        r.prepare_ms += std::chrono::duration<double, std::milli>(Clock::now() - begin).count(); return;
    }
    if (embedding) {
        auto values = llama_get_embeddings_seq(r.context, 0);
        require(values != nullptr, "preparation produced no embedding", -6);
        double norm = 0; for (int i = 0; i < 384; ++i) { require(std::isfinite(values[i]), "nonfinite preparation embedding"); norm += double(values[i]) * values[i]; }
        require(norm > 0, "zero preparation embedding");
    } else require(llama_get_logits_ith(r.context, -1) != nullptr, "preparation produced no logits", -6);
    if (auto memory = llama_get_memory(r.context)) llama_memory_clear(memory, true);
    r.warm_tokens.clear(); r.prepared = true; r.prepare_ms += std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
}
// Requests retain owned bounded input/results; only their session owns compute state.
void request_open(VM &vm, Resource &session, const Config &config, rxllama_result &out) {
    require(session.prepared, "session is not prepared", -7);
    bool generation = session.model->profile == "smollm2-360m-instruct";
    require(!session.active_request, "session has an active request", -7);
    config.validate();
    for (const auto &entry : config.ints) {
        bool limit = entry.first == "request_rows" || entry.first == "request_tokens" || entry.first == "request_bytes" ||
                     (generation && entry.first == "output_tokens");
        auto previous = session.config.ints.at(entry.first);
        require(limit ? entry.second <= previous : entry.second == previous, "request option conflicts with prepared session");
    }
    require(config.texts == session.config.texts && config.floats == session.config.floats,
            "request option conflicts with prepared session");
    Config snapshot = config;
    auto request = std::make_unique<Request>(); request->generation = generation;
    auto charge = config.i("request_bytes") + config.i("request_tokens") * int64_t(sizeof(llama_token)) +
                  config.i("request_rows") * (generation ? config.i("output_tokens") * int64_t(sizeof(llama_token)) + 256
                                                        : 384 * int64_t(sizeof(double)) + 256);
    if (generation) charge += 2 * config.i("request_bytes");
    auto &rt = runtime(vm, session); reserve(rt, charge);
    try {
        auto &r = make(vm, request_kind, out); r.parent = session.id;
        r.config = std::move(snapshot); r.request = std::move(request); r.charge = charge;
        session.active_request = r.id;
    } catch (...) { rt.reserved -= charge; throw; }
}
void valid_utf8(const std::string &text) {
    // The RXPA text boundary is UTF-8. Validate before handing bytes to a tokenizer.
    size_t index = 0;
    while (index < text.size()) {
        auto first = static_cast<unsigned char>(text[index++]);
        if (first < 0x80) continue;
        unsigned count, scalar, minimum;
        if (first >= 0xc2 && first <= 0xdf) { count = 1; scalar = first & 31; minimum = 0x80; }
        else if (first >= 0xe0 && first <= 0xef) { count = 2; scalar = first & 15; minimum = 0x800; }
        else if (first >= 0xf0 && first <= 0xf4) { count = 3; scalar = first & 7; minimum = 0x10000; }
        else throw Failure(-1, "invalid UTF-8 input");
        require(count <= text.size() - index, "incomplete UTF-8 input");
        while (count--) {
            auto next = static_cast<unsigned char>(text[index++]);
            require((next & 0xc0) == 0x80, "invalid UTF-8 continuation"); scalar = (scalar << 6) | (next & 63);
        }
        require(scalar >= minimum && scalar <= 0x10ffff && (scalar < 0xd800 || scalar > 0xdfff), "invalid UTF-8 scalar");
    }
}
std::string selector(const rxllama_argument &arg) {
    require(arg.text && !std::memchr(arg.text, 0, arg.text_length), "embedded NUL or missing selector/path");
    return std::string(arg.text, arg.text_length);
}
void add_embedding(Resource &r, const char *text, size_t length, const std::string &role, rxllama_result &out) {
    auto &q = *r.request;
    require(q.state == "building", "request is not building", -7);
    require(!q.generation, "embedding operation on a generation request", -4);
    require(text, "missing embedding input");
    require(role == "query" || role == "document", "unsupported embedding role");
    require(q.rows.size() < size_t(r.config.i("request_rows")), "request row limit exceeded", -8);
    const char *prefix = role == "query" ? "Represent this sentence for searching relevant passages: " : "";
    auto prefix_length = std::strlen(prefix);
    require(length <= size_t(r.config.i("request_bytes") - q.bytes) &&
            prefix_length <= size_t(r.config.i("request_bytes") - q.bytes) - length, "request byte limit exceeded", -8);
    EmbeddingRow row; row.text = prefix; row.text.append(text, length); valid_utf8(row.text);
    q.rows.push_back(std::move(row)); q.bytes += int64_t(length + prefix_length); out.integer = int64_t(q.rows.size());
}
void release_active(VM &vm, Resource &r) {
    auto &session = *vm.resources.at(r.parent);
    if (session.active_request == r.id) session.active_request = 0;
}
void submit(VM &vm, Resource &r) {
    auto &q = *r.request;
    require(q.state == "building", "request is not building", -7);
    auto begin = Clock::now();
    try {
        require(!q.rows.empty(), "empty request batch");
        if (q.generation) q.generated.resize(q.rows.size());
        auto &session = *vm.resources.at(r.parent);
        const auto vocab = llama_model_get_vocab(session.model->model);
        int64_t total = 0;
        for (auto &row : q.rows) {
            int count = llama_tokenize(vocab, row.text.data(), int(row.text.size()), nullptr, 0, true, q.generation);
            require(count < 0 && count != INT32_MIN, "tokenizer size failed", -6); count = -count;
            require(q.generation ? count <= r.config.i("context_tokens") - r.config.i("output_tokens")
                                 : count <= r.config.i("context_tokens") && count <= 512,
                    "request row exceeds context/token limit");
            require(count <= r.config.i("request_tokens") - total, "request token limit exceeded", -8);
            row.tokens.resize(size_t(count));
            require(llama_tokenize(vocab, row.text.data(), int(row.text.size()), row.tokens.data(), count, true, q.generation) == count,
                    "tokenization failed", -6);
            total += count;
        }
        if (q.generation) for (size_t row = 0; row < q.rows.size(); ++row) {
            q.generated[row].position = int(q.rows[row].tokens.size());
            q.generated[row].tokens.reserve(size_t(r.config.i("output_tokens")));
        }
        q.tokens = total; q.state = "running";
    } catch (const std::exception &e) {
        q.state = "failed"; q.error = e.what();
        for (auto &row : q.generated) row.finish = "error";
        release_active(vm, r); throw;
    }
    auto finished = Clock::now();
    q.submit_ms = std::chrono::duration<double, std::milli>(finished - begin).count();
    if (vm.probes_enabled) vm.probes.submit_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(finished - begin).count();
}
void process_embedding(VM &vm, Resource &r, int64_t work) {
    auto &q = *r.request;
    require(work > 0, "processing work budget must be positive");
    if (q.state == "cancelled" || q.state == "complete") return;
    require(q.state == "running", "request is not running", -7);
    auto &session = *vm.resources.at(r.parent);
    auto begin = Clock::now();
    try {
        std::vector<double> output(q.rows.size() * 384);
        struct Batch {
            llama_batch value;
            explicit Batch(int count): value(llama_batch_init(count, 0, 1)) {}
            ~Batch() { llama_batch_free(value); }
        } batch{int(q.tokens)};
        for (size_t row = 0; row < q.rows.size(); ++row) {
            for (size_t pos = 0; pos < q.rows[row].tokens.size(); ++pos) {
                auto i = batch.value.n_tokens++;
                batch.value.token[i] = q.rows[row].tokens[pos]; batch.value.pos[i] = int(pos);
                batch.value.n_seq_id[i] = 1; batch.value.seq_id[i][0] = int(row); batch.value.logits[i] = true;
            }
        }
        if (auto memory = llama_get_memory(session.context)) llama_memory_clear(memory, true);
        ++q.decode_calls;
        // BERT attention is noncausal: the entire admitted batch is one bounded unit.
        auto decode_begin = vm.probes_enabled ? Clock::now() : Clock::time_point{};
        int status = llama_decode(session.context, batch.value); llama_synchronize(session.context);
        auto decode_end = vm.probes_enabled ? Clock::now() : Clock::time_point{};
        require(status == 0, "embedding inference failed", -6);
        for (size_t row = 0; row < q.rows.size(); ++row) {
            const auto values = llama_get_embeddings_seq(session.context, int(row));
            require(values != nullptr, "missing sequence embedding", -6);
            double norm = 0;
            for (int d = 0; d < 384; ++d) { require(std::isfinite(values[d]), "nonfinite embedding", -6); norm += double(values[d]) * values[d]; }
            require(norm > 0 && std::isfinite(norm), "invalid embedding norm", -6);
            norm = std::sqrt(norm);
            for (int d = 0; d < 384; ++d) output[row * 384 + size_t(d)] = values[d] / norm;
        }
        if (vm.probes_enabled) {
            auto normalized = Clock::now();
            vm.probes.setup_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(decode_begin - begin).count();
            vm.probes.decode_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(decode_end - decode_begin).count();
            vm.probes.normalize_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(normalized - decode_end).count();
            ++vm.probes.requests;
        }
        q.values = std::move(output); q.state = "complete"; ++session.completed_requests;
        release_active(vm, r);
    } catch (const std::exception &e) {
        llama_synchronize(session.context); q.state = "failed"; q.error = e.what(); q.values.clear(); release_active(vm, r); throw;
    }
    q.process_ms = std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
}
#include "generation.h"
const char *operations[] = {"configcreate","configint","configfloat","configtext","runtimeopen","devicecount","deviceinfo","modelopen","modelstate","modelcancel","sessionopen","prepare","infotext","infoint","diagnostic","close","requestopen","addembedding","submit","process","embeddings","cancel","addprompt","readtext"};
}
extern "C" {
#ifdef CREXX_LLAMA_TESTING
LLAMA_BRIDGE_API int rxllama_test_probe_cycle(void) {
    try {
        auto root = engine_directory();
        auto manifest = rxllama_package::read(root / "rxllama.runtime.json");
        for (const auto &entry : manifest.at("backends")) {
            if (entry.at("backend").get<std::string>().rfind("cpu",0) != 0) continue;
            auto path = rxllama_package::verified(root, entry);
            require(cpu_score(path) > 0 && cpu_score(path) > 0, "CPU probe cycle failed");
            initialize_engine(); return 0;
        }
    } catch (...) { return 1; }
    return 1;
}
#endif
uint64_t rxllama_probe_clock_ns(void) {
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count());
}
void *rxllama_vm_create(void) {
    try { std::lock_guard<std::mutex> lock(vm_mutex); auto p = std::make_unique<VM>(next_vm++); vms.emplace(p->id, p.get()); return p.release(); } catch (...) { return nullptr; }
}
int rxllama_retain(rxllama_token t) {
    std::lock_guard<std::mutex> lock(vm_mutex);
    auto vm = vms.find(t.owner); if (vm == vms.end()) return 0;
    auto it = vm->second->resources.find(t.id); if (it == vm->second->resources.end()) return 0;
    ++it->second->references; return 1;
}
void rxllama_release(rxllama_token t) {
    std::lock_guard<std::mutex> lock(vm_mutex);
    auto vm = vms.find(t.owner); if (vm == vms.end()) return;
    auto it = vm->second->resources.find(t.id);
    if (it != vm->second->resources.end() && it->second->references) --it->second->references;
}
void rxllama_vm_destroy(void *opaque) {
    if (!opaque) return;
    auto &vm = *static_cast<VM *>(opaque);
    { std::lock_guard<std::mutex> lock(vm_mutex); vms.erase(vm.id); }
    { std::lock_guard<std::mutex> lock(engine_mutex); for (auto &entry : vm.resources) if (entry.second->kind == model_kind && entry.second->model && entry.second->model->owners == 1) entry.second->model->cancel = true; }
    for (auto it = vm.resources.rbegin(); it != vm.resources.rend(); ++it) { try { close(vm, *it->second); } catch (...) {} }
    delete &vm;
}
int rxllama_call(void *opaque, int op, const rxllama_argument *a, rxllama_result *out) {
    if (!opaque || !a || !out || op < 0 || op > RXLLAMA_READ_TEXT) return -2;
    auto &vm = *static_cast<VM *>(opaque);
    out->probes = nullptr;
    if (op == RXLLAMA_DIAGNOSTIC) { out->integer = vm.error; out->operation = vm.operation.c_str(); out->message = vm.message.c_str(); return 0; }
    vm.error = 0; vm.operation = operations[op]; vm.message.clear(); vm.text.clear();
    try {
        collect(vm);
        switch (op) {
        case RXLLAMA_CONFIG_CREATE: make(vm, config_kind, *out); break;
        case RXLLAMA_CONFIG_INT: case RXLLAMA_CONFIG_FLOAT: case RXLLAMA_CONFIG_TEXT: {
            auto &r = get(vm, a[0].handle, config_kind); auto c = r.config; std::string key(selector(a[1]));
            if (op == RXLLAMA_CONFIG_INT) {
                require(c.ints.count(key), "unknown integer option");
                require(a[2].integer >= 0 && (a[2].integer > 0 || key == "seed" || key == "threads" || key == "batch_threads" || key == "batch_tokens"), "invalid integer option");
                require(a[2].integer <= (INT64_C(1) << 40), "integer option exceeds supported range");
                c.ints[key] = a[2].integer;
            } else if (op == RXLLAMA_CONFIG_FLOAT) {
                require(c.floats.count(key) && a[2].number == 0.0, "only greedy temperature zero is qualified"); c.floats[key] = a[2].number;
            } else {
                require(c.texts.count(key), "unknown text option"); std::string value(selector(a[2]));
                if (key == "hardware_mode") require(value == "auto" || value == "cpu" || value == "required-gpu", "unknown hardware mode");
                if (key == "backend") require(value == "auto" || value == "cpu" || value == "metal" || value == "cuda" || value == "vulkan", "unknown backend");
                if (key == "sampler") require(value == "greedy", "unqualified sampler");
                if (key == "devices") require(value.find(',') == std::string::npos, "multiple device placement awaits qualification");
                c.texts[key] = value;
            }
            c.validate(); r.config = std::move(c); break;
        }
        case RXLLAMA_RUNTIME_OPEN: {
            auto c = get(vm, a[0].handle, config_kind).config; c.validate(); initialize_engine();
            auto &r = make(vm, runtime_kind, *out); r.config = std::move(c); break;
        }
        case RXLLAMA_DEVICE_COUNT: get(vm, a[0].handle, runtime_kind); out->integer = int64_t(inventory.size()); break;
        case RXLLAMA_DEVICE_INFO: {
            get(vm, a[0].handle, runtime_kind); require(a[1].integer > 0 && size_t(a[1].integer) <= inventory.size(), "device index out of range");
            const auto &d = inventory[size_t(a[1].integer - 1)]; std::string key(selector(a[2]));
            if (key == "name") vm.text = d.name; else if (key == "backend") vm.text = d.backend;
            else if (key == "description") vm.text = d.description; else if (key == "device_id") vm.text = d.id;
            else if (key == "exclusion") vm.text = d.exclusion; else if (key == "memory_free") vm.text = std::to_string(d.available);
            else if (key == "memory_total") vm.text = std::to_string(d.total); else if (key == "type") vm.text = d.gpu ? "gpu" : "cpu";
            else throw Failure(-1, "unknown device property"); break;
        }
        case RXLLAMA_MODEL_OPEN: {
            auto &rt = get(vm, a[0].handle, runtime_kind); auto c = get(vm, a[4].handle, config_kind).config; c.validate();
            require(c.i("memory_bytes") <= rt.config.i("memory_bytes"), "model budget exceeds runtime budget");
            int64_t charge = 0, vram_charge = 0;
            auto admission = [&](const SharedModel *shared) {
                for (const auto &entry : vm.resources) {
                    const auto &peer = *entry.second;
                    if (!peer.closed && peer.kind == model_kind && peer.parent == rt.id && peer.model.get() == shared) return;
                }
                auto required = shared->weight_reserve;
                if (shared->gpu) require(required <= std::min(rt.config.i("vram_bytes"), c.i("vram_bytes")) - rt.vram_reserved, "VRAM model reservation exceeds budget", -8);
                reserve(rt, required); charge = required;
                if (shared->gpu) { vram_charge = required; rt.vram_reserved += required; }
            };
            std::shared_ptr<SharedModel> model;
            std::string selection;
            try {
                model = open_model(c, fs::u8path(selector(a[1])), selector(a[2]), selector(a[3]), admission, rt.config.i("vram_bytes") - rt.vram_reserved, selection);
                auto &r = make(vm, model_kind, *out); r.parent = rt.id; r.config = std::move(c); r.model = std::move(model); r.charge = charge; r.vram_charge = vram_charge;
                r.selection = std::move(selection);
            } catch (...) {
                rt.reserved -= charge; rt.vram_reserved -= vram_charge;
                if (model) { std::lock_guard<std::mutex> lock(engine_mutex); if (--model->owners == 0) model->cancel = true; }
                throw;
            } break;
        }
        case RXLLAMA_MODEL_STATE: vm.text = model_state(get(vm, a[0].handle, model_kind, true)); break;
        case RXLLAMA_MODEL_CANCEL: { auto &r = get(vm, a[0].handle, model_kind); require(!has_children(vm, r), "model has live sessions", -7); r.cancelled = true; { std::lock_guard<std::mutex> lock(engine_mutex); if (r.model->owners == 1) r.model->cancel = true; } break; }
        case RXLLAMA_SESSION_OPEN: { auto &m = get(vm, a[0].handle, model_kind); auto c = get(vm, a[2].handle, config_kind).config; session_open(vm, m, c, selector(a[1]), *out); break; }
        case RXLLAMA_PREPARE: { auto &r = get(vm, a[0].handle, session_kind); prepare(r, a[1].integer); vm.text = r.prepared ? "ready" : "preparing"; break; }
        case RXLLAMA_REQUEST_OPEN: request_open(vm, get(vm, a[0].handle, session_kind), get(vm, a[1].handle, config_kind).config, *out); break;
        case RXLLAMA_ADD_EMBEDDING: {
            auto begin = vm.probes_enabled ? rxllama_probe_clock_ns() : 0;
            add_embedding(get(vm, a[0].handle, request_kind), a[1].text, a[1].text_length, selector(a[2]), *out);
            if (vm.probes_enabled) vm.probes.admit_ns += rxllama_probe_clock_ns() - begin;
            break;
        }
        case RXLLAMA_ADD_PROMPT: add_prompt(get(vm, a[0].handle, request_kind), a[1], a[2], *out); break;
        case RXLLAMA_READ_TEXT: read_text(vm, get(vm, a[0].handle, request_kind), a[1].integer, *out); break;
        case RXLLAMA_SUBMIT: submit(vm, get(vm, a[0].handle, request_kind)); break;
        case RXLLAMA_PROCESS: {
            auto &r = get(vm, a[0].handle, request_kind);
            if (r.request->generation) process_generation(vm, r, a[1].integer);
            else process_embedding(vm, r, a[1].integer);
            vm.text = r.request->state; break;
        }
        case RXLLAMA_EMBEDDINGS: {
            auto &r = get(vm, a[0].handle, request_kind); auto &q = *r.request;
            require(!q.generation, "embedding operation on a generation request", -4);
            require(q.state == "complete", "embedding result is not complete", -7);
            out->probes = vm.probes_enabled ? &vm.probes : nullptr;
            out->values = q.values.data(); out->value_count = int64_t(q.values.size());
            out->rows = int64_t(q.rows.size()); out->dimensions = 384; break;
        }
        case RXLLAMA_CANCEL: {
            auto &r = get(vm, a[0].handle, request_kind); auto &q = *r.request;
            if (q.state == "building" || q.state == "running") {
                q.state = "cancelled"; q.values.clear();
                if (q.generation && q.generated.empty()) q.generated.resize(q.rows.size());
                for (auto &row : q.generated) if (row.finish.empty()) { row.finish = "cancelled"; row.partial.clear(); }
                release_active(vm, r);
            }
            break;
        }
        case RXLLAMA_CLOSE: close(vm, get(vm, a[0].handle, -1, true)); break;
        case RXLLAMA_INFO_TEXT: case RXLLAMA_INFO_INT: {
            auto &r = get(vm, a[0].handle, -1, true); std::string key(selector(a[1]));
            if (op == RXLLAMA_INFO_TEXT) {
                if (key == "runtime_build") vm.text = std::string(engine_id) + ":" + CREXX_LLAMA_BUILD_ID;
                else if (key == "state") vm.text = r.closed ? "closed" : r.kind == model_kind ? model_state(r) : r.request ? r.request->state : r.prepared ? "ready" : "open";
                else if (r.request && key == "error") vm.text = r.request->error;
                else if (r.model) {
                    auto &m = *r.model; std::lock_guard<std::mutex> lock(m.mutex);
                    if (key == "shared_model_id") vm.text = m.id; else if (key == "sha256") vm.text = m.hash;
                    else if (key == "profile") vm.text = m.profile; else if (key == "backend") vm.text = m.backend;
                    else if (key == "device") vm.text = m.device; else if (key == "selection") vm.text = r.selection;
                    else if (key == "error") vm.text = m.error;
                    else if (key == "placement") vm.text = m.placement;
                    else if (key == "generation_spec" && m.profile == "smollm2-360m-instruct") vm.text = "smollm2-360m-instruct;Q8_0;greedy;explicit-system-user-template;bounded-prefill-decode;UTF-8";
                    else if (key == "embedding_spec" && m.profile == "bge-small-en-v1.5") vm.text = "bge-small-en-v1.5;F16;384;CLS;L2;512;query=Represent this sentence for searching relevant passages: ;document=unchanged";
                    else throw Failure(-1, "unknown model text property");
                } else throw Failure(-1, "unknown resource text property");
            } else {
                if (key.rfind("probe_", 0) == 0 && r.kind == runtime_kind) {
                    require(vm.probes_enabled, "glue probes are not enabled");
                    if (key == "probe_admit_ns") out->integer = int64_t(vm.probes.admit_ns);
                    else if (key == "probe_submit_ns") out->integer = int64_t(vm.probes.submit_ns);
                    else if (key == "probe_setup_ns") out->integer = int64_t(vm.probes.setup_ns);
                    else if (key == "probe_decode_ns") out->integer = int64_t(vm.probes.decode_ns);
                    else if (key == "probe_normalize_ns") out->integer = int64_t(vm.probes.normalize_ns);
                    else if (key == "probe_copy_ns") out->integer = int64_t(vm.probes.copy_ns);
                    else if (key == "probe_copies") out->integer = int64_t(vm.probes.copies);
                    else if (key == "probe_bytes") out->integer = int64_t(vm.probes.bytes);
                    else if (key == "probe_requests") out->integer = int64_t(vm.probes.requests);
                    else throw Failure(-1, "unknown private glue probe");
                }
                else if (key == "reserved_bytes") out->integer = r.kind == runtime_kind ? r.reserved : r.charge;
                else if (key == "reserved_vram_bytes") out->integer = r.kind == runtime_kind ? r.vram_reserved : r.vram_charge;
                else if (key == "active_sessions" && r.kind == runtime_kind) out->integer = r.sessions;
                else if (key == "completed_requests" && r.kind == session_kind) out->integer = r.completed_requests;
                else if (r.request && key == "rows") out->integer = int64_t(r.request->rows.size());
                else if (r.request && key == "input_tokens") out->integer = r.request->tokens;
                else if (r.request && key == "input_bytes") out->integer = r.request->bytes;
                else if (r.request && key == "output_bytes") out->integer = r.request->output_bytes;
                else if (r.request && key == "last_work_tokens") out->integer = r.request->last_work_tokens;
                else if (r.request && key == "maximum_work_tokens") out->integer = r.request->maximum_work_tokens;
                else if (r.request && key == "prefill_calls") out->integer = r.request->prefill_calls;
                else if (r.request && key == "token_calls") out->integer = r.request->token_calls;
                else if (r.request && key == "decode_calls") out->integer = r.request->decode_calls;
                else if (r.request && key == "submit_us") out->integer = int64_t(r.request->submit_ms * 1000);
                else if (r.request && key == "process_us") out->integer = int64_t(r.request->process_ms * 1000);
                else if (key == "prepare_us") out->integer = int64_t(r.prepare_ms * 1000);
                else if (key == "effective_threads" && r.context) out->integer = llama_n_threads(r.context);
                else if (key == "effective_batch_threads" && r.context) out->integer = llama_n_threads_batch(r.context);
                else if (key == "effective_context_tokens" && r.context) out->integer = llama_n_ctx(r.context);
                else if (key == "effective_batch_tokens" && r.context) out->integer = llama_n_ubatch(r.context);
                else if (r.config.ints.count(key)) out->integer = r.config.ints.at(key);
                else if (r.model) {
                    auto &m = *r.model; std::lock_guard<std::mutex> lock(m.mutex);
                    if (key == "owner_count") out->integer = m.owners;
                    else if (key == "peak_owner_count") out->integer = m.peak_owners;
                    else if (key == "load_count") out->integer = m.model ? 1 : 0;
                    else if (key == "load_us") out->integer = int64_t(m.load_ms * 1000);
                    else if (key == "gpu_layers") out->integer = m.actual_gpu_layers;
                    else if (key == "dimensions") out->integer = m.model ? llama_model_n_embd_out(m.model) : 0;
                    else if (key == "model_bytes") out->integer = m.model ? int64_t(llama_model_size(m.model)) : 0;
                    else throw Failure(-1, "unknown model integer property");
                } else throw Failure(-1, "unknown resource integer property");
            }
            break;
        }
        default: throw Failure(-1, "unknown operation");
        }
        out->text = vm.text.c_str(); return 0;
    } catch (const Failure &e) { vm.error = e.code; vm.message = e.what(); }
      catch (const std::bad_alloc &) { vm.error = -5; vm.message = "native allocation failed"; }
      catch (const std::exception &e) { vm.error = -6; vm.message = e.what(); }
      catch (...) { vm.error = -6; vm.message = "unexpected native exception"; }
    return vm.error;
}
}
