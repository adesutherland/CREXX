/* Ordinary STEP-03 lifecycle controls against the real packaged engine. */
#include "bridge.h"
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif
extern "C" LLAMA_BRIDGE_API int rxllama_test_probe_cycle(void);
using Args = std::array<rxllama_argument, 6>;
using Clock = std::chrono::steady_clock;
static void check(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
static rxllama_argument h(rxllama_token t) { rxllama_argument a{}; a.handle=t; return a; }
static rxllama_argument s(const char *t) { rxllama_argument a{}; a.text=t; a.text_length=std::strlen(t); return a; }
static rxllama_argument i(int64_t n) { rxllama_argument a{}; a.integer=n; return a; }
struct VM {
    void *vm = rxllama_vm_create();
    VM() { check(vm != nullptr, "VM creation"); }
    ~VM() { rxllama_vm_destroy(vm); }
    int raw(int op, Args a, rxllama_result &r) { return rxllama_call(vm, op, a.data(), &r); }
    rxllama_result call(int op, Args a = {}) {
        rxllama_result r{};
        if (raw(op,a,r)) { rxllama_result d{}; raw(RXLLAMA_DIAGNOSTIC, {}, d); throw std::runtime_error(std::string(d.operation)+": "+d.message); }
        return r;
    }
    void reject(int op, Args a) { rxllama_result r{}; check(raw(op,a,r) != 0,"negative control accepted"); }
    void close(rxllama_token t) { call(RXLLAMA_CLOSE, {h(t)}); rxllama_release(t); }
    std::string ready(rxllama_token t) {
        auto begin = Clock::now();
        for (;;) {
            std::string state(call(RXLLAMA_MODEL_STATE,{h(t)}).text);
            if (state != "loading") return state;
            check(Clock::now()-begin < std::chrono::seconds(60), "model deadline");
            std::this_thread::yield();
        }
    }
    void prepare(rxllama_token t) {
        auto begin = Clock::now();
        while (std::string(call(RXLLAMA_PREPARE,{h(t),i(1)}).text) != "ready")
            check(Clock::now()-begin < std::chrono::seconds(60), "preparation deadline");
    }
};
struct Barrier { std::mutex m; std::condition_variable cv; int arrived=0; bool failed=false;
    void arrive() { std::unique_lock<std::mutex> lock(m); if (++arrived == 4) cv.notify_all(); else cv.wait(lock,[&]{return arrived == 4 || failed;}); }
    void fail() { std::lock_guard<std::mutex> lock(m); failed=true; cv.notify_all(); }
};
static uint64_t peak_memory() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS info{};
    check(GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info)), "peak memory probe");
    return info.PeakWorkingSetSize;
#else
    rusage usage{}; check(getrusage(RUSAGE_SELF, &usage)==0, "peak memory probe");
#ifdef __APPLE__
    return uint64_t(usage.ru_maxrss);
#else
    return uint64_t(usage.ru_maxrss)*1024;
#endif
#endif
}
static void co_resident(char **argv) {
    VM vm; auto c=vm.call(RXLLAMA_CONFIG_CREATE).handle;
    vm.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s(argv[1])});
    auto rt=vm.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
    auto bge=vm.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[2]),s(argv[3]),s("bge-small-en-v1.5"),h(c)}).handle;
    auto smol=vm.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[4]),s(argv[5]),s("smollm2-360m-instruct"),h(c)}).handle;
    check(vm.ready(bge)=="ready" && vm.ready(smol)=="ready", "co-resident models ready");
    std::vector<rxllama_token> embeddings, generations;
    for(int count=0;count<4;++count) {
        auto e=vm.call(RXLLAMA_SESSION_OPEN,{h(bge),s("embedding"),h(c)}).handle;
        auto g=vm.call(RXLLAMA_SESSION_OPEN,{h(smol),s("generation"),h(c)}).handle;
        vm.prepare(e); vm.prepare(g); embeddings.push_back(e);generations.push_back(g);
    }
    auto reserved=vm.call(RXLLAMA_INFO_INT,{h(rt),s("reserved_bytes")}).integer;
    check(reserved > 0 && reserved <= INT64_C(4294967296),"two-model 4 GiB admission envelope");
    vm.reject(RXLLAMA_SESSION_OPEN,{h(smol),s("generation"),h(c)});
    auto peak=peak_memory(); check(peak <= UINT64_C(4294967296), "two-model peak exceeds 4 GiB");
    for(auto e:embeddings) vm.close(e);
    vm.close(bge);
    for(auto g:generations) {vm.prepare(g);vm.close(g);}
    vm.close(smol);
    check(vm.call(RXLLAMA_INFO_INT,{h(rt),s("reserved_bytes")}).integer==0,"two-model cleanup");
    auto low=vm.call(RXLLAMA_CONFIG_CREATE).handle;
    vm.call(RXLLAMA_CONFIG_INT,{h(low),s("memory_bytes"),i(1024)});
    auto small=vm.call(RXLLAMA_RUNTIME_OPEN,{h(low)}).handle;
    vm.reject(RXLLAMA_MODEL_OPEN,{h(small),s(argv[2]),s(argv[3]),s("bge-small-en-v1.5"),h(low)});
    std::cout << "PASS: co-resident two-model four-session admission/cleanup " << argv[1] << " reserved=" << reserved << " peak_bytes=" << peak << '\n';
}
int main(int argc, char **argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--probe-cycle") return rxllama_test_probe_cycle();
        if (argc == 6) { co_resident(argv); return 0; }
        check(argc == 5, "usage: bridge_lifecycle MODE MODEL SHA256 PROFILE");
        const char *cap = std::string(argv[4]) == "bge-small-en-v1.5" ? "embedding" : "generation";
        VM vm;
        auto config = vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(config),s("hardware_mode"),s(argv[1])});
        vm.call(RXLLAMA_CONFIG_INT,{h(config),s("max_sessions"),i(1)});
        vm.reject(RXLLAMA_CONFIG_INT,{h(config),s("memory_bytes"),i(-1)});
        vm.reject(RXLLAMA_CONFIG_TEXT,{h(config),s("memory_bytes"),s("1")});
        auto devices = vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(devices),s("hardware_mode"),s("cpu")});
        vm.reject(RXLLAMA_CONFIG_TEXT,{h(devices),s("devices"),s("nonexistent-gpu")});
        vm.close(devices);
        auto runtime = vm.call(RXLLAMA_RUNTIME_OPEN,{h(config)}).handle;
        check(vm.call(RXLLAMA_DEVICE_COUNT,{h(runtime)}).integer > 0, "CPU inventory");
        VM foreign;
        foreign.reject(RXLLAMA_DEVICE_COUNT,{h(runtime)});
        vm.reject(RXLLAMA_DEVICE_INFO,{h(runtime),i(0),s("name")});
        auto model = vm.call(RXLLAMA_MODEL_OPEN,{h(runtime),s(argv[2]),s(argv[3]),s(argv[4]),h(config)}).handle;
        auto peer = vm.call(RXLLAMA_MODEL_OPEN,{h(runtime),s(argv[2]),s(argv[3]),s(argv[4]),h(config)}).handle;
        check(vm.ready(model) == "ready" && vm.ready(peer) == "ready", "ready model");
        auto wrong_device = vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(wrong_device),s("devices"),s("nonexistent-gpu")});
        vm.reject(RXLLAMA_SESSION_OPEN,{h(model),s(cap),h(wrong_device)});
        vm.close(wrong_device);
        auto policy = vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_INT,{h(policy),s("vram_bytes"),i(1)});
        auto policy_runtime = vm.call(RXLLAMA_RUNTIME_OPEN,{h(policy)}).handle;
        auto fallback = vm.call(RXLLAMA_MODEL_OPEN,{h(policy_runtime),s(argv[2]),s(argv[3]),s(argv[4]),h(policy)}).handle;
        check(std::string(vm.call(RXLLAMA_INFO_TEXT,{h(fallback),s("backend")}).text)=="cpu", "auto memory fallback");
        check(std::string(vm.call(RXLLAMA_INFO_TEXT,{h(fallback),s("selection")}).text).find("CPU fallback")!=std::string::npos, "owner-specific fallback diagnostic");
        vm.close(fallback);
        vm.call(RXLLAMA_CONFIG_TEXT,{h(policy),s("hardware_mode"),s("required-gpu")});
        vm.reject(RXLLAMA_MODEL_OPEN,{h(policy_runtime),s(argv[2]),s(argv[3]),s(argv[4]),h(policy)});
        vm.close(policy_runtime); vm.close(policy);
        std::string identity(vm.call(RXLLAMA_INFO_TEXT,{h(model),s("shared_model_id")}).text);
        check(identity == vm.call(RXLLAMA_INFO_TEXT,{h(peer),s("shared_model_id")}).text, "shared identity");
        check(vm.call(RXLLAMA_INFO_INT,{h(peer),s("owner_count")}).integer == 2,"shared owners");
        auto session = vm.call(RXLLAMA_SESSION_OPEN,{h(model),s(cap),h(config)}).handle;
        vm.reject(RXLLAMA_SESSION_OPEN,{h(peer),s(cap),h(config)});
        vm.reject(RXLLAMA_CLOSE,{h(model)});
        check(rxllama_retain(session), "handle retain");
        vm.prepare(session); vm.prepare(session);
        if (std::string(argv[1]) == "required-gpu") check(vm.call(RXLLAMA_INFO_INT,{h(model),s("gpu_layers")}).integer > 0,"real offload");
        vm.close(session); vm.reject(RXLLAMA_PREPARE,{h(session),i(512)}); rxllama_release(session);
        vm.close(model);
        session = vm.call(RXLLAMA_SESSION_OPEN,{h(peer),s(cap),h(config)}).handle;
        vm.prepare(session); vm.close(session); vm.close(peer);
        check(vm.call(RXLLAMA_INFO_INT,{h(runtime),s("reserved_bytes")}).integer == 0,"reservation cleanup");
        // A real file hash mismatch and a missing file remain failed model jobs.
        std::string bad_hash(64,'0');
        for (auto path : std::array<const char *,2>{argv[2], "/missing/rxllama/model.gguf"}) {
            auto bad = vm.call(RXLLAMA_MODEL_OPEN,{h(runtime),s(path),s(bad_hash.c_str()),s(argv[4]),h(config)}).handle;
            check(vm.ready(bad) == "failed", "failed model state"); vm.close(bad);
        }
        auto cancelled = vm.call(RXLLAMA_MODEL_OPEN,{h(runtime),s(argv[2]),s(argv[3]),s(argv[4]),h(config)}).handle;
        auto cancel_begin = Clock::now();
        vm.call(RXLLAMA_MODEL_CANCEL,{h(cancelled)});
        check(vm.ready(cancelled) == "cancelled", "cancelled model state"); vm.close(cancelled);
        check(Clock::now()-cancel_begin < std::chrono::seconds(1),"load cancel/drain deadline");
        vm.close(runtime); vm.close(config);
        // Four independent VMs prepare real private contexts on one shared model.
        Barrier barrier; std::mutex results_mutex; std::set<std::string> identities;
        std::vector<std::string> errors; std::vector<std::thread> workers;
        for (int worker=0;worker<4;++worker) workers.emplace_back([&] {
            try {
                VM owner; auto c=owner.call(RXLLAMA_CONFIG_CREATE).handle;
                owner.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s(argv[1])});
                auto rt=owner.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
                auto m=owner.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[2]),s(argv[3]),s(argv[4]),h(c)}).handle;
                check(owner.ready(m)=="ready","worker model ready");
                std::string id(owner.call(RXLLAMA_INFO_TEXT,{h(m),s("shared_model_id")}).text);
                { std::lock_guard<std::mutex> lock(results_mutex); identities.insert(id); }
                barrier.arrive();
                auto session=owner.call(RXLLAMA_SESSION_OPEN,{h(m),s(cap),h(c)}).handle;
                owner.prepare(session);
                // Intentionally leave values live: VM destruction must drain
                // private contexts before releasing shared model ownership.
            } catch (const std::exception &e) { {std::lock_guard<std::mutex> lock(results_mutex);errors.push_back(e.what());} barrier.fail(); }
        });
        for(auto &thread:workers) thread.join();
        for(auto &error:errors) std::cerr << error << '\n';
        check(errors.empty() && identities.size()==1,"four-VM sharing/preparation/teardown");
        std::cout << "PASS: bridge lifecycle " << argv[1] << " " << argv[4] << " shared_owners=4\n";
        return 0;
    } catch(const std::exception &e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
