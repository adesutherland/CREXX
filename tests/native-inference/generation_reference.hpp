/* Independent fixed SmolLM2 oracle. Mirrors the retained STEP-02 upstream API
 * control, not the provider request machine. No provider call performs inference.
 */
#include "llama.h"
#include "ggml-backend.h"
#include <algorithm>
#include <cmath>
#include <memory>
struct GenerationReference {
    using Tokens = std::vector<llama_token>;
    struct Output { std::vector<Tokens> tokens; std::vector<std::string> text, finish; };
    struct Batch {
        llama_batch value;
        explicit Batch(int n): value(llama_batch_init(n, 0, 1)) {}
        ~Batch() { llama_batch_free(value); }
        void add(llama_token token, int pos, int row, bool output) {
            int n=value.n_tokens++; value.token[n]=token; value.pos[n]=pos;
            value.n_seq_id[n]=1; value.seq_id[n][0]=row; value.logits[n]=output;
        }
    };
    std::unique_ptr<llama_model,decltype(&llama_model_free)> model{nullptr,llama_model_free};
    std::unique_ptr<llama_context,decltype(&llama_free)> context{nullptr,llama_free};
    static std::string prompt(const std::string &user, const std::string &system="") {
        return "<|im_start|>system\n"+(system.empty()?std::string("You are a helpful AI assistant named SmolLM, trained by Hugging Face"):system)+
               "<|im_end|>\n<|im_start|>user\n"+user+"<|im_end|>\n<|im_start|>assistant\n";
    }
    Tokens tokenize(const std::string &text, bool special=true) {
        auto vocab=llama_model_get_vocab(model.get());
        int count=-llama_tokenize(vocab,text.data(),int(text.size()),nullptr,0,true,special);
        check(count>0,"reference token size"); Tokens tokens(static_cast<size_t>(count));
        check(llama_tokenize(vocab,text.data(),int(text.size()),tokens.data(),count,true,special)==count,"reference tokenize");
        return tokens;
    }
    void clear() { if(auto memory=llama_get_memory(context.get())) llama_memory_clear(memory,true); }
    void decode(Batch &batch) {
        int status=llama_decode(context.get(),batch.value); llama_synchronize(context.get());
        check(status==0,"reference decode");
    }
    llama_token greedy(int output) {
        auto values=llama_get_logits_ith(context.get(),output);
        check(values!=nullptr,"reference logits"); int n=llama_vocab_n_tokens(llama_model_get_vocab(model.get()));
        for(int j=0;j<n;++j) check(std::isfinite(values[j]),"reference finite logits");
        return llama_token(std::max_element(values,values+n)-values);
    }
    GenerationReference(const char *path, bool gpu) {
        ggml_backend_dev_t device=nullptr;
        if(gpu) for(size_t j=0;j<ggml_backend_dev_count();++j) {
            auto d=ggml_backend_dev_get(j);auto type=ggml_backend_dev_type(d);
            if(type==GGML_BACKEND_DEVICE_TYPE_GPU || type==GGML_BACKEND_DEVICE_TYPE_IGPU) {device=d;break;}
        }
        check(!gpu || device,"reference GPU unavailable");
        ggml_backend_dev_t devices[]={device,nullptr};
        auto mp=llama_model_default_params();mp.devices=devices;mp.n_gpu_layers=gpu?999:0;
        model.reset(llama_model_load_from_file(path,mp));check(bool(model),"reference model");
        auto cp=llama_context_default_params();cp.n_ctx=4096;cp.n_seq_max=8;cp.n_batch=512;cp.n_ubatch=128;
        cp.n_outputs_max=8;cp.n_outputs_max_per_seq=1;cp.n_threads=gpu?2:4;cp.n_threads_batch=cp.n_threads;
        cp.embeddings=false;cp.pooling_type=LLAMA_POOLING_TYPE_NONE;
        cp.offload_kqv=gpu;cp.op_offload=gpu;cp.no_perf=false;
        context.reset(llama_init_from_model(model.get(),cp));check(bool(context),"reference context");
        auto warm=tokenize("Warm up.",false);Batch batch(int(warm.size()));
        for(size_t j=0;j<warm.size();++j)batch.add(warm[j],int(j),0,j+1==warm.size());
        decode(batch);check(llama_get_logits_ith(context.get(),-1)!=nullptr,"reference preparation logits");clear();
    }
    Output generate(const std::vector<std::string> &texts, int maximum=32, const std::string &system="") {
        std::vector<Tokens> inputs;
        for(auto &text:texts)inputs.push_back(tokenize(prompt(text,system)));
        check(!inputs.empty() && inputs.size()<=8,"reference rows");
        Batch batch(4096);std::vector<int> logits(inputs.size()),positions(inputs.size());
        for(size_t row=0;row<inputs.size();++row) {
            check(inputs[row].size()+size_t(maximum)<=512,"reference context bound");
            for(size_t pos=0;pos<inputs[row].size();++pos) batch.add(inputs[row][pos],int(pos),int(row),pos+1==inputs[row].size());
            positions[row]=int(inputs[row].size());
        }
        clear();std::vector<llama_token> first_tokens(inputs.size());Batch unit(128);
        for(int first=0;first<batch.value.n_tokens;first+=128) {
            unit.value.n_tokens=0;
            for(int j=first;j<std::min(batch.value.n_tokens,first+128);++j)
                unit.add(batch.value.token[j],batch.value.pos[j],batch.value.seq_id[j][0],batch.value.logits[j]);
            decode(unit);
            for(int j=0;j<unit.value.n_tokens;++j)if(unit.value.logits[j])first_tokens[size_t(unit.value.seq_id[j][0])]=greedy(j);
        }
        Output out{std::vector<Tokens>(texts.size()),std::vector<std::string>(texts.size()),std::vector<std::string>(texts.size())};
        auto vocab=llama_model_get_vocab(model.get());
        for(int step=0;step<maximum;++step) {
            batch.value.n_tokens=0;
            for(size_t row=0;row<inputs.size();++row) {
                if(!out.finish[row].empty())continue;
                auto next=step==0?first_tokens[row]:greedy(logits[row]);
                if(llama_vocab_is_eog(vocab,next)){out.finish[row]="eos";continue;}
                out.tokens[row].push_back(next);
                if(step+1==maximum){out.finish[row]="output_limit";continue;}
                logits[row]=batch.value.n_tokens;batch.add(next,positions[row]++,int(row),true);
            }
            if(batch.value.n_tokens==0)break;
            decode(batch);
        }
        for(size_t row=0;row<inputs.size();++row) for(auto token:out.tokens[row]) {
            int count=llama_token_to_piece(vocab,token,nullptr,0,0,false);
            if(count==0)continue;
            check(count<0,"reference piece size");std::vector<char> piece(static_cast<size_t>(-count));
            check(llama_token_to_piece(vocab,token,piece.data(),int(piece.size()),0,false)==int(piece.size()),"reference piece");
            out.text[row].append(piece.data(),piece.size());
        }
        return out;
    }
};
