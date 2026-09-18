/* MIT. Generic profiles: no artifact/name allowlist, dynamic output and cleanup. */
#include "bridge.h"
#include "llama.h"
#include "ggml-backend.h"
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <filesystem>
using Args = std::array<rxllama_argument, 6>;
static rxllama_argument h(rxllama_token t) { rxllama_argument a{}; a.handle=t; return a; }
static rxllama_argument s(const char *t) { rxllama_argument a{}; a.text=t; a.text_length=std::strlen(t); return a; }
static rxllama_argument i(int64_t n) { rxllama_argument a{}; a.integer=n; return a; }
static void check(bool ok, const char *why) { if (!ok) throw std::runtime_error(why); }
struct Host {
    void *vm = rxllama_vm_create();
    ~Host() { rxllama_vm_destroy(vm); }
    rxllama_result call(int op, Args args={}) {
        rxllama_result result{};
        if (rxllama_call(vm,op,args.data(),&result)) {
            rxllama_call(vm,RXLLAMA_DIAGNOSTIC,args.data(),&result);
            throw std::runtime_error(std::string(result.operation)+": "+result.message);
        }
        return result;
    }
    void close(rxllama_token t) { call(RXLLAMA_CLOSE,{h(t)}); rxllama_release(t); }
};
static void negative_controls(const char *path, const char *hash) {
    for (const std::string kind : {"hash", "malformed-hash", "missing-file", "preset", "template", "missing-template", "budget", "context", "pooling", "tokenizer"}) {
        Host host; auto c=host.call(RXLLAMA_CONFIG_CREATE).handle;
        host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s("cpu")});
        if(kind!="missing-template" && kind!="preset") host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("chat_template"),s(kind=="template"?"unsupported-template-xyz":"raw")});
        if(kind=="budget") host.call(RXLLAMA_CONFIG_INT,{h(c),s("memory_bytes"),i(64*1024*1024)});
        if(kind=="context") host.call(RXLLAMA_CONFIG_INT,{h(c),s("context_tokens"),i(8192)});
        auto rt=host.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
        auto input=kind=="tokenizer"?(std::filesystem::path(path).parent_path()/"llama-dense.gguf").string():std::string(path);
        if(kind=="missing-file") input += ".absent";
        auto expected=kind=="tokenizer"?"bb9b0debefdfec589f4c6d94c6bcc38daea64ab20b48eb95809e5c367863f5de":hash;
        if(kind=="hash") expected="0000000000000000000000000000000000000000000000000000000000000000";
        if(kind=="malformed-hash") expected="invalid";
        if(kind=="missing-file" || kind=="preset") expected="";
        std::string error;
        try {
            auto m=host.call(RXLLAMA_MODEL_OPEN,{h(rt),s(input.c_str()),s(expected),s(kind=="preset"?"smollm2-360m-instruct":kind=="pooling"?"embedding":"generation"),h(c)}).handle;
            const auto until=std::chrono::steady_clock::now()+std::chrono::minutes(5);
            std::string state;
            do {state=host.call(RXLLAMA_MODEL_STATE,{h(m)}).text; check(std::chrono::steady_clock::now()<until,"negative load hang"); std::this_thread::yield();} while(state=="loading");
            check(state=="failed","invalid model unexpectedly ready");
            error=host.call(RXLLAMA_INFO_TEXT,{h(m),s("error")}).text;
        } catch(const std::exception &e) { error=e.what(); }
        const char *needle=(kind=="hash" || kind=="malformed-hash")?"SHA-256":kind=="missing-file"?"file":kind=="preset"?"pinned model profile":kind=="budget"?"budget":kind=="context"?"context":kind=="pooling"?"explicit":kind=="tokenizer"?"tokenizer":"template";
        check(error.find(needle)!=std::string::npos,(kind+": wrong diagnostic: "+error).c_str());
        std::cout<<"PASS: rejected "<<kind<<" with actionable diagnostic\n";
    }
}
static std::vector<double> reference_embedding(const char *path, int dimensions, enum llama_pooling_type pooling) {
    ggml_backend_dev_t devices[]={nullptr};
    auto mp=llama_model_default_params(); mp.devices=devices; mp.n_gpu_layers=0;
    std::unique_ptr<llama_model,decltype(&llama_model_free)> model(llama_model_load_from_file(path,mp),llama_model_free);
    check(bool(model),"reference model");
    auto cp=llama_context_default_params(); cp.n_ctx=512; cp.n_seq_max=2; cp.n_batch=512; cp.n_ubatch=512;
    cp.n_threads=2; cp.n_threads_batch=2; cp.embeddings=true; cp.pooling_type=pooling; cp.offload_kqv=false; cp.op_offload=false;
    std::unique_ptr<llama_context,decltype(&llama_free)> context(llama_init_from_model(model.get(),cp),llama_free);
    check(bool(context),"reference context");
    auto batch=llama_batch_init(512,0,1);
    int row=0;
    for(const std::string text : {"query: Hello","World"}) {
        auto vocab=llama_model_get_vocab(model.get());
        int n=-llama_tokenize(vocab,text.data(),int(text.size()),nullptr,0,true,false);
        check(n>0 && n<=256,"reference token bound");
        std::vector<llama_token> tokens(static_cast<size_t>(n));
        check(llama_tokenize(vocab,text.data(),int(text.size()),tokens.data(),n,true,false)==n,"reference tokenization");
        for(int j=0;j<n;++j) {auto index=batch.n_tokens++;batch.token[index]=tokens[j];batch.pos[index]=j;batch.n_seq_id[index]=1;batch.seq_id[index][0]=row;batch.logits[index]=true;}
        ++row;
    }
    auto status=llama_decode(context.get(),batch); llama_batch_free(batch); check(status==0,"reference decode");
    llama_synchronize(context.get()); std::vector<double> values(2*dimensions);
    for(row=0;row<2;++row) {
        auto data=llama_get_embeddings_seq(context.get(),row); check(data,"reference pooled output");
        double norm=0;for(int d=0;d<dimensions;++d) norm+=double(data[d])*data[d];
        check(norm>0 && std::isfinite(norm),"reference norm");
        for(int d=0;d<dimensions;++d) values[row*dimensions+d]=data[d]/std::sqrt(norm);
    }
    return values;
}
int main(int argc, char **argv) {
    try {
        check(argc==5 || argc==6,"usage: MODEL SHA256 generation|embedding|negative EXPECTED_DIMENSIONS [cls|mean|last]");
        if(std::string(argv[3])=="negative") { negative_controls(argv[1],argv[2]); return 0; }
        const bool embedding=std::string(argv[3])=="embedding";
        const char *pooling=argc==6?argv[5]:"mean";
        Host host; auto c=host.call(RXLLAMA_CONFIG_CREATE).handle;
        host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s("cpu")});
        host.call(RXLLAMA_CONFIG_INT,{h(c),s("request_tokens"),i(512)});
        host.call(RXLLAMA_CONFIG_INT,{h(c),s("request_rows"),i(2)});
        host.call(RXLLAMA_CONFIG_INT,{h(c),s("context_tokens"),i(256)});
        host.call(RXLLAMA_CONFIG_INT,{h(c),s("output_tokens"),i(4)});
        if (embedding) {
            host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("pooling"),s(pooling)});
            host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("normalization"),s("l2")});
            host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("query_prefix"),s("query: ")});
            host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("document_prefix"),s("")});
        } else host.call(RXLLAMA_CONFIG_TEXT,{h(c),s("chat_template"),s("raw")});
        auto rt=host.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
        auto m=host.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[1]),s(""),s(argv[3]),h(c)}).handle;
        check(std::string(host.call(RXLLAMA_INFO_TEXT,{h(m),s("sha256")}).text)==argv[2],"inferred content hash");
        auto pinned=host.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[1]),s(argv[2]),s(argv[3]),h(c)}).handle;
        const std::string shared_id=host.call(RXLLAMA_INFO_TEXT,{h(m),s("shared_model_id")}).text;
        check(std::string(host.call(RXLLAMA_INFO_TEXT,{h(pinned),s("shared_model_id")}).text)==shared_id,"inferred and explicit owners share weights");
        auto inferred=host.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[1]),s(""),s(argv[3]),h(c)}).handle;
        check(std::string(host.call(RXLLAMA_INFO_TEXT,{h(inferred),s("shared_model_id")}).text)==shared_id,"repeated inferred owner shares weights");
        host.close(inferred); host.close(pinned);
        auto deadline=std::chrono::steady_clock::now()+std::chrono::minutes(5);
        std::string state;
        do {
            state=host.call(RXLLAMA_MODEL_STATE,{h(m)}).text;
            check(std::chrono::steady_clock::now()<deadline,"model hang backstop");
            std::this_thread::yield();
        } while(state=="loading");
        if(state!="ready") throw std::runtime_error(host.call(RXLLAMA_INFO_TEXT,{h(m),s("error")}).text);
        check(host.call(RXLLAMA_INFO_INT,{h(m),s("dimensions")}).integer==std::stoi(argv[4]),"model dimensions");
        auto session=host.call(RXLLAMA_SESSION_OPEN,{h(m),s(argv[3]),h(c)}).handle;
        do {
            state=host.call(RXLLAMA_PREPARE,{h(session),i(128)}).text;
            check(std::chrono::steady_clock::now()<deadline,"prepare hang backstop");
        } while(state!="ready");
        auto reference=embedding?reference_embedding(argv[1],std::stoi(argv[4]),std::string(pooling)=="cls"?LLAMA_POOLING_TYPE_CLS:std::string(pooling)=="last"?LLAMA_POOLING_TYPE_LAST:LLAMA_POOLING_TYPE_MEAN):std::vector<double>{};
        for(int repeat=0;repeat<2;++repeat) {
            auto q=host.call(RXLLAMA_REQUEST_OPEN,{h(session),h(c)}).handle;
            if(embedding) {
                host.call(RXLLAMA_ADD_EMBEDDING,{h(q),s("Hello"),s("query")});
                host.call(RXLLAMA_ADD_EMBEDDING,{h(q),s("World"),s("document")});
            } else host.call(RXLLAMA_ADD_PROMPT,{h(q),s(""),s("Hello")});
            host.call(RXLLAMA_SUBMIT,{h(q)});
            do {
                state=host.call(RXLLAMA_PROCESS,{h(q),i(128)}).text;
                check(std::chrono::steady_clock::now()<deadline,"request hang backstop");
            } while(state=="running");
            check(state=="complete","complete request");
            if(embedding) {
                auto result=host.call(RXLLAMA_EMBEDDINGS,{h(q)});
                check(result.rows==2 && result.dimensions==std::stoi(argv[4]),"dynamic result shape");
                for(int row=0;row<2;++row) {
                    double norm=0;
                    for(int d=0;d<result.dimensions;++d) { auto v=result.values[row*result.dimensions+d]; check(std::isfinite(v),"finite embedding"); norm+=v*v; check(std::abs(v-reference[row*result.dimensions+d])<0.00001,"matched upstream embedding coordinate"); }
                    check(std::abs(norm-1)<0.00001,"L2 normalization");
                }
            } else check(host.call(RXLLAMA_READ_TEXT,{h(q),i(1)}).finish[0]!=0,"generation finish");
            host.close(q);
        }
        check(host.call(RXLLAMA_INFO_INT,{h(m),s("load_count")}).integer==1,"persistent weights");
        host.close(session); host.close(m);
        check(host.call(RXLLAMA_INFO_INT,{h(rt),s("reserved_bytes")}).integer==0,"reservation cleanup");
        host.close(rt); host.close(c);
        std::cout<<"PASS: generic "<<argv[3]<<" dimensions="<<argv[4]<<" repeated requests and cleanup\n";
        return 0;
    } catch(const std::exception &e) { std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1; }
}
