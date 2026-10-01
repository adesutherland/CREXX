/* S5 minimum correctness against a separately loaded direct llama.cpp control. */
#define main rxllama_lifecycle_fixture_main
#include "bridge_lifecycle.cpp"
#undef main
#include "generation_reference.hpp"
#include "generation_utf8.h"
#include <atomic>
static rxllama_argument complete_text(const std::string &text) {
    auto out=s(text.c_str());out.text_length=text.size();return out;
}
static thread_local double maximum_generation_unit_ms=0;
static GenerationReference::Output generate(VM &vm, rxllama_token session, rxllama_token config,
        const std::vector<std::string> &texts, int work=128, const std::string &system="") {
    auto q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(config)}).handle;
    vm.reject(RXLLAMA_REQUEST_OPEN,{h(session),h(config)});
    for(size_t row=0;row<texts.size();++row)
        check(vm.call(RXLLAMA_ADD_PROMPT,{h(q),complete_text(system),complete_text(texts[row])}).integer==int64_t(row+1),"prompt row identity");
    vm.reject(RXLLAMA_EMBEDDINGS,{h(q)});vm.reject(RXLLAMA_READ_TEXT,{h(q),i(1)});
    vm.call(RXLLAMA_SUBMIT,{h(q)});vm.reject(RXLLAMA_ADD_PROMPT,{h(q),s(""),s("late")});
    vm.reject(RXLLAMA_PROCESS,{h(q),i(0)});
    GenerationReference::Output out{std::vector<GenerationReference::Tokens>(texts.size()),std::vector<std::string>(texts.size()),std::vector<std::string>(texts.size())};
    auto begin=Clock::now();
    for(;;) {
        auto unit_begin=Clock::now();
        auto state=std::string(vm.call(RXLLAMA_PROCESS,{h(q),i(work)}).text);
        maximum_generation_unit_ms=std::max(maximum_generation_unit_ms,std::chrono::duration<double,std::milli>(Clock::now()-unit_begin).count());
        check(vm.call(RXLLAMA_INFO_INT,{h(q),s("last_work_tokens")}).integer<=work,"work budget exceeded");
        for(size_t row=0;row<texts.size();++row) {
            auto chunk=vm.call(RXLLAMA_READ_TEXT,{h(q),i(int64_t(row+1))});
            check(chunk.row==int64_t(row+1) && chunk.integer>=0,"chunk identity/count");
            out.text[row].append(chunk.text,chunk.text_length);out.finish[row]=chunk.finish;
            if(chunk.integer)out.tokens[row].insert(out.tokens[row].end(),chunk.token_ids,chunk.token_ids+chunk.integer);
        }
        if(state=="complete")break;
        check(state=="running" && Clock::now()-begin<std::chrono::minutes(10),"generation deadline/state");
    }
    vm.reject(RXLLAMA_READ_TEXT,{h(q),i(0)});vm.reject(RXLLAMA_READ_TEXT,{h(q),i(int64_t(texts.size()+1))});
    auto again=vm.call(RXLLAMA_READ_TEXT,{h(q),i(1)});
    check(again.integer==0 && again.text_length==0 && std::string(again.finish)==out.finish[0],"incremental output replayed");
    vm.close(q);return out;
}
static void equal(const GenerationReference::Output &a,const GenerationReference::Output &b) {
    check(a.tokens==b.tokens,"native/direct generated token mismatch");
    check(a.text==b.text,"native/direct complete output mismatch");
    check(a.finish==b.finish,"native/direct finish mismatch");
}
#include "generation_qualification.hpp"
int main(int argc,char **argv) {
    try {
        if(argc>1 && std::string(argv[1]).find("--memory-")==0)return generation_memory_control(argc,argv);
        // Exact prefix controls independent of a model's chosen output pieces.
        for (auto scalar : {std::string("A"), std::string(1,0), std::string("é"), std::string("日"), std::string("😀")}) {
            for(size_t split=1;split<scalar.size();++split)
                check(rxllama_utf8_prefix("x"+scalar.substr(0,split))==1,"incomplete scalar leaked");
            check(rxllama_utf8_prefix("x"+scalar)==ptrdiff_t(scalar.size()+1),"complete scalar lost");
        }
        for (std::string bad : {"\xc0\xaf", "\xed\xa0\x80", "\xf4\x90\x80\x80", "\xc3x", "\x80"})
            check(rxllama_utf8_prefix(bad)==-1,"invalid scalar accepted");
        bool boundary_only=argc==5 && std::string(argv[4])=="--output-boundary";
        check(argc==4 || argc==6 || boundary_only,"usage: generation_bridge MODE SMOL HASH [BGE HASH|--output-boundary]");
        VM vm;auto c=vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s(argv[1])});
        auto rt=vm.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
        GenerationReference direct(argv[2],std::string(argv[1])!="cpu");
        auto model=vm.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[2]),s(argv[3]),s("smollm2-360m-instruct"),h(c)}).handle;
        check(vm.ready(model)=="ready","generation model ready");
        if(std::string(argv[1])!="cpu")check(vm.call(RXLLAMA_INFO_INT,{h(model),s("gpu_layers")}).integer>0,"generation GPU offload");
        vm.reject(RXLLAMA_SESSION_OPEN,{h(model),s("embedding"),h(c)});
        auto session=vm.call(RXLLAMA_SESSION_OPEN,{h(model),s("generation"),h(c)}).handle;
        vm.reject(RXLLAMA_REQUEST_OPEN,{h(session),h(c)});vm.prepare(session);
        if(boundary_only) {
            generation_output_boundary(vm,session,c,direct,argv[1]);
            vm.close(session);vm.close(model);vm.close(rt);vm.close(c);return 0;
        }
        std::vector<std::string> texts={"Name one animal.","What is two plus two?","Say hello in French.","Name one planet."};
        auto expected=direct.generate(texts);equal(generate(vm,session,c,texts),expected);
        equal(generate(vm,session,c,texts),expected);
        equal(generate(vm,session,c,{texts[0]},1),direct.generate({texts[0]}));
        std::string nul="before";nul.push_back(0);nul+="after";
        equal(generate(vm,session,c,{nul},128,"Reply briefly."),direct.generate({nul},32,"Reply briefly."));
        auto limited=vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(limited),s("hardware_mode"),s(argv[1])});
        vm.call(RXLLAMA_CONFIG_INT,{h(limited),s("output_tokens"),i(1)});
        auto capped=generate(vm,session,limited,{texts[0]});equal(capped,direct.generate({texts[0]},1));
        check(capped.finish[0]=="output_limit" && capped.tokens[0].size()==1,"explicit output limit");
        vm.close(limited);
        for(int phase:{0,1,2}) {
            auto q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(c)}).handle;
            vm.call(RXLLAMA_ADD_PROMPT,{h(q),s(""),s("Name three animals.")});
            if(phase) {vm.call(RXLLAMA_SUBMIT,{h(q)});vm.call(RXLLAMA_PROCESS,{h(q),i(phase==1?1:128)});}
            if(phase==2) {
                vm.call(RXLLAMA_PROCESS,{h(q),i(128)});
                check(property(vm,q,"token_calls")>0 && property_text(vm,q,"state")=="running","cancellation fixture reached active decode");
            }
            vm.call(RXLLAMA_CANCEL,{h(q)});
            check(std::string(vm.call(RXLLAMA_PROCESS,{h(q),i(1)}).text)=="cancelled","generation cancellation");
            auto out=vm.call(RXLLAMA_READ_TEXT,{h(q),i(1)});check(std::string(out.finish)=="cancelled","cancellation finish");
            vm.close(q);
        }
        auto q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(c)}).handle;
        vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s("wrong"),s("document")});
        vm.reject(RXLLAMA_SUBMIT,{h(q)});vm.close(q);
        q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(c)}).handle;
        std::string oversized(10000,'x');vm.call(RXLLAMA_ADD_PROMPT,{h(q),s(""),complete_text(oversized)});
        vm.reject(RXLLAMA_SUBMIT,{h(q)});vm.close(q);
        equal(generate(vm,session,c,texts),expected);
        if(argc==6)generation_qualification(vm,rt,model,session,c,direct,argv);
        check(vm.call(RXLLAMA_INFO_INT,{h(model),s("load_count")}).integer==1,"generation model reloaded");
        vm.close(session);vm.close(model);
        check(vm.call(RXLLAMA_INFO_INT,{h(rt),s("reserved_bytes")}).integer==0,"generation reservation cleanup");
        vm.close(rt);vm.close(c);
        std::cout<<"PASS: generation direct parity, batching, incremental output, limits, cancellation and recovery "<<argv[1]<<'\n';return 0;
    }catch(const std::exception &e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
