/* STEP-04 integration oracle: fixed BGE settings, not an upstream benchmark. */
#define main rxllama_lifecycle_fixture_main
#include "bridge_lifecycle.cpp"
#undef main
#include "llama.h"
#include "ggml-backend.h"
#include <cmath>
#include <memory>

static void parity(const std::vector<double> &a, const std::vector<double> &b,
                   double delta = 0.00001, double minimum_cosine = 0.999999) {
    check(a.size() == b.size() && a.size() % 384 == 0, "embedding shape mismatch");
    for (size_t row = 0; row < a.size(); row += 384) {
        double dot=0, aa=0, bb=0;
        for (size_t d=0; d<384; ++d) {
            check(std::isfinite(a[row+d]) && std::abs(a[row+d]-b[row+d]) <= delta, "embedding coordinate tripwire");
            dot+=a[row+d]*b[row+d]; aa+=a[row+d]*a[row+d]; bb+=b[row+d]*b[row+d];
        }
        check(std::abs(std::sqrt(aa)-1) <= 0.00001 && dot/std::sqrt(aa*bb) >= minimum_cosine, "embedding norm/cosine tripwire");
    }
}
static rxllama_argument text_arg(const std::string &text) {
    auto arg=s(text.c_str()); arg.text_length=text.size(); return arg;
}
static std::vector<double> request(VM &vm, rxllama_token session, rxllama_token config,
                                   const std::vector<std::string> &texts, bool query=false) {
    auto q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(config)}).handle;
    for (size_t row=0; row<texts.size(); ++row)
        check(vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),text_arg(texts[row]),s(query ? "query" : "document")}).integer == int64_t(row+1), "row order");
    vm.call(RXLLAMA_SUBMIT,{h(q)});
    int64_t bytes=0;
    for (const auto &text:texts) bytes+=int64_t(text.size())+(query?int64_t(std::strlen("Represent this sentence for searching relevant passages: ")):0);
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("input_bytes")}).integer==bytes,"complete text byte count");
    check(std::string(vm.call(RXLLAMA_PROCESS,{h(q),i(1)}).text)=="complete", "whole noncausal work unit");
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("decode_calls")}).integer==1,"batch split into multiple decodes");
    auto out=vm.call(RXLLAMA_EMBEDDINGS,{h(q)});
    check(out.rows==int64_t(texts.size()) && out.dimensions==384,"result shape");
    std::vector<double> values(out.values,out.values+out.value_count);
    vm.close(q); return values;
}
struct Reference {
    rxllama_glue_probes *probes = nullptr;
    std::unique_ptr<llama_model,decltype(&llama_model_free)> model{nullptr,llama_model_free};
    std::unique_ptr<llama_context,decltype(&llama_free)> context{nullptr,llama_free};
    Reference(const char *path, bool gpu) {
        ggml_backend_dev_t device=nullptr;
        if(gpu) for(size_t j=0;j<ggml_backend_dev_count();++j) {
            auto d=ggml_backend_dev_get(j);
            if(ggml_backend_dev_type(d)==GGML_BACKEND_DEVICE_TYPE_GPU) {device=d;break;}
        }
        check(!gpu || device,"reference GPU unavailable");
        ggml_backend_dev_t devices[]={device,nullptr};
        auto mp=llama_model_default_params();mp.devices=devices;mp.n_gpu_layers=gpu?999:0;
        model.reset(llama_model_load_from_file(path,mp));check(bool(model),"reference model");
        auto cp=llama_context_default_params(); cp.n_ctx=4096;cp.n_seq_max=8;cp.n_batch=4096;cp.n_ubatch=4096;
        cp.n_threads=2;cp.n_threads_batch=2;cp.embeddings=true;cp.pooling_type=LLAMA_POOLING_TYPE_CLS;
        cp.offload_kqv=gpu;cp.op_offload=gpu;cp.no_perf=false;
        context.reset(llama_init_from_model(model.get(),cp));check(bool(context),"reference context");
    }
    std::vector<double> embed(const std::vector<std::string> &texts) {
        auto token_begin=probes?rxllama_probe_clock_ns():0;
        std::vector<std::vector<llama_token>> rows;
        int total=0;
        for(size_t row=0;row<texts.size();++row) {
            const auto &text=texts[row];auto vocab=llama_model_get_vocab(model.get());
            int count=-llama_tokenize(vocab,text.data(),int(text.size()),nullptr,0,true,false);
            check(count>0 && count<=512,"reference token boundary");
            std::vector<llama_token> tokens(static_cast<size_t>(count));
            check(llama_tokenize(vocab,text.data(),int(text.size()),tokens.data(),count,true,false)==count,"reference tokenize");
            total+=count;rows.push_back(std::move(tokens));
        }
        auto token_end=probes?rxllama_probe_clock_ns():0;
        auto batch=llama_batch_init(total,0,1);
        for(size_t row=0;row<rows.size();++row)
            for(size_t pos=0;pos<rows[row].size();++pos) {auto n=batch.n_tokens++;batch.token[n]=rows[row][pos];batch.pos[n]=int(pos);batch.n_seq_id[n]=1;batch.seq_id[n][0]=int(row);batch.logits[n]=true;}
        if(auto memory=llama_get_memory(context.get())) llama_memory_clear(memory,true);
        auto decode_begin=probes?rxllama_probe_clock_ns():0;
        auto status=llama_decode(context.get(),batch);llama_batch_free(batch);llama_synchronize(context.get());
        auto decode_end=probes?rxllama_probe_clock_ns():0;
        check(status==0,"reference decode");std::vector<double> result(texts.size()*384);
        for(size_t row=0;row<texts.size();++row) {
            auto data=llama_get_embeddings_seq(context.get(),int(row));check(data,"reference output");
            double norm=0;for(int d=0;d<384;++d) norm+=double(data[d])*data[d];
            for(int d=0;d<384;++d) result[row*384+size_t(d)]=data[d]/std::sqrt(norm);
        }
        if(probes) {
            auto normalized=rxllama_probe_clock_ns();
            probes->submit_ns+=token_end-token_begin;
            probes->setup_ns+=decode_begin-token_end;
            probes->decode_ns+=decode_end-decode_begin;
            probes->normalize_ns+=normalized-decode_end;
            ++probes->requests;
        }
        return result;
    }
};
#ifndef RXLLAMA_EMBEDDING_REFERENCE_ONLY
static void request_boundaries(VM &vm, rxllama_token runtime, rxllama_token session,
                               rxllama_token config, Reference &reference, const char *mode) {
    auto reserved=vm.call(RXLLAMA_INFO_INT,{h(runtime),s("reserved_bytes")}).integer;
    vm.reject(RXLLAMA_REQUEST_OPEN,{h(runtime),h(config)});
    vm.reject(RXLLAMA_CANCEL,{h(session)});
    auto limited=vm.call(RXLLAMA_CONFIG_CREATE).handle;
    vm.call(RXLLAMA_CONFIG_TEXT,{h(limited),s("hardware_mode"),s(mode)});
    vm.call(RXLLAMA_CONFIG_INT,{h(limited),s("request_tokens"),i(6)});
    vm.call(RXLLAMA_CONFIG_INT,{h(limited),s("request_rows"),i(2)});
    auto q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(limited)}).handle;
    vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),s(" a"),s("document")});
    vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),s(" a"),s("document")});
    vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s(" a"),s("document")});
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("rows")}).integer==2,"row rejection changed admitted rows");
    vm.call(RXLLAMA_SUBMIT,{h(q)});
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("input_tokens")}).integer==6,"exact aggregate token boundary");
    vm.reject(RXLLAMA_PROCESS,{h(q),i(0)});
    vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s(" a"),s("document")});
    vm.reject(RXLLAMA_SUBMIT,{h(q)});
    vm.call(RXLLAMA_PROCESS,{h(q),i(1)});
    vm.call(RXLLAMA_PROCESS,{h(q),i(1)});
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("decode_calls")}).integer==1,"complete request decoded again");
    vm.call(RXLLAMA_CANCEL,{h(q)});
    check(std::string(vm.call(RXLLAMA_INFO_TEXT,{h(q),s("state")}).text)=="complete","late cancellation destroyed complete result");
    auto output=vm.call(RXLLAMA_EMBEDDINGS,{h(q)});
    parity(std::vector<double>(output.values,output.values+output.value_count),reference.embed({" a"," a"}));
    vm.close(q);
    q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(limited)}).handle;
    vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),s(" a"),s("document")});
    vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),s(" a a"),s("document")});
    vm.reject(RXLLAMA_SUBMIT,{h(q)});
    auto diagnostic=vm.call(RXLLAMA_DIAGNOSTIC);
    check(diagnostic.integer!=0 && std::string(diagnostic.operation)=="submit","typed failed-submit diagnostic");
    check(std::string(vm.call(RXLLAMA_INFO_TEXT,{h(q),s("state")}).text)=="failed","aggregate overflow not failed");
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("decode_calls")}).integer==0,"overflow decoded a partial batch");
    rxllama_result absent{};
    check(vm.raw(RXLLAMA_EMBEDDINGS,{h(q)},absent)!=0 && !absent.values && absent.value_count==0,"failure published partial output");
    auto recovery=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(config)}).handle;
    vm.call(RXLLAMA_CANCEL,{h(recovery)});vm.close(recovery);vm.close(q);vm.close(limited);

    limited=vm.call(RXLLAMA_CONFIG_CREATE).handle;
    vm.call(RXLLAMA_CONFIG_TEXT,{h(limited),s("hardware_mode"),s(mode)});
    vm.call(RXLLAMA_CONFIG_INT,{h(limited),s("request_bytes"),i(3)});
    q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(limited)}).handle;
    vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s(""),s("query")}); // prefix counts against bytes
    vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),s("cat"),s("document")});
    vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s("x"),s("document")});
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("input_bytes")}).integer==3,"byte rejection mutated admitted input");
    vm.call(RXLLAMA_CANCEL,{h(q)});vm.call(RXLLAMA_CANCEL,{h(q)});
    check(std::string(vm.call(RXLLAMA_PROCESS,{h(q),i(1)}).text)=="cancelled","building cancellation state");
    check(vm.call(RXLLAMA_INFO_INT,{h(q),s("decode_calls")}).integer==0,"cancelled request decoded");
    vm.close(q);vm.close(limited);

    std::string prefix="Represent this sentence for searching relevant passages: ";
    std::string exact;
    auto vocab=llama_model_get_vocab(reference.model.get());
    int count=0;
    for(int n=0;n<512 && count<512;++n) {
        exact+=" a";auto prepared=prefix+exact;
        count=-llama_tokenize(vocab,prepared.data(),int(prepared.size()),nullptr,0,true,false);
    }
    check(count==512,"query fixture did not reach exact 512 tokens");
    parity(request(vm,session,config,{exact},true),reference.embed({prefix+exact}));
    q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(config)}).handle;
    auto over=exact+" a";auto prepared=prefix+over;
    check(-llama_tokenize(vocab,prepared.data(),int(prepared.size()),nullptr,0,true,false)==513,"query fixture is not 513 tokens");
    vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),text_arg(over),s("query")});vm.reject(RXLLAMA_SUBMIT,{h(q)});vm.close(q);
    parity(request(vm,session,config,{"recovered"}),reference.embed({"recovered"}));
    check(vm.call(RXLLAMA_INFO_INT,{h(runtime),s("reserved_bytes")}).integer==reserved,"boundary requests leaked reservations");
    std::cout<<"PASS: exact row/token/byte limits, query 512/513, cancellation, failed batch and recovery "<<mode<<'\n';
}

static void cross_device(const char *path, const char *hash) {
    std::vector<std::vector<double>> cpu;
    for(const char *mode:{"cpu","required-gpu"}) {
        VM vm;auto c=vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s(mode)});
        auto rt=vm.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
        auto model=vm.call(RXLLAMA_MODEL_OPEN,{h(rt),s(path),s(hash),s("bge-small-en-v1.5"),h(c)}).handle;
        check(vm.ready(model)=="ready","cross-device model ready");
        auto session=vm.call(RXLLAMA_SESSION_OPEN,{h(model),s("embedding"),h(c)}).handle;vm.prepare(session);
        if(std::string(mode)!="cpu") check(vm.call(RXLLAMA_INFO_INT,{h(model),s("gpu_layers")}).integer>0,"cross-device GPU offload absent");
        size_t cell=0;
        for(int rows:{1,4,8}) for(bool query:{false,true}) {
            std::vector<std::string> texts;
            for(int row=0;row<rows;++row) texts.push_back("A cat sits by the window. Row "+std::to_string(row));
            auto values=request(vm,session,c,texts,query);
            if(std::string(mode)=="cpu") cpu.push_back(std::move(values));
            else parity(values,cpu.at(cell),0.002,0.9999);
            ++cell;
        }
        vm.close(session);vm.close(model);vm.close(rt);vm.close(c);
    }
    std::cout<<"PASS: CPU/GPU embedding parity for document/query layouts 1/4/8; absolute<=0.002 cosine>=0.9999\n";
}

int main(int argc,char **argv) {
    try {
        check(argc==4,"usage: embedding_bridge MODE MODEL SHA256");
        if(std::string(argv[1])=="cross-device") {cross_device(argv[2],argv[3]);return 0;}
        VM vm;auto c=vm.call(RXLLAMA_CONFIG_CREATE).handle;
        vm.call(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),s(argv[1])});
        auto rt=vm.call(RXLLAMA_RUNTIME_OPEN,{h(c)}).handle;
        auto model=vm.call(RXLLAMA_MODEL_OPEN,{h(rt),s(argv[2]),s(argv[3]),s("bge-small-en-v1.5"),h(c)}).handle;
        check(vm.ready(model)=="ready","model load");
        auto session=vm.call(RXLLAMA_SESSION_OPEN,{h(model),s("embedding"),h(c)}).handle;
        vm.reject(RXLLAMA_REQUEST_OPEN,{h(session),h(c)}); vm.prepare(session);
        auto reserved=vm.call(RXLLAMA_INFO_INT,{h(rt),s("reserved_bytes")}).integer;
        Reference reference(argv[2],std::string(argv[1])!="cpu");
        request_boundaries(vm,rt,session,c,reference,argv[1]);
        std::vector<std::string> texts={"The cat is sleeping on a warm windowsill.","Where does the cat sleep?",u8"Bonjour. Café, Straße, 日本語.","","A sleeping cat rests beside the window.","engine engine engine", "Numbers: 17, 29, and 43.","Rain falls over the hills of Scotland."};
        texts[6]=std::string("cat\0 dog",8);
        check(-llama_tokenize(llama_model_get_vocab(reference.model.get()),texts[6].data(),int(texts[6].size()),nullptr,0,true,false)==4,
              "direct NUL tokenizer control used by the cREXX ownership test");
        auto expected=reference.embed(texts);auto batch=request(vm,session,c,texts);parity(batch,expected);
        parity(request(vm,session,c,texts),batch);
        std::vector<double> singles;
        for(const auto &text:texts) {auto values=request(vm,session,c,{text});singles.insert(singles.end(),values.begin(),values.end());}
        parity(singles,batch,0.001,0.99998);
        std::vector<std::string> query={"Where does the cat sleep?"};
        parity(request(vm,session,c,query,true),reference.embed({"Represent this sentence for searching relevant passages: "+query[0]}));
        std::string exact;for(int k=0;k<510;++k) exact+=" a";
        parity(request(vm,session,c,{exact}),reference.embed({exact}));
        auto q=vm.call(RXLLAMA_REQUEST_OPEN,{h(session),h(c)}).handle;
        VM foreign;foreign.reject(RXLLAMA_CANCEL,{h(q)});vm.reject(RXLLAMA_EMBEDDINGS,{h(q)});
        vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s("cat"),s("unsupported")});
        vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s("cat"),text_arg(std::string("document\0suffix",15))});
        vm.reject(RXLLAMA_CONFIG_TEXT,{h(c),s("hardware_mode"),text_arg(std::string("cpu\0suffix",10))});
        // An early NUL must not let a byte-limit violation through admission.
        std::string too_many(1024*1024+1,'x');too_many[1]=0;
        vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),text_arg(too_many),s("document")});
        vm.reject(RXLLAMA_ADD_EMBEDDING,{h(q),s("\xc0\x80"),s("document")});
        auto oversized=exact+" a";vm.call(RXLLAMA_ADD_EMBEDDING,{h(q),s(oversized.c_str()),s("document")});
        vm.reject(RXLLAMA_SUBMIT,{h(q)});check(std::string(vm.call(RXLLAMA_INFO_TEXT,{h(q),s("state")}).text)=="failed","failed request state");
        vm.reject(RXLLAMA_EMBEDDINGS,{h(q)});vm.close(q);
        auto peer=vm.call(RXLLAMA_SESSION_OPEN,{h(model),s("embedding"),h(c)}).handle;vm.prepare(peer);
        parity(request(vm,peer,c,texts),batch);vm.close(peer);
        check(vm.call(RXLLAMA_INFO_INT,{h(rt),s("reserved_bytes")}).integer==reserved,"request reservations retained");
        check(vm.call(RXLLAMA_INFO_INT,{h(model),s("load_count")}).integer==1,"model reloaded");
        vm.close(session);vm.close(model);vm.close(rt);vm.close(c);
        std::cout<<"PASS: embedding glue parity, boundaries, batching, private state and cleanup "<<argv[1]<<'\n';return 0;
    } catch(const std::exception &e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}

#endif
