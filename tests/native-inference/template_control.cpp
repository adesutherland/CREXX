// Independent pinned-GGUF template/token oracle using upstream's Jinja engine.
#include "llama.h"
#include "ggml-backend.h"
#include "chat.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

static void require(bool value, const char * message) {
    if (!value) throw std::runtime_error(message);
}
static std::vector<llama_token> tokens(const llama_vocab * vocab, const std::string & text) {
    int count = llama_tokenize(vocab,text.data(),int(text.size()),nullptr,0,true,true);
    require(count < 0,"tokenizer sizing failed");
    std::vector<llama_token> result(size_t(-count));
    require(llama_tokenize(vocab,text.data(),int(text.size()),result.data(),int(result.size()),true,true) == -count,"tokenizer count changed");
    return result;
}
int main(int argc, char ** argv) {
    try {
        require(argc == 4,"template_control MODEL CPU_BACKEND_FILE OUTPUT");
        require(ggml_backend_load(argv[2]) != nullptr,"CPU backend load failed");
        llama_backend_init();
        auto params = llama_model_default_params(); params.vocab_only = true; params.n_gpu_layers = 0;
        std::unique_ptr<llama_model,decltype(&llama_model_free)> model(llama_model_load_from_file(argv[1],params),llama_model_free);
        require(bool(model),"vocabulary model load failed");
        const char * raw = llama_model_chat_template(model.get(),nullptr);
        require(raw != nullptr,"pinned template absent");
        auto tmpls = common_chat_templates_init(model.get(),"");
        nlohmann::ordered_json result = {{"scope","upstream Jinja versus approved explicit system/user rendering"},{"gguf_template",raw}};
        auto & cases = result["cases"] = nlohmann::ordered_json::array();
        for (const auto & item : std::vector<std::pair<std::string,std::string>>{
                {"","Name one animal."}, {"Answer briefly.","Name one animal."},
                {"",""}, {"","Caf\u00e9, Stra\u00dfe, \u65e5\u672c\u8a9e.\nSecond line."}}) {
            common_chat_templates_inputs input;
            if (!item.first.empty()) input.messages.push_back({"system",item.first});
            input.messages.push_back({"user",item.second});
            input.use_jinja = true; input.add_generation_prompt = true;
            std::string rendered = common_chat_templates_apply(tmpls.get(),input).prompt;
            std::string system = item.first.empty() ? "You are a helpful AI assistant named SmolLM, trained by Hugging Face" : item.first;
            std::string explicit_render = "<|im_start|>system\n"+system+"<|im_end|>\n<|im_start|>user\n"+item.second+"<|im_end|>\n<|im_start|>assistant\n";
            require(rendered == explicit_render,"rendered template differs from pinned contract");
            auto actual = tokens(llama_model_get_vocab(model.get()),rendered);
            require(actual == tokens(llama_model_get_vocab(model.get()),explicit_render),"rendered token mismatch");
            cases.push_back({{"system",item.first},{"user",item.second},{"rendered",rendered},{"token_ids",actual}});
        }
        result["result"] = "PASS";
        std::ofstream out(argv[3]); out << result.dump(2) << '\n'; out.close(); require(bool(out),"output failed");
        tmpls.reset(); model.reset(); llama_backend_free();
        std::cout << "PASS native inference template control\n";
        return 0;
    } catch (const std::exception & error) {
        std::cerr << "FAIL template control: " << error.what() << '\n'; return 1;
    }
}
