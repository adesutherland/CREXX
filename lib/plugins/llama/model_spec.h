/* MIT. Validated GGUF properties and explicit preprocessing for general profiles.
 * Included by bridge.cpp after Config. No model-name or artifact allowlist.
 */
#include <cstring>
#include <type_traits>
struct ModelSpec {
    bool preset = false, embedding = false, normalize = true;
    int dimensions = 0, context = 0, layers = 0, heads = 0, vocabulary = 0;
    int64_t weights = 0, state_width = 0, feed_forward = 0;
    enum llama_pooling_type pooling = LLAMA_POOLING_TYPE_NONE;
    std::string architecture, query_prefix, document_prefix, chat_template, system_prompt;
    json identity;
};
int64_t metadata_integer(const gguf_context *g, const std::string &key, int64_t fallback = -1) {
    auto id = gguf_find_key(g, key.c_str());
    if (id < 0) return fallback;
    switch (gguf_get_kv_type(g, id)) {
    case GGUF_TYPE_UINT32: return gguf_get_val_u32(g, id);
    case GGUF_TYPE_INT32: return gguf_get_val_i32(g, id);
    case GGUF_TYPE_UINT64: {
        auto value = gguf_get_val_u64(g, id);
        require(value <= INT64_MAX, "GGUF integer overflows supported range"); return int64_t(value);
    }
    case GGUF_TYPE_INT64: return gguf_get_val_i64(g, id);
    default: throw Failure(-1, "GGUF property must be a scalar integer: " + key);
    }
}
std::string metadata_string(const gguf_context *g, const std::string &key, bool required = true) {
    auto id = gguf_find_key(g, key.c_str());
    if (id < 0 && !required) return "";
    require(id >= 0, "required GGUF string metadata is missing");
    require(gguf_get_kv_type(g, id) == GGUF_TYPE_STRING, "GGUF property must be a string");
    return gguf_get_val_str(g, id);
}
int metadata_bound(const gguf_context *g, const std::string &key, int maximum) {
    auto n = metadata_integer(g, key);
    require(n > 0 && n <= maximum, "missing or unsupported GGUF model geometry"); return int(n);
}
template<typename T> int64_t metadata_array_integer(const void *data, size_t index) {
    T value;
    std::memcpy(&value, static_cast<const char *>(data) + index * sizeof(T), sizeof(T));
    if constexpr (std::is_unsigned<T>::value)
        require(value <= uint64_t(INT64_MAX), "GGUF integer overflows supported range");
    return int64_t(value);
}
int64_t metadata_layer_bound(const gguf_context *g, const std::string &key,
                             int layers, int64_t fallback, int64_t maximum) {
    const auto id = gguf_find_key(g, key.c_str());
    if (id < 0 || gguf_get_kv_type(g, id) != GGUF_TYPE_ARRAY) {
        const auto value = metadata_integer(g, key, fallback);
        require(value > 0 && value <= maximum, "unsupported per-layer model geometry");
        return value;
    }
    require(gguf_get_arr_n(g, id) == size_t(layers), "GGUF geometry array must contain one value per layer");
    const auto type = gguf_get_arr_type(g, id);
    require(type == GGUF_TYPE_UINT32 || type == GGUF_TYPE_INT32 ||
            type == GGUF_TYPE_UINT64 || type == GGUF_TYPE_INT64,
            "GGUF geometry array must contain integers");
    const auto data = gguf_get_arr_data(g, id);
    int64_t largest = 0;
    for (int layer = 0; layer < layers; ++layer) {
        const auto value = type == GGUF_TYPE_UINT32 ? metadata_array_integer<uint32_t>(data, layer) :
            type == GGUF_TYPE_INT32 ? metadata_array_integer<int32_t>(data, layer) :
            type == GGUF_TYPE_UINT64 ? metadata_array_integer<uint64_t>(data, layer) :
            metadata_array_integer<int64_t>(data, layer);
        require(value > 0 && value <= maximum, "unsupported per-layer model geometry");
        largest = std::max(largest, value);
    }
    // Reserve the largest geometry for every layer. Independent maxima may
    // overestimate heterogeneous layouts; admission must never underestimate.
    return largest;
}
int64_t model_context_reservation(const Config &c, const ModelSpec &s) {
    const int64_t tokens = c.i("context_tokens"), rows = c.i("request_rows");
    if (s.preset) return 128 * MiB + tokens * rows * (s.embedding ? 32768 : 65536);
    // Conservative dense-transformer admission, not a hard upstream allocator cap.
    // Includes float-width KV/activations and the largest admitted attention graph.
    const int64_t query = s.embedding ? tokens : std::min<int64_t>(tokens, 128);
    const int64_t bytes = 128 * MiB + tokens * rows * s.layers * s.state_width * 16 +
        query * rows * s.feed_forward * 16 +
        query * tokens * rows * s.heads * 8 + int64_t(s.vocabulary) * rows * 8;
    require(bytes > 0 && bytes <= (INT64_C(1) << 40), "model context reservation exceeds supported range", -8);
    return bytes;
}
ModelSpec read_model_spec(const Config &c, const fs::path &path, const std::string &profile) {
    ModelSpec s;
    s.preset = profile == "bge-small-en-v1.5" || profile == "smollm2-360m-instruct";
    require(s.preset || profile == "generation" || profile == "embedding", "unknown model profile; use generation or embedding");
    s.embedding = profile == "bge-small-en-v1.5" || profile == "embedding";
    if (s.preset) {
        // Preserve the original reference presets, including their admission limits.
        for (const char *key : {"pooling", "normalization", "query_prefix", "document_prefix", "chat_template", "system_prompt"})
            require(!c.explicit_texts.count(key), "preprocessing overrides require a general profile");
        s.dimensions = s.embedding ? 384 : 960;
        s.context = s.embedding ? 512 : 8192;
        s.weights = (s.embedding ? 160 : 800) * MiB;
        s.pooling = s.embedding ? LLAMA_POOLING_TYPE_CLS : LLAMA_POOLING_TYPE_NONE;
        s.query_prefix = s.embedding ? "Represent this sentence for searching relevant passages: " : "";
        return s;
    }
    const auto file_bytes = fs::file_size(path);
    require(file_bytes > 0 && file_bytes <= (UINT64_C(1) << 39), "unsupported model file size", -8);
    s.weights = int64_t(file_bytes) * 2 + 128 * MiB;
    require(s.weights <= c.i("memory_bytes"), "model weight reservation exceeds configured RAM budget", -8);
    // Metadata-only parsing precedes engine allocation. Model bytes are hashed
    // before sharing lookup (automatic identity) or by the loader (explicit
    // verification); application provisioning must remain immutable.
    std::unique_ptr<gguf_context, decltype(&gguf_free)> g(
        gguf_init_from_file(rxllama_package::utf8_path(path).c_str(), {true, nullptr}), gguf_free);
    require(bool(g), "invalid GGUF metadata");
    s.architecture = metadata_string(g.get(), "general.architecture");
    const auto tokenizer = metadata_string(g.get(), "tokenizer.ggml.model");
    require(tokenizer != "none" && tokenizer != "no_vocab", "GGUF has no text tokenizer; token-only fixtures cannot serve text requests");
    const auto prefix = s.architecture + ".";
    s.dimensions = metadata_bound(g.get(), prefix + "embedding_length", 32768);
    s.context = metadata_bound(g.get(), prefix + "context_length", 16 * 1024 * 1024);
    s.layers = metadata_bound(g.get(), prefix + "block_count", 1024);
    s.heads = metadata_bound(g.get(), prefix + "attention.head_count", 512);
    require(s.dimensions % s.heads == 0, "unsupported attention head geometry");
    require(metadata_integer(g.get(), prefix + "expert_count", 0) == 0, "expert model memory layout is not supported");
    const auto head_width = s.dimensions / s.heads;
    // The pinned engine accepts scalar widths; KV heads and feed-forward
    // widths may be scalar or per-layer arrays (llama-model.cpp get_key_or_arr).
    const auto key_width = metadata_integer(g.get(), prefix + "attention.key_length", head_width);
    const auto value_width = metadata_integer(g.get(), prefix + "attention.value_length", head_width);
    const auto kv_heads = metadata_layer_bound(g.get(), prefix + "attention.head_count_kv", s.layers, s.heads, s.heads);
    require(key_width > 0 && key_width <= 32768 && value_width > 0 && value_width <= 32768,
            "unsupported attention state geometry");
    s.state_width = std::max<int64_t>(s.dimensions, kv_heads * (key_width + value_width));
    s.feed_forward = metadata_layer_bound(g.get(), prefix + "feed_forward_length", s.layers, s.dimensions * 4, 1024 * 1024);
    auto tokens = gguf_find_key(g.get(), "tokenizer.ggml.tokens");
    require(tokens >= 0 && gguf_get_kv_type(g.get(), tokens) == GGUF_TYPE_ARRAY &&
            gguf_get_arr_type(g.get(), tokens) == GGUF_TYPE_STRING, "GGUF tokenizer vocabulary is missing");
    auto vocabulary = gguf_get_arr_n(g.get(), tokens);
    require(vocabulary > 0 && vocabulary <= 1024 * 1024, "unsupported vocabulary size");
    s.vocabulary = int(vocabulary);
    require(c.i("context_tokens") <= s.context, "requested context exceeds model training context");
    if (s.embedding) {
        for (const char *key : {"pooling", "normalization", "query_prefix", "document_prefix"})
            require(c.explicit_texts.count(key), "embedding requires explicit pooling, normalization, query_prefix and document_prefix (empty is allowed)");
        require(c.s("chat_template").empty() && c.s("system_prompt").empty(), "chat settings do not apply to embeddings");
        s.pooling = c.s("pooling") == "cls" ? LLAMA_POOLING_TYPE_CLS :
                    c.s("pooling") == "mean" ? LLAMA_POOLING_TYPE_MEAN : LLAMA_POOLING_TYPE_LAST;
        s.normalize = c.s("normalization") == "l2";
        s.query_prefix = c.s("query_prefix"); s.document_prefix = c.s("document_prefix");
    } else {
        for (const char *key : {"pooling", "normalization", "query_prefix", "document_prefix"})
            require(!c.explicit_texts.count(key), "embedding settings do not apply to generation");
        require(s.architecture.find("bert") == std::string::npos, "noncausal BERT model cannot be used for generation");
        s.chat_template = c.s("chat_template");
        if (s.chat_template.empty()) s.chat_template = metadata_string(g.get(), "tokenizer.chat_template", false);
        require(!s.chat_template.empty(), "no chat template in GGUF; set an explicit supported template or raw");
        s.system_prompt = c.s("system_prompt");
        if (s.chat_template == "raw") require(s.system_prompt.empty(), "raw template does not accept a system prompt");
        else {
            llama_chat_message message{"user", "Hello"};
            require(llama_chat_apply_template(s.chat_template.c_str(), &message, 1, true, nullptr, 0) >= 0,
                    "unsupported GGUF chat template; set a supported template override or raw");
        }
    }
    s.identity = {{"version", 1}, {"profile", profile}, {"architecture", s.architecture},
        {"dimensions", s.dimensions}, {"context_tokens", c.i("context_tokens")},
        {"tokenizer", metadata_string(g.get(), "tokenizer.ggml.model")},
        {"pooling", c.s("pooling")}, {"normalization", c.s("normalization")},
        {"query_prefix", s.query_prefix}, {"document_prefix", s.document_prefix},
        {"chat_template", s.chat_template}, {"system_prompt", s.system_prompt}};
    return s;
}
void validate_loaded_model(const llama_model *model, const ModelSpec &s) {
    require(llama_model_n_embd_out(model) == s.dimensions, "model output dimension differs from validated metadata");
    if (s.preset) return;
    require(!llama_model_has_encoder(model) && llama_model_has_decoder(model) &&
            !llama_model_is_recurrent(model) && !llama_model_is_hybrid(model) && !llama_model_is_diffusion(model),
            "unsupported encoder/recurrent/hybrid/diffusion execution layout");
    require(llama_model_n_embd(model) == s.dimensions && llama_model_n_layer(model) == s.layers &&
            llama_model_n_head(model) == s.heads && llama_model_n_ctx_train(model) == s.context,
            "engine geometry differs from admitted GGUF metadata");
    require(llama_model_size(model) <= uint64_t(s.weights), "loaded weights exceed admission envelope", -8);
}
