/* Fixed direct-library control. Capture and reduction use the Level B tools. */
#define main rxllama_lifecycle_fixture_main
#include "bridge_lifecycle.cpp"
#undef main
#include "generation_reference.hpp"
#include <iomanip>
static int64_t micros(Clock::time_point begin) {
    return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-begin).count();
}
int main(int argc, char **argv) {
    try {
        check(argc==6,"usage: reference MODE MODEL SHA ROWS REPEATS");
        int rows=std::stoi(argv[4]),repeats=std::stoi(argv[5]);
        check((rows==1 || rows==4) && repeats>0,"fixed workload");
        auto started=Clock::now();
        VM vm;auto c=vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s(argv[1])});
        auto rt=vm.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
        GenerationReference reference(argv[2],std::string(argv[1])!="cpu");
        auto prepare=micros(started);
        std::vector<std::string> texts(size_t(rows),"Name one animal.");
        GenerationReference::Output result;
        auto run=[&] {
            result=reference.generate(texts);
            check(result.text.size()==size_t(rows),"result row count");
            for(int j=0;j<rows;++j)check(!result.text[j].empty() && !result.finish[j].empty(),"complete output");
        };
        started=Clock::now();run();auto first=micros(started);
        started=Clock::now();for(int j=0;j<repeats;++j)run();auto warm=micros(started);
        size_t tokens=0;for(auto &row:result.tokens)tokens+=row.size();
        std::cout<<"PREPARE_US="<<prepare<<"\nFIRST_US="<<first<<"\nWARM_US="<<warm<<" us\nTOKENS="<<tokens<<'\n';
        for(int j=0;j<rows;++j) {
            std::cout<<"TEXT"<<j+1<<'=';
            for(unsigned char ch:result.text[j])std::cout<<std::hex<<std::uppercase<<std::setw(2)<<std::setfill('0')<<unsigned(ch);
            std::cout<<std::dec<<"\nFINISH"<<j+1<<'='<<result.finish[j]<<'\n';
        }
        std::cout<<"PASS: NI-S5 fixed generation work "<<rows<<' '<<repeats<<" load_count=1\n";
        vm.close(rt);vm.close(c);return 0;
    } catch(const std::exception &e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
