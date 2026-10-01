/* MIT. Standalone generated dense model and byte vocabulary. No trained artifact,
 * downloaded tokenizer, corpus or checkpoint is an input to this fixture.
 */
#include "gguf.h"
#include "ggml.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
int main(int argc, char **argv) {
    try {
        if (argc != 2) throw std::runtime_error("usage: generate_text_fixture OUTPUT_GGUF");
        auto g = gguf_init_empty();
        auto ctx = ggml_init({16*1024*1024, nullptr, false});
        if (!g || !ctx) throw std::runtime_error("fixture allocation");
        gguf_set_val_str(g,"general.architecture","llama");
        gguf_set_val_str(g,"general.name","cREXX generated text fixture");
        gguf_set_val_u32(g,"llama.context_length",2048);
        gguf_set_val_u32(g,"llama.embedding_length",64);
        gguf_set_val_u32(g,"llama.feed_forward_length",172);
        gguf_set_val_u32(g,"llama.block_count",5);
        gguf_set_val_u32(g,"llama.attention.head_count",8);
        gguf_set_val_u32(g,"llama.attention.head_count_kv",4);
        gguf_set_val_u32(g,"llama.rope.dimension_count",8);
        gguf_set_val_f32(g,"llama.attention.layer_norm_rms_epsilon",0.00001f);
        std::vector<std::string> words={"<unk>","<s>","</s>"};
        std::vector<int32_t> types={2,3,3};
        for (int i=0;i<256;++i) { char word[7]; std::snprintf(word,sizeof(word),"<0x%02X>",i); words.emplace_back(word); types.push_back(6); }
        words.emplace_back("\xe2\x96\x81"); types.push_back(1); // SentencePiece space marker.
        while(words.size()<512) {words.push_back("[unused"+std::to_string(words.size())+"]"); types.push_back(5);}
        std::vector<const char *> strings; for(const auto &w:words) strings.push_back(w.c_str());
        std::vector<float> scores(512,0.0f);
        gguf_set_val_str(g,"tokenizer.ggml.model","llama");
        gguf_set_arr_str(g,"tokenizer.ggml.tokens",strings.data(),strings.size());
        gguf_set_arr_data(g,"tokenizer.ggml.scores",GGUF_TYPE_FLOAT32,scores.data(),scores.size());
        gguf_set_arr_data(g,"tokenizer.ggml.token_type",GGUF_TYPE_INT32,types.data(),types.size());
        gguf_set_val_u32(g,"tokenizer.ggml.unknown_token_id",0);
        gguf_set_val_u32(g,"tokenizer.ggml.bos_token_id",1);
        gguf_set_val_u32(g,"tokenizer.ggml.eos_token_id",2);
        gguf_set_val_bool(g,"tokenizer.ggml.add_bos_token",true);
        gguf_set_val_bool(g,"tokenizer.ggml.add_eos_token",false);
        uint32_t seed=1234;
        auto tensor=[&](const std::string &name,int64_t x,int64_t y=0) {
            auto t=y?ggml_new_tensor_2d(ctx,GGML_TYPE_F32,x,y):ggml_new_tensor_1d(ctx,GGML_TYPE_F32,x);
            ggml_set_name(t,name.c_str()); auto values=static_cast<float *>(t->data);
            for(int64_t i=0;i<ggml_nelements(t);++i) {
                seed=seed*1664525u+1013904223u;
                values[i]=name.find("norm")!=std::string::npos?1.0f:float(int(seed%10001)-5000)/250000.0f;
                if(name=="output.weight") values[i]=0.0f; // Stable, finite equal logits.
            }
            gguf_add_tensor(g,t);
        };
        tensor("token_embd.weight",64,512); tensor("output_norm.weight",64); tensor("output.weight",64,512);
        for(int i=0;i<5;++i) {
            auto p="blk."+std::to_string(i)+".";
            tensor(p+"attn_norm.weight",64); tensor(p+"attn_q.weight",64,64);
            tensor(p+"attn_k.weight",64,32); tensor(p+"attn_v.weight",64,32);
            tensor(p+"attn_output.weight",64,64); tensor(p+"ffn_norm.weight",64);
            tensor(p+"ffn_gate.weight",64,172); tensor(p+"ffn_down.weight",172,64); tensor(p+"ffn_up.weight",64,172);
        }
        if(!gguf_write_to_file(g,argv[1],false)) throw std::runtime_error("write");
        gguf_free(g); ggml_free(ctx);
        std::cout<<"Generated independent random weights and byte vocabulary\n";
    } catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
}
