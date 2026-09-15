/* Fixed C API control required to isolate cREXX glue. Capture/reduction is Level B. */
#define RXLLAMA_EMBEDDING_REFERENCE_ONLY
#include "embedding_bridge.cpp"
static int64_t micros(std::chrono::steady_clock::time_point begin) {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now()-begin).count();
}
int main(int argc, char **argv) {
    try {
        check(argc==6,"usage: reference MODE MODEL SHA ROWS REPEATS");
        int rows=std::stoi(argv[4]), repeats=std::stoi(argv[5]);
        check((rows==1 || rows==8) && repeats>0,"fixed bounded workload");
        auto started=std::chrono::steady_clock::now();
        // Same packaged backend initialization; no bridge requests/model sharing
        // in the measured direct-library path. Hash verification belongs to the
        // provider's cold load and is deliberately excluded from warm comparison.
        VM vm; auto c=vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s(argv[1])});
        auto rt=vm.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
        Reference reference(argv[2],std::string(argv[1])!="cpu");
        reference.embed({"Warm up."});
        auto prepare=micros(started);
        rxllama_glue_probes probes{};
        const char *enabled=std::getenv("CREXX_LLAMA_GLUE_PROBES");
        bool probing=enabled && std::strcmp(enabled,"1")==0;
        if(probing) reference.probes=&probes;
        std::vector<std::string> texts(size_t(rows),"A cat sits by the window.");
        auto run=[&] {
            auto result=reference.embed(texts);
            // Match the public packed owner's independent copy and lifetime.
            auto copy_begin=probing?rxllama_probe_clock_ns():0;
            std::vector<double> owner(result);
            if(probing) {probes.copy_ns+=rxllama_probe_clock_ns()-copy_begin;probes.bytes+=owner.size()*sizeof(double);++probes.copies;}
            check(owner.size()==size_t(rows*384) && std::isfinite(owner[0]),"reference result");
        };
        started=std::chrono::steady_clock::now(); run(); auto first=micros(started);
        auto before=probes;
        started=std::chrono::steady_clock::now();
        for(int j=0;j<repeats;++j) run();
        auto warm=micros(started);
        if(probing) {
            check(probes.copies-before.copies==uint64_t(repeats) && probes.bytes-before.bytes==uint64_t(repeats*rows*384*8) && probes.requests-before.requests==uint64_t(repeats),"probe copy volume/count");
#define REPORT(field) std::cout<<"PROBE_" #field "="<<(probes.field-before.field)<<'\n';
            REPORT(admit_ns) REPORT(submit_ns) REPORT(setup_ns) REPORT(decode_ns)
            REPORT(normalize_ns) REPORT(copy_ns) REPORT(copies) REPORT(bytes) REPORT(requests)
#undef REPORT
        }
        std::cout<<"PREPARE_US="<<prepare<<"\nFIRST_US="<<first
                 <<"\nWARM_US="<<warm<<" us\nPASS: NI-S4 fixed embedding work "
                 <<rows<<' '<<repeats<<" load_count=1\n";
        vm.close(rt);vm.close(c);return 0;
    } catch(const std::exception &e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
