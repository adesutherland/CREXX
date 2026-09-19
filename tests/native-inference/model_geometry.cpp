/* MIT. Metadata-only admission controls: no weights, downloads or GPU allocation. */
#include "package.hpp"
#include "llama.h"
#include "gguf.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace fs = std::filesystem;
using rxllama_package::json;
constexpr int64_t MiB = 1024 * 1024;
struct Failure : std::runtime_error {
    int code;
    Failure(int c, const std::string &why) : std::runtime_error(why), code(c) {}
};
void require(bool ok, const char *why, int code = -1) { if (!ok) throw Failure(code, why); }
// Minimal configuration input to the production metadata reader. Config parsing
// and resource lifecycle remain covered by the public bridge/toolchain tests.
struct Config {
    std::map<std::string, int64_t> ints{{"memory_bytes", 4096 * MiB}, {"context_tokens", 256}, {"request_rows", 1}};
    std::map<std::string, std::string> texts{{"chat_template", "raw"}, {"system_prompt", ""},
        {"pooling", ""}, {"normalization", ""}, {"query_prefix", ""}, {"document_prefix", ""}};
    std::set<std::string> explicit_texts;
    int64_t i(const char *key) const { return ints.at(key); }
    const std::string &s(const char *key) const { return texts.at(key); }
};
#include "model_spec.h"

int main(int argc, char **argv) {
    try {
        require(argc == 2, "usage: model_geometry PRIVATE_WORK_DIR");
        fs::create_directories(argv[1]);
        const auto path = fs::path(argv[1]) / "geometry.gguf";
        std::unique_ptr<gguf_context, decltype(&gguf_free)> g(gguf_init_empty(), gguf_free);
        gguf_set_val_str(g.get(), "general.architecture", "llama");
        gguf_set_val_str(g.get(), "tokenizer.ggml.model", "gpt2");
        const char *vocab[]{"a", "b"};
        gguf_set_arr_str(g.get(), "tokenizer.ggml.tokens", vocab, 2);
        gguf_set_val_u32(g.get(), "llama.embedding_length", 64);
        gguf_set_val_u32(g.get(), "llama.context_length", 512);
        gguf_set_val_u32(g.get(), "llama.block_count", 2);
        gguf_set_val_u32(g.get(), "llama.attention.head_count", 8);
        gguf_set_val_u32(g.get(), "llama.attention.head_count_kv", 2);
        gguf_set_val_u32(g.get(), "llama.attention.key_length", 8);
        gguf_set_val_u32(g.get(), "llama.attention.value_length", 8);
        gguf_set_val_u32(g.get(), "llama.feed_forward_length", 256);
        Config config;
        auto read = [&] {
            require(gguf_write_to_file(g.get(), rxllama_package::utf8_path(path).c_str(), true), "write fixture");
            return read_model_spec(config, path, "generation");
        };
        auto scalar = read();
        require(scalar.state_width == 64 && scalar.feed_forward == 256, "scalar geometry changed");
        const int64_t expected = 128 * MiB + 256 * 2 * 64 * 16 + 128 * 256 * 16 + 128 * 256 * 8 * 8 + 2 * 8;
        require(model_context_reservation(config, scalar) == expected, "scalar reservation changed");
        std::cout << "PASS scalar geometry and exact reservation\n";
        int failures = 0;
        auto rejects = [&](const char *label) {
            try { (void)read(); std::cerr << "FAIL accepted " << label << '\n'; ++failures; }
            catch (const Failure &) { std::cout << "PASS rejected " << label << '\n'; }
        };
        const char *kv = "llama.attention.head_count_kv";
        for (const std::vector<int64_t> values : {std::vector<int64_t>{1}, {1,2,3}, {}, {1,0}, {1,-1}, {1,9}, {1,INT64_MAX}}) {
            gguf_set_arr_data(g.get(), kv, GGUF_TYPE_INT64, values.data(), values.size());
            rejects("invalid per-layer length/value");
        }
        const uint64_t overflow[]{1, UINT64_MAX};
        gguf_set_arr_data(g.get(), kv, GGUF_TYPE_UINT64, overflow, 2); rejects("unsigned overflow");
        const float noninteger[]{1,2};
        gguf_set_arr_data(g.get(), kv, GGUF_TYPE_FLOAT32, noninteger, 2); rejects("noninteger array");
        gguf_set_arr_str(g.get(), kv, vocab, 2); rejects("string array");
        gguf_set_val_u32(g.get(), kv, 9); rejects("oversized scalar");
        const uint32_t variable[]{1,8};
        gguf_set_arr_data(g.get(), kv, GGUF_TYPE_UINT32, variable, 2);
        try {
            auto per_layer = read();
            require(per_layer.state_width == 128, "reservation ignored largest layer");
            require(model_context_reservation(config, per_layer) > expected, "per-layer reservation underestimated");
            std::cout << "PASS variable per-layer KV geometry and conservative reservation\n";
        } catch (const Failure &e) { std::cerr << "FAIL valid per-layer KV: " << e.what() << '\n'; ++failures; }
        gguf_set_val_u32(g.get(), kv, 2);
        for (const char *field : {"llama.feed_forward_length"}) {
            const uint32_t sizes[]{8,256};
            gguf_set_arr_data(g.get(), field, GGUF_TYPE_UINT32, sizes, 2);
            try { (void)read(); std::cout << "PASS per-layer " << field << '\n'; }
            catch (const Failure &e) { std::cerr << "FAIL valid " << field << ": " << e.what() << '\n'; ++failures; }
            const uint32_t bad[]{8,0}; gguf_set_arr_data(g.get(), field, GGUF_TYPE_UINT32, bad, 2);
            rejects(field); gguf_set_val_u32(g.get(), field, std::string(field).find("feed_forward") != std::string::npos ? 256 : 8);
        }
        for (const char *field : {"llama.attention.key_length", "llama.attention.value_length"}) {
            gguf_set_arr_data(g.get(), field, GGUF_TYPE_UINT32, variable, 2);
            rejects("per-layer key/value width unsupported by pinned engine");
            gguf_set_val_u32(g.get(), field, 8);
        }
        fs::remove(path);
        return failures ? 1 : 0;
    } catch (const std::exception &e) { std::cerr << "FAIL " << e.what() << '\n'; return 1; }
}
