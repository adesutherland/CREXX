// cREXX native-inference direct-library acceptance workload. MIT licence.
// No RXPA provider implementation: this is the independent STEP-02 control.
#include "llama.h"
#include "ggml-backend.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#if CONTROL_ASAN
#include <sanitizer/allocator_interface.h>
#endif
#if defined(__APPLE__)
#include <mach/mach.h>
#elif defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::ordered_json;
using Model = std::unique_ptr<llama_model, decltype(&llama_model_free)>;
using Context = std::unique_ptr<llama_context, decltype(&llama_free)>;
using Tokens = std::vector<llama_token>;
using Vectors = std::vector<std::vector<double>>;
using Clock = std::chrono::steady_clock;
static double milliseconds(Clock::time_point start) {
    return std::chrono::duration<double,std::milli>(Clock::now()-start).count();
}
struct UnitTime { size_t calls = 0; double total = 0, maximum = 0; };
static thread_local std::map<std::string,UnitTime> unit_times;
static int decode_unit(llama_context * ctx, llama_batch batch, const std::string & kind) {
    auto started = Clock::now();
    int status = llama_decode(ctx,batch);
    llama_synchronize(ctx); // Include GPU completion, not just command submission.
    double duration = milliseconds(started);
    auto & value = unit_times[kind];
    ++value.calls; value.total += duration; value.maximum = std::max(value.maximum,duration);
    return status;
}
static json unit_report() {
    json report = json::object();
    for (const auto & item : unit_times)
        report[item.first] = {{"calls",item.second.calls},{"total_ms",item.second.total},{"max_ms",item.second.maximum}};
    return report;
}
static json parity_failures = json::array();
static json parity_summary = json::object();
// S2-D01 version 2, approved 2026-09-14. These are numerical regression
// tripwires for the pinned BGE F16 controls, not retrieval-quality scores.
struct ParityLimit { double absolute; double cosine; };
static constexpr ParityLimit same_layout{0.00001,0.999999};
static constexpr ParityLimit different_layout{0.001,0.99998};
static constexpr ParityLimit different_backend{0.002,0.9999};
static uint64_t resident_bytes() {
#if defined(__APPLE__)
    mach_task_basic_info_data_t info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS)
        throw std::runtime_error("cannot read resident memory");
    return info.resident_size;
#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS info{};
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) throw std::runtime_error("cannot read resident memory");
    return info.WorkingSetSize;
#else
    uint64_t total = 0, resident = 0;
    std::ifstream input("/proc/self/statm");
    if (!(input >> total >> resident)) throw std::runtime_error("cannot read resident memory");
    return resident * uint64_t(sysconf(_SC_PAGESIZE));
#endif
}
static void require(bool condition, const std::string & message) {
    if (!condition) throw std::runtime_error(message);
}
struct Batch {
    llama_batch value;
    explicit Batch(int capacity): value(llama_batch_init(capacity, 0, 1)) {}
    ~Batch() { llama_batch_free(value); }
    Batch(const Batch &) = delete;
    Batch & operator=(const Batch &) = delete;
    void add(llama_token token, int position, int sequence, bool output) {
        int i = value.n_tokens++;
        value.token[i] = token;
        value.pos[i] = position;
        value.n_seq_id[i] = 1;
        value.seq_id[i][0] = sequence;
        value.logits[i] = output;
    }
};
static Tokens tokenize(const llama_vocab * vocab, const std::string & text, bool special = false) {
    int count = llama_tokenize(vocab, text.data(), int(text.size()), nullptr, 0, true, special);
    require(count <= 0 && count != INT32_MIN, "tokenizer size failed");
    Tokens tokens(size_t(-count));
    int actual = llama_tokenize(vocab, text.data(), int(text.size()), tokens.data(), int(tokens.size()), true, special);
    require(actual >= 0, "tokenizer failed");
    tokens.resize(size_t(actual));
    return tokens;
}
static Context context(llama_model * model, bool embedding, bool gpu, int threads) {
    auto p = llama_context_default_params();
    p.n_ctx = 4096; p.n_seq_max = 8;
    p.n_batch = embedding ? 4096 : 512; p.n_ubatch = embedding ? 4096 : 128;
    if (!embedding) { p.n_outputs_max = 8; p.n_outputs_max_per_seq = 1; }
    p.n_threads = threads; p.n_threads_batch = threads;
    p.embeddings = embedding;
    p.pooling_type = embedding ? LLAMA_POOLING_TYPE_CLS : LLAMA_POOLING_TYPE_NONE;
    p.offload_kqv = gpu; p.op_offload = gpu; p.no_perf = false;
    const char * flash = std::getenv("CREXX_LLAMA_CONTROL_FLASH");
    if (flash) {
        require(std::string(flash) == "off", "only the labelled flash-off diagnostic is supported");
        p.flash_attn_type = LLAMA_FLASH_ATTN_TYPE_DISABLED;
    }
    Context ctx(llama_init_from_model(model, p), llama_free);
    require(bool(ctx), "context initialization failed");
    return ctx;
}
static void clear(llama_context * ctx) {
    if (auto memory = llama_get_memory(ctx)) llama_memory_clear(memory, true);
}
static Vectors embed(llama_context * ctx, llama_model * model, const std::vector<Tokens> & inputs) {
    require(!inputs.empty() && inputs.size() <= 8, "embedding row limit");
    Batch batch(4096);
    for (size_t row = 0; row < inputs.size(); ++row) {
        require(!inputs[row].empty() && inputs[row].size() <= 512, "embedding token limit");
        for (size_t pos = 0; pos < inputs[row].size(); ++pos)
            batch.add(inputs[row][pos], int(pos), int(row), true);
    }
    clear(ctx);
    // llama.cpp routes BERT through decode despite its noncausal attention.
    require(decode_unit(ctx, batch.value,"embedding") == 0, "embedding decode failed");
    const int dimensions = llama_model_n_embd_out(model);
    require(dimensions == 384, "BGE dimensions differ from pinned contract");
    Vectors output;
    for (size_t row = 0; row < inputs.size(); ++row) {
        const float * values = llama_get_embeddings_seq(ctx, int(row));
        require(values != nullptr, "missing sequence embedding");
        double norm = 0;
        for (int i = 0; i < dimensions; ++i) {
            require(std::isfinite(values[i]), "nonfinite embedding");
            norm += double(values[i]) * values[i];
        }
        require(norm > 0, "zero embedding");
        std::vector<double> normalized(size_t(dimensions), 0);
        for (int i = 0; i < dimensions; ++i) normalized[size_t(i)] = values[i] / std::sqrt(norm);
        output.push_back(std::move(normalized));
    }
    return output;
}
static double cosine(const std::vector<double> & a, const std::vector<double> & b) {
    require(a.size() == b.size(), "vector dimensions mismatch");
    double dot = 0, aa = 0, bb = 0;
    for (size_t i = 0; i < a.size(); ++i) { dot += a[i]*b[i]; aa += a[i]*a[i]; bb += b[i]*b[i]; }
    return dot / std::sqrt(aa*bb);
}
static void compare(const Vectors & actual, const Vectors & expected, ParityLimit limit, const std::string & label) {
    require(!actual.empty(), "empty vector comparison");
    require(actual.size() == expected.size(), "row count mismatch");
    double maximum_delta = 0, lowest_cosine = 1;
    for (size_t r = 0; r < actual.size(); ++r) {
        require(actual[r].size() == 384, "BGE reference dimensions mismatch");
        require(actual[r].size() == expected[r].size(), "dimensions mismatch");
        double squared_norm = 0, expected_squared_norm = 0;
        for (size_t d = 0; d < actual[r].size(); ++d) {
            require(std::isfinite(actual[r][d]) && std::isfinite(expected[r][d]), "nonfinite comparison vector");
            maximum_delta = std::max(maximum_delta, std::abs(actual[r][d] - expected[r][d]));
            squared_norm += actual[r][d]*actual[r][d];
            expected_squared_norm += expected[r][d]*expected[r][d];
        }
        require(std::abs(std::sqrt(squared_norm) - 1) <= 0.00001, "embedding norm invariant");
        require(std::abs(std::sqrt(expected_squared_norm) - 1) <= 0.00001, "reference norm invariant");
        lowest_cosine = std::min(lowest_cosine, cosine(actual[r], expected[r]));
    }
    if (!parity_summary.contains(label))
        parity_summary[label] = {{"comparisons",0},{"max_abs",0.0},{"min_cosine",1.0},
                                 {"max_abs_allowed",limit.absolute},{"min_cosine_allowed",limit.cosine}};
    auto & summary = parity_summary[label];
    summary["comparisons"] = summary["comparisons"].get<int>() + 1;
    summary["max_abs"] = std::max(summary["max_abs"].get<double>(),maximum_delta);
    summary["min_cosine"] = std::min(summary["min_cosine"].get<double>(),lowest_cosine);
    if (maximum_delta > limit.absolute || lowest_cosine < limit.cosine)
        std::cerr << "embedding parity max_abs=" << maximum_delta << " min_cosine=" << lowest_cosine << '\n';
    if (maximum_delta > limit.absolute || lowest_cosine < limit.cosine)
        parity_failures.push_back({{"comparison",label},{"max_abs",maximum_delta},{"min_cosine",lowest_cosine},
                                  {"max_abs_allowed",limit.absolute},{"min_cosine_allowed",limit.cosine}});
}
static void test_tripwire() {
    Vectors reference(1,std::vector<double>(384,0)); reference[0][0] = 1;
    auto rotated = [&](double delta, size_t count) {
        auto result = reference;
        for (size_t i = 1; i <= count; ++i) result[0][i] = delta;
        result[0][0] = std::sqrt(1-count*delta*delta);
        return result;
    };
    compare(reference,reference,same_layout,"identical");
    compare(rotated(0.0005,1),reference,different_layout,"accepted_layout_variation");
    require(parity_failures.empty(),"tripwire rejected positive controls");
    compare(rotated(0.0005,1),reference,same_layout,"same_layout_drift");
    compare(rotated(0.002,1),reference,different_layout,"coordinate_corruption");
    compare(rotated(0.0007,383),reference,different_layout,"distributed_direction_corruption");
    compare(rotated(0.003,1),reference,different_backend,"backend_coordinate_corruption");
    compare(rotated(0.001,383),reference,different_backend,"backend_direction_corruption");
    require(parity_failures.size() == 5,"tripwire missed numerical corruption");
    auto rejects = [&](const Vectors & actual, const Vectors & expected) {
        bool rejected = false;
        try { compare(actual,expected,different_layout,"invalid"); }
        catch (const std::runtime_error &) { rejected = true; }
        require(rejected,"tripwire missed invalid vectors");
    };
    auto invalid = reference; invalid[0][1] = std::numeric_limits<double>::quiet_NaN();
    rejects(invalid,reference); rejects(reference,invalid);
    invalid = reference; invalid[0][0] = std::numeric_limits<double>::infinity();
    rejects(invalid,reference);
    invalid = reference; invalid[0][0] = 0; rejects(invalid,reference);
    invalid = reference; invalid[0].pop_back(); rejects(invalid,reference);
    rejects({},{});
    parity_failures = json::array(); parity_summary = json::object();
    std::cout << "PASS embedding tripwire: accepted variation; rejected coordinate/direction drift, NaN, infinity, zero, dimensions and empty output\n";
}
static std::string prompt(const std::string & text) {
    return "<|im_start|>system\nYou are a helpful AI assistant named SmolLM, trained by Hugging Face<|im_end|>\n"
           "<|im_start|>user\n" + text + "<|im_end|>\n<|im_start|>assistant\n";
}
struct Generated { std::vector<Tokens> tokens; std::vector<std::string> finish; };
static llama_token greedy(llama_context * ctx, const llama_vocab * vocab, int output) {
    const float * values = llama_get_logits_ith(ctx,output);
    require(values != nullptr,"missing logits");
    int size = llama_vocab_n_tokens(vocab);
    for (int i = 0; i < size; ++i) require(std::isfinite(values[i]),"nonfinite logits");
    return int(std::max_element(values,values+size)-values);
}
static Generated generate(llama_context * ctx, llama_model * model, const std::vector<Tokens> & inputs) {
    require(!inputs.empty() && inputs.size() <= 8, "generation row limit");
    auto vocab = llama_model_get_vocab(model);
    Batch batch(4096);
    std::vector<int> logits(inputs.size()), positions(inputs.size());
    for (size_t row = 0; row < inputs.size(); ++row) {
        require(!inputs[row].empty() && inputs[row].size() + 32 <= 512, "generation context limit");
        for (size_t pos = 0; pos < inputs[row].size(); ++pos) {
            batch.add(inputs[row][pos], int(pos), int(row), pos + 1 == inputs[row].size());
        }
        logits[row] = batch.value.n_tokens - 1;
        positions[row] = int(inputs[row].size());
    }
    clear(ctx);
    // Causal generation can yield between the already configured 128-token
    // physical batches. Preserve all logical rows/positions; collect each
    // prompt's first sample before a later decode replaces its logits. BERT's
    // noncausal embedding batch must not use this splitting path.
    std::vector<llama_token> first_tokens(inputs.size());
    Batch unit(int(llama_n_ubatch(ctx)));
    for (int first = 0; first < batch.value.n_tokens; first += int(llama_n_ubatch(ctx))) {
        unit.value.n_tokens = 0;
        int end = std::min(batch.value.n_tokens,first+int(llama_n_ubatch(ctx)));
        for (int i = first; i < end; ++i)
            unit.add(batch.value.token[i],batch.value.pos[i],batch.value.seq_id[i][0],batch.value.logits[i]);
        require(decode_unit(ctx,unit.value,"prefill") == 0,"generation prefill failed");
        for (int i = 0; i < unit.value.n_tokens; ++i)
            if (unit.value.logits[i]) first_tokens[size_t(unit.value.seq_id[i][0])] = greedy(ctx,vocab,i);
    }
    Generated result{std::vector<Tokens>(inputs.size()), std::vector<std::string>(inputs.size())};
    for (int step = 0; step < 32; ++step) {
        batch.value.n_tokens = 0;
        for (size_t row = 0; row < inputs.size(); ++row) {
            if (!result.finish[row].empty()) continue;
            llama_token next = step == 0 ? first_tokens[row] : greedy(ctx,vocab,logits[row]);
            if (llama_vocab_is_eog(vocab, next)) { result.finish[row] = "eos"; continue; }
            result.tokens[row].push_back(next);
            if (step == 31) { result.finish[row] = "output_limit"; continue; }
            logits[row] = batch.value.n_tokens;
            batch.add(next, positions[row]++, int(row), true);
        }
        if (batch.value.n_tokens == 0) break;
        require(decode_unit(ctx, batch.value,"decode") == 0, "generation decode failed");
    }
    return result;
}
static json perf(llama_context * ctx) {
    // Upstream diagnostic counters, kept separate from Level B process timing.
    auto p = llama_perf_context(ctx);
    return {{"prompt_tokens",p.n_p_eval},{"generated_tokens",p.n_eval},
            {"prompt_eval_ms",p.t_p_eval_ms},{"token_eval_ms",p.t_eval_ms}};
}
static void load_backend(const std::filesystem::path & dir, const std::string & name) {
    for (const auto & file : {"libggml-" + name + ".so", "libggml-" + name + ".dylib", "ggml-" + name + ".dll"}) {
        auto path = dir / file;
        if (std::filesystem::exists(path)) {
            require(ggml_backend_load(path.string().c_str()) != nullptr, "backend load failed: " + path.string());
            return;
        }
    }
    throw std::runtime_error("required backend absent: " + name);
}
static json cancellation_probe(llama_context * ctx, const std::vector<Tokens> & inputs, bool embedding, bool gpu) {
    json report = json::array();
    auto build_prefill = [&](Batch & batch) {
        for (size_t row = 0; row < inputs.size(); ++row)
            for (size_t pos = 0; pos < inputs[row].size(); ++pos)
                batch.add(inputs[row][pos],int(pos),int(row),embedding || pos+1 == inputs[row].size());
    };
    for (const auto & phase : embedding ? std::vector<std::string>{"embedding"} : std::vector<std::string>{"prefill","decode"}) {
        clear(ctx);
        Batch batch(4096);
        if (phase == "decode") {
            build_prefill(batch);
            require(decode_unit(ctx,batch.value,"cancel_setup") == 0,"cancel prefill setup failed");
            batch.value.n_tokens = 0;
            for (size_t row = 0; row < inputs.size(); ++row)
                batch.add(inputs[row].back(),int(inputs[row].size()),int(row),true);
        } else build_prefill(batch);
        // CPU: request cancellation from the first actual compute callback and
        // abort at the next callback. GPU: another thread requests it while the
        // admitted unit is executing; we drain before observing the flag.
        struct Cancel { std::atomic<int> callbacks{0}; Clock::time_point requested; } cancellation;
        if (!gpu) llama_set_abort_callback(ctx,[](void * data) {
            auto & c = *static_cast<Cancel *>(data);
            if (c.callbacks.fetch_add(1) == 0) { c.requested = Clock::now(); return false; }
            return true;
        },&cancellation);
        std::promise<void> start;
        std::future<void> request;
        if (gpu) request = std::async(std::launch::async,[&] {
            start.get_future().wait();
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            cancellation.requested = Clock::now();
        });
        auto began = Clock::now();
        if (gpu) start.set_value();
        int status = decode_unit(ctx,batch.value,"cancel_"+phase);
        auto completed = Clock::now();
        if (gpu) request.get();
        llama_set_abort_callback(ctx,nullptr,nullptr);
        require(gpu ? status == 0 : status == 2,"unexpected inference cancellation result");
        require(gpu || cancellation.callbacks.load() >= 2,"CPU abort callback did not execute");
        double elapsed = std::max(0.0,std::chrono::duration<double,std::milli>(completed-cancellation.requested).count());
        bool during = cancellation.requested >= began && cancellation.requested < completed;
        report.push_back({{"phase",phase},{"mechanism",gpu ? "between_units_after_gpu_drain" : "cpu_abort_callback"},
            {"return_code",status},{"request_during_unit",during},{"callbacks",cancellation.callbacks.load()},
            {"cancel_to_drain_ms",elapsed},{"unit_ms",std::chrono::duration<double,std::milli>(completed-began).count()}});
        require(during,"cancellation probe missed the active work unit");
#if CONTROL_TIMING_GATE
        require(elapsed <= 250,"warm cancellation exceeded 250 ms");
#endif
    }
    clear(ctx);
    return report;
}
static json invalid_loads(const std::filesystem::path & real_model, llama_model_params params) {
    auto dir = std::filesystem::temp_directory_path() / ("crexx-ni-invalid-"+std::to_string(Clock::now().time_since_epoch().count()));
    require(std::filesystem::create_directory(dir),"cannot create invalid-model fixture directory");
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove_all(path,error); } } cleanup{dir};
    { std::ofstream f(dir/"bad-magic.gguf",std::ios::binary); f << "this is not GGUF\n"; }
    { std::ifstream source(real_model,std::ios::binary); char header[64]; source.read(header,sizeof(header));
      require(source.gcount() == sizeof(header),"cannot read truncated fixture input");
      std::ofstream f(dir/"truncated.gguf",std::ios::binary); f.write(header,sizeof(header)); }
    json report = json::array();
    for (const auto & file : {"missing.gguf","bad-magic.gguf","truncated.gguf"}) {
        Model invalid(llama_model_load_from_file((dir/file).string().c_str(),params),llama_model_free);
        require(!invalid,"invalid model was accepted");
        report.push_back({{"case",file},{"rejected",true}});
    }
    return report;
}
static int co_resident(int argc, char ** argv) {
    require(argc == 7,"--co-resident BGE SMOL BACKEND_DIR cpu|metal OUTPUT");
    std::string device = argv[5];
    require(device == "cpu" || device == "metal","co-resident local control backend");
    load_backend(argv[4],"cpu");
    if (device != "cpu") load_backend(argv[4],device);
    llama_backend_init();
    ggml_backend_dev_t gpu = nullptr;
    for (size_t i = 0; i < ggml_backend_dev_count(); ++i)
        if (device != "cpu" && ggml_backend_dev_type(ggml_backend_dev_get(i)) == GGML_BACKEND_DEVICE_TYPE_GPU) gpu = ggml_backend_dev_get(i);
    require(device == "cpu" || gpu,"required Metal device absent");
    ggml_backend_dev_t placement[] = {gpu,nullptr};
    auto params = llama_model_default_params(); params.devices = placement; params.n_gpu_layers = gpu ? 999 : 0;
    json result = {{"device",device},{"scope","direct two-model allocation/lifecycle control; no provider registry"},
                   {"resident_before_bytes",resident_bytes()},{"primary_model_loads",2}};
    Model bge(llama_model_load_from_file(argv[2],params),llama_model_free);
    Model smol(llama_model_load_from_file(argv[3],params),llama_model_free);
    require(bge && smol,"two-model load failed");
    result["unique_model_payload_bytes"] = llama_model_size(bge.get())+llama_model_size(smol.get());
    result["resident_after_models_bytes"] = resident_bytes();
    std::vector<Tokens> documents, prompts;
    for (int i = 0; i < 8; ++i) {
        std::string text;
        for (int j = 0; j < (i%2 ? 126 : 382); ++j) text += "cat ";
        documents.push_back(tokenize(llama_model_get_vocab(bge.get()),text));
    }
    for (int i = 0; i < 4; ++i) prompts.push_back(tokenize(llama_model_get_vocab(smol.get()),prompt("Name one animal."),true));
    std::vector<Context> embeddings, generations;
    json stages = json::array();
    for (int owners : {1,2,4}) {
        while (int(embeddings.size()) < owners) {
            embeddings.push_back(context(bge.get(),true,gpu != nullptr,2));
            generations.push_back(context(smol.get(),false,gpu != nullptr,2));
            require(llama_get_model(embeddings.back().get()) == bge.get(),"embedding weight identity changed");
            require(llama_get_model(generations.back().get()) == smol.get(),"generation weight identity changed");
        }
        auto expected_vectors = embed(embeddings[0].get(),bge.get(),documents);
        auto expected_tokens = generate(generations[0].get(),smol.get(),prompts).tokens;
        auto started = Clock::now();
        std::vector<std::future<Vectors>> ejobs;
        std::vector<std::future<Generated>> gjobs;
        for (int i = 0; i < owners; ++i) {
            ejobs.push_back(std::async(std::launch::async,[&,i]{return embed(embeddings[i].get(),bge.get(),documents);}));
            gjobs.push_back(std::async(std::launch::async,[&,i]{return generate(generations[i].get(),smol.get(),prompts);}));
        }
        for (auto & job : ejobs) compare(job.get(),expected_vectors,same_layout,"two_model_private_contexts");
        for (auto & job : gjobs) require(job.get().tokens == expected_tokens,"two-model generation isolation failed");
        auto resident = resident_bytes();
        stages.push_back({{"owners_per_model",owners},{"unique_models",2},{"private_contexts",2*owners},
                          {"resident_bytes",resident},{"concurrent_work_ms",milliseconds(started)}});
        require(resident <= 4ull*1024*1024*1024,"two-model process exceeds 4 GiB budget");
    }
    result["allocation_stages"] = stages;
    embeddings.clear(); bge.reset();
    require(!generate(generations[0].get(),smol.get(),prompts).tokens[0].empty(),"closing embedding model damaged generation");
    generations.clear(); smol.reset();
    result["resident_after_close_bytes"] = resident_bytes();
    json reloads = json::array(), live_reload_allocations = json::array();
    for (int i = 0; i < 4; ++i) {
        Model model(llama_model_load_from_file(argv[2],params),llama_model_free);
        require(bool(model),"reload failed");
        auto ctx = context(model.get(),true,gpu != nullptr,2);
        require(embed(ctx.get(),model.get(),documents).size() == 8,"reload inference failed");
        ctx.reset(); model.reset(); reloads.push_back(resident_bytes());
#if CONTROL_ASAN
        live_reload_allocations.push_back(__sanitizer_get_current_allocated_bytes());
#endif
    }
    const auto & retained_samples = live_reload_allocations.empty() ? reloads : live_reload_allocations;
    uint64_t first = retained_samples[0].get<uint64_t>(), maximum = first;
    for (auto value : retained_samples) maximum = std::max(maximum,value.get<uint64_t>());
    require(maximum-first <= 32u*1024*1024,"close/reload retains more than 32 MiB");
    result["closed_reload_cycle_resident_bytes"] = reloads;
    result["closed_reload_cycle_live_allocation_bytes"] = live_reload_allocations;
    result["retention_metric"] = live_reload_allocations.empty() ? "process_rss" : "asan_live_allocations";
    result["closed_reload_retained_growth_bytes"] = maximum-first;
    result["parity_failures"] = parity_failures;
    result["result"] = parity_failures.empty() ? "PASS" : "FAIL";
    llama_backend_free();
    std::ofstream output(argv[6]); output << result.dump(2) << '\n'; output.close(); require(bool(output),"co-resident output failed");
    std::cout << "CONTROL_JSON " << result.dump() << '\n' << result["result"].get<std::string>() << " co-resident inference control\n";
    return parity_failures.empty() ? 0 : 1;
}
int main(int argc, char ** argv) {
    try {
        if (argc > 1 && std::string(argv[1]) == "--co-resident") return co_resident(argc,argv);
        if (argc == 2 && std::string(argv[1]) == "--help") {
            std::cout << "model backend-directory cpu|metal|cuda|vulkan bge|smol|scratch output.json [repeats=100] [batches=20] [owners=4] [threads=4] [reference.json]\n";
            return 0;
        }
        if (argc == 2 && std::string(argv[1]) == "--test-tripwire") {
            test_tripwire(); return 0;
        }
        require(argc >= 6 && argc <= 11, "use --help for arguments");
        std::string device = argv[3], profile = argv[4];
        require(device == "cpu" || device == "metal" || device == "cuda" || device == "vulkan", "unsupported control device");
        require(profile == "bge" || profile == "smol" || profile == "scratch", "unsupported control profile");
        int repeats = argc > 6 ? std::stoi(argv[6]) : 100;
        int batches = argc > 7 ? std::stoi(argv[7]) : 20;
        int owners = argc > 8 ? std::stoi(argv[8]) : 4;
        int threads = argc > 9 ? std::stoi(argv[9]) : 4;
        require(repeats >= 1 && batches >= 1 && owners >= 1 && owners <= 4 && threads >= 1, "invalid work limits");
        auto backend_started = Clock::now();
        load_backend(argv[2], "cpu");
        if (device != "cpu") load_backend(argv[2], device);
        llama_backend_init();
        ggml_backend_dev_t selected = nullptr;
        json devices = json::array();
        for (size_t i = 0; i < ggml_backend_dev_count(); ++i) {
            auto d = ggml_backend_dev_get(i);
            devices.push_back({{"name",ggml_backend_dev_name(d)},{"description",ggml_backend_dev_description(d)}});
            if (device != "cpu" && ggml_backend_dev_type(d) == GGML_BACKEND_DEVICE_TYPE_GPU && selected == nullptr) selected = d;
        }
        require(device == "cpu" || selected != nullptr, "required GPU absent");
        double backend_ms = milliseconds(backend_started);
        ggml_backend_dev_t placement[] = {selected, nullptr};
        auto mp = llama_model_default_params();
        mp.devices = placement; mp.n_gpu_layers = selected ? 999 : 0;
        auto load_started = Clock::now();
        Model model(llama_model_load_from_file(argv[1], mp), llama_model_free);
        require(bool(model), "model load failed");
        double model_load_ms = milliseconds(load_started);
        uint64_t memory_after_model = resident_bytes();
        const auto vocab = llama_model_get_vocab(model.get());
        bool embedding = profile == "bge";
        std::vector<Context> contexts;
        json context_memory = json::array();
        json context_times = json::array();
        for (int i = 0; i < owners; ++i) {
            auto started = Clock::now();
            contexts.push_back(context(model.get(), embedding, selected != nullptr, threads));
            context_times.push_back(milliseconds(started));
            require(llama_get_model(contexts.back().get()) == model.get(),"context does not share original model");
            context_memory.push_back(resident_bytes());
        }
        auto ctx = contexts[0].get();
        std::vector<std::string> texts = {
            "The cat is sleeping on a warm windowsill.",
            "Represent this sentence for searching relevant passages: Where does the cat sleep?",
            "Bonjour. Caf\u00e9, Stra\u00dfe, \u65e5\u672c\u8a9e.", "",
            "A sleeping cat rests beside the window.", "The engine converts electrical energy into motion.",
            "Numbers: 17, 29, and 43.", "Rain falls over the hills of Scotland."};
        if (embedding) {
            texts[5].clear(); texts[6].clear(); texts[7].clear();
            for (int i = 0; i < 126; ++i) texts[5] += "engine ";
            for (int i = 0; i < 30; ++i) texts[6] += "number ";
            for (int i = 0; i < 382; ++i) texts[7] += "rain ";
        }
        std::vector<Tokens> inputs;
        for (size_t i = 0; i < (embedding ? 8u : 4u); ++i) {
            inputs.push_back(profile == "scratch" ? Tokens{1,2,3,4,int(5+i)} :
                             tokenize(vocab, embedding ? texts[i] : prompt(texts[i]), !embedding));
        }
        json output = {{"profile",profile},{"device",device},{"devices",devices},
            {"model_bytes",llama_model_size(model.get())},{"model_parameters",llama_model_n_params(model.get())},
            {"model_loads",1},{"owners",owners},{"repeated_requests",repeats},{"batches",batches},
            {"threads_per_context",threads},{"input_token_ids",inputs}};
        output["flash_attention"] = std::getenv("CREXX_LLAMA_CONTROL_FLASH") ? "off-diagnostic" : "auto";
        output["context_model_identity_shared"] = true;
        output["resident_after_model_bytes"] = memory_after_model;
        output["resident_after_each_context_bytes"] = context_memory;
        output["backend_setup_ms"] = backend_ms;
        output["model_load_ms"] = model_load_ms;
        output["context_create_ms"] = context_times;
        output["timing_limits_enforced"] = bool(CONTROL_TIMING_GATE);
        json memory_samples = json::array();
        json allocation_samples = json::array();
        auto sample_memory = [&] {
            memory_samples.push_back(resident_bytes());
#if CONTROL_ASAN
            allocation_samples.push_back({{"live_bytes",__sanitizer_get_current_allocated_bytes()},
                {"heap_bytes",__sanitizer_get_heap_size()},{"free_bytes",__sanitizer_get_free_bytes()}});
#endif
        };
        json single_times = json::array(), batch_times = json::array();
        auto preparation_started = Clock::now();
        if (embedding) {
            output["numeric_policy"] = "S2-D01-v2";
            Vectors serial;
            for (const auto & input : inputs) {
                auto started = Clock::now();
                serial.push_back(embed(ctx,model.get(),{input})[0]);
                if (serial.size() == 1) output["first_request_ms"] = milliseconds(started);
            }
            const auto batched = embed(ctx,model.get(),inputs);
            compare(batched,serial,different_layout,"single_vs_batch8");
            Vectors batch4;
            for (size_t first = 0; first < inputs.size(); first += 4) {
                std::vector<Tokens> rows(inputs.begin()+first,inputs.begin()+first+4);
                auto result = embed(ctx,model.get(),rows);
                compare(embed(ctx,model.get(),rows),result,same_layout,"batch4_repeat");
                batch4.insert(batch4.end(),result.begin(),result.end());
            }
            compare(batch4,serial,different_layout,"single_vs_batch4");
            compare(batch4,batched,different_layout,"batch4_vs_batch8");
            for (int i = 0; i < 2; ++i) compare(embed(ctx,model.get(),inputs),batched,same_layout,"batch8_warmup");
            output["preparation_requests_ms"] = milliseconds(preparation_started);
            output["preparation_compute_units"] = unit_report();
            sample_memory();
            llama_perf_context_reset(ctx);
            unit_times.clear();
            for (int i = 0; i < repeats; ++i) {
                auto started = Clock::now();
                auto actual = embed(ctx,model.get(),{inputs[size_t(i)%inputs.size()]});
                single_times.push_back(milliseconds(started));
                compare(actual,{serial[size_t(i)%inputs.size()]},same_layout,"single_repeat");
                if ((i+1)%10 == 0) sample_memory();
            }
            for (int i = 0; i < batches; ++i) {
                auto started = Clock::now();
                auto actual = embed(ctx,model.get(),inputs);
                batch_times.push_back(milliseconds(started));
                compare(actual,batched,same_layout,"batch8_repeat");
            }
            sample_memory();
            output["warm_upstream_counters"] = perf(ctx);
            output["warm_units"] = unit_report();
            std::vector<std::future<Vectors>> jobs;
            for (auto & owner : contexts) jobs.push_back(std::async(std::launch::async,[&,c=owner.get()]{return embed(c,model.get(),inputs);}));
            for (auto & job : jobs) compare(job.get(),batched,same_layout,"batch8_private_contexts");
            std::string boundary;
            for (int i = 0; i < 510; ++i) boundary += " a";
            auto exact = tokenize(vocab,boundary);
            require(exact.size() == 512, "512-token fixture changed");
            embed(ctx,model.get(),{exact});
            auto oversized = tokenize(vocab,boundary+" a");
            require(oversized.size() == 513, "513-token fixture changed");
            bool rejected = false;
            try { embed(ctx,model.get(),{oversized}); } catch (const std::runtime_error & e) { rejected = std::string(e.what()) == "embedding token limit"; }
            require(rejected,"oversized embedding accepted");
            output["boundary_tokens"] = {exact.size(),oversized.size()};
            const std::string prefix = "Represent this sentence for searching relevant passages: ";
            std::string query_boundary = prefix;
            auto prefix_tokens = tokenize(vocab,prefix).size();
            require(prefix_tokens < 512,"query instruction exceeds context");
            for (size_t i = prefix_tokens; i < 512; ++i) query_boundary += "a ";
            auto query_exact = tokenize(vocab,query_boundary);
            auto query_over = tokenize(vocab,query_boundary+"a ");
            require(query_exact.size() == 512 && query_over.size() == 513,"query boundary fixture changed");
            embed(ctx,model.get(),{query_exact});
            rejected = false;
            try { embed(ctx,model.get(),{query_over}); } catch (const std::runtime_error & e) { rejected = std::string(e.what()) == "embedding token limit"; }
            require(rejected,"oversized query accepted");
            output["query_boundary_tokens"] = {query_exact.size(),query_over.size()};
            output["vectors"] = serial;
            output["vectors_batch4"] = batch4;
            output["vectors_batch8"] = batched;
            require(cosine(serial[1],serial[0]) > cosine(serial[1],serial[5]),"semantic positive control failed");
            if (argc == 11) {
                std::ifstream input(argv[10]); require(bool(input),"reference file absent");
                json reference; input >> reference;
                require(reference["input_token_ids"] == output["input_token_ids"],"reference tokenizer mismatch");
                require(reference["numeric_policy"] == "S2-D01-v2", "reference policy mismatch");
                require(reference["result"] == "PASS", "reference control failed");
                compare(serial,reference["vectors"].get<Vectors>(),different_backend,"reference_single");
                compare(batch4,reference["vectors_batch4"].get<Vectors>(),different_backend,"reference_batch4");
                compare(batched,reference["vectors_batch8"].get<Vectors>(),different_backend,"reference_batch8");
                output["cross_backend_parity_checked"] = true;
            }
        } else {
            auto first_started = Clock::now();
            auto expected = generate(ctx,model.get(),inputs);
            output["first_request_ms"] = milliseconds(first_started);
            require(std::any_of(expected.tokens.begin(),expected.tokens.end(),[](const Tokens & t){return !t.empty();}),"all generation controls are empty");
            for (int i = 0; i < 2; ++i) require(generate(ctx,model.get(),inputs).tokens==expected.tokens,"generation warmup mismatch");
            output["preparation_requests_ms"] = milliseconds(preparation_started);
            output["preparation_compute_units"] = unit_report();
            sample_memory();
            llama_perf_context_reset(ctx);
            unit_times.clear();
            for (int i = 0; i < repeats; ++i) {
                auto row = size_t(i)%inputs.size();
                auto started = Clock::now();
                auto actual = generate(ctx,model.get(),{inputs[row]});
                single_times.push_back(milliseconds(started));
                require(actual.tokens[0]==expected.tokens[row],"generation single/batch mismatch");
                if ((i+1)%10 == 0) sample_memory();
            }
            for (int i = 0; i < batches; ++i) {
                auto started = Clock::now();
                auto actual = generate(ctx,model.get(),inputs);
                batch_times.push_back(milliseconds(started));
                require(actual.tokens==expected.tokens,"generation repeat mismatch");
            }
            sample_memory();
            output["warm_upstream_counters"] = perf(ctx);
            output["warm_units"] = unit_report();
            std::vector<std::future<Generated>> jobs;
            for (auto & owner : contexts) jobs.push_back(std::async(std::launch::async,[&,c=owner.get()]{return generate(c,model.get(),inputs);}));
            for (auto & job : jobs) require(job.get().tokens==expected.tokens,"generation shared-context mismatch");
            output["generated_token_ids"] = expected.tokens;
            output["finish_reasons"] = expected.finish;
        }
        output["warm_single_request_ms"] = single_times;
        output["warm_batch_request_ms"] = batch_times;
        double single_total = 0, batch_total = 0;
        for (auto value : single_times) single_total += value.get<double>();
        for (auto value : batch_times) batch_total += value.get<double>();
        output["warm_single_total_ms"] = single_total;
        output["warm_batch_total_ms"] = batch_total;
        // Frozen workload's indivisible units bound cooperative GPU response.
#if CONTROL_TIMING_GATE
        for (const auto & item : output["warm_units"].items())
            require(item.value()["max_ms"].get<double>() <= 250,"warm compute unit exceeded 250 ms");
#endif
        if (profile != "scratch") output["inference_cancellation"] = cancellation_probe(ctx,inputs,embedding,selected != nullptr);
        // Positive recovery after aborted/cleared inference, before failed loads.
        if (embedding) compare(embed(ctx,model.get(),inputs),output["vectors_batch8"].get<Vectors>(),same_layout,"post_cancel_recovery");
        else require(generate(ctx,model.get(),inputs).tokens == output["generated_token_ids"].get<std::vector<Tokens>>(),"post-cancel generation recovery failed");
        Batch empty(1);
        require(llama_decode(ctx,empty.value) == -1,"empty native batch accepted");
        output["invalid_batch_rejected"] = true;
        output["invalid_model_controls"] = invalid_loads(argv[1],mp);
        // Cancel a separate load at its first progress boundary; the existing
        // prepared model/contexts remain a positive lifetime control.
        int progress_calls = 0;
        mp.progress_callback_user_data = &progress_calls;
        mp.progress_callback = [](float,void * user){++*static_cast<int *>(user);return false;};
        auto cancel_started = std::chrono::steady_clock::now();
        Model cancelled(llama_model_load_from_file(argv[1],mp),llama_model_free);
        auto cancel_ms = std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-cancel_started).count();
        require(!cancelled && progress_calls > 0,"cancelled load unexpectedly completed");
        output["cancelled_load_progress_calls"] = progress_calls;
        output["load_cancel_elapsed_ms"] = cancel_ms;
        output["resident_memory_samples_bytes"] = memory_samples;
        uint64_t warmed = memory_samples[0].get<uint64_t>();
        uint64_t maximum = warmed;
        for (auto value : memory_samples) maximum = std::max(maximum,value.get<uint64_t>());
        output["retained_growth_bytes"] = maximum-warmed;
        output["sanitizer_allocation_samples"] = allocation_samples;
        uint64_t retained_growth = maximum-warmed;
#if CONTROL_ASAN
        // ASan's quarantine/reserved heap inflates RSS without growing live
        // allocations. Keep RSS diagnostic; apply the same 32 MiB allowance to
        // owned live allocations in this instrumented lane (S2-QA01).
        uint64_t live_first = allocation_samples[0]["live_bytes"].get<uint64_t>(), live_max = live_first;
        for (const auto & sample : allocation_samples) live_max = std::max(live_max,sample["live_bytes"].get<uint64_t>());
        retained_growth = live_max-live_first;
        output["retention_metric"] = "asan_live_allocations";
#else
        output["retention_metric"] = "process_rss";
#endif
        output["retention_guard_growth_bytes"] = retained_growth;
        require(retained_growth <= 32u*1024u*1024u,"retained memory growth exceeds 32 MiB");
#if CONTROL_TIMING_GATE
        require(cancel_ms <= 1000,"load cancellation exceeded 1 second");
#endif
        contexts.clear(); model.reset(); llama_backend_free();
        output["parity_failures"] = parity_failures;
        output["parity_summary"] = parity_summary;
        output["result"] = parity_failures.empty() ? "PASS" : "FAIL";
        std::ofstream result(argv[5]); require(bool(result),"cannot open output file");
        result << output.dump(2) << '\n'; result.close(); require(bool(result),"output write failed");
        json compact = output;
        for (const auto & key : {"vectors","vectors_batch4","vectors_batch8"}) compact.erase(key);
        std::cout << "CONTROL_JSON " << compact.dump() << '\n';
        std::cout << (parity_failures.empty() ? "PASS" : "FAIL") << " native inference direct control\n";
        return parity_failures.empty() ? 0 : 1;
    } catch (const std::exception & e) {
        std::cerr << "FAIL native inference direct control: " << e.what() << '\n';
        return 1;
    }
}
