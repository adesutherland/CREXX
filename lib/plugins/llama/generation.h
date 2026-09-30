/* cREXX License (MIT). Private STEP-05 request processing, included by bridge.cpp.
 * One native decode per process call; all state belongs to the request/session.
 */
std::string generation_prompt(const std::string &system, const std::string &prompt) {
    return "<|im_start|>system\n" + (system.empty()
        ? "You are a helpful AI assistant named SmolLM, trained by Hugging Face" : system) +
        "<|im_end|>\n<|im_start|>user\n" + prompt +
        "<|im_end|>\n<|im_start|>assistant\n";
}
std::string model_prompt(const ModelSpec &spec, const std::string &system, const std::string &prompt, size_t available) {
    if (spec.preset) return generation_prompt(system, prompt);
    const auto &sys = system.empty() ? spec.system_prompt : system;
    if (spec.chat_template == "raw") {
        require(sys.empty(), "raw template does not accept a system prompt"); return prompt;
    }
    // The engine API accepts C strings. Never silently truncate embedded NULs.
    require(sys.find('\0') == std::string::npos && prompt.find('\0') == std::string::npos,
            "chat templates cannot encode embedded NUL; use raw for literal text");
    std::vector<llama_chat_message> messages;
    if (!sys.empty()) messages.push_back({"system", sys.c_str()});
    messages.push_back({"user", prompt.c_str()});
    auto size = llama_chat_apply_template(spec.chat_template.c_str(), messages.data(), messages.size(), true, nullptr, 0);
    require(size >= 0, "unsupported chat template");
    require(size_t(size) <= available, "rendered prompt exceeds request byte limit", -8);
    std::string rendered(size_t(size), '\0');
    require(llama_chat_apply_template(spec.chat_template.c_str(), messages.data(), messages.size(), true,
        rendered.data(), size) == size, "chat template rendering failed");
    return rendered;
}
void add_prompt(Resource &r, const rxllama_argument &system, const rxllama_argument &prompt,
                rxllama_result &out) {
    auto &q = *r.request;
    require(q.generation, "generation operation on an embedding request", -4);
    require(q.state == "building", "request is not building", -7);
    require(system.text && prompt.text, "missing prompt input");
    require(q.rows.size() < size_t(r.config.i("request_rows")), "request row limit exceeded", -8);
    auto available = size_t(r.config.i("request_bytes") - q.bytes);
    require(system.text_length <= available && prompt.text_length <= available - system.text_length,
            "request byte limit exceeded", -8);
    std::string sys(system.text, system.text_length), user(prompt.text, prompt.text_length);
    valid_utf8(sys); valid_utf8(user);
    EmbeddingRow row; row.text = model_prompt(r.model->spec, sys, user, available);
    require(row.text.size() <= available, "rendered prompt exceeds request byte limit", -8);
    auto bytes = int64_t(row.text.size());
    q.rows.push_back(std::move(row)); q.bytes += bytes; out.integer = int64_t(q.rows.size());
}
llama_token generation_greedy(llama_context *ctx, const llama_vocab *vocab, int output) {
    const auto values = llama_get_logits_ith(ctx, output);
    require(values != nullptr, "missing generation logits", -6);
    const int count = llama_vocab_n_tokens(vocab);
    require(count > 0, "empty generation vocabulary", -6);
    for (int j = 0; j < count; ++j) require(std::isfinite(values[j]), "nonfinite generation logits", -6);
    return llama_token(std::max_element(values, values + count) - values);
}
void generation_sample(Resource &r, size_t index, const llama_vocab *vocab, llama_token token) {
    auto &q = *r.request; auto &row = q.generated[index];
    if (llama_vocab_is_eog(vocab, token)) row.finish = "eos";
    else {
        char token_piece[256];
        int length = llama_token_to_piece(vocab, token, token_piece, sizeof(token_piece), 0, false);
        require(length != INT32_MIN, "generation detokenizer size failed", -6);
        if (length >= 0) row.partial.append(token_piece, size_t(length));
        else {
            auto count = size_t(-length);
            require(count <= size_t(r.config.i("request_bytes") - q.output_bytes), "generation output byte limit exceeded", -8);
            std::vector<char> piece(count);
            require(llama_token_to_piece(vocab, token, piece.data(), int(count), 0, false) == int(count),
                    "generation detokenizer failed", -6);
            row.partial.append(piece.data(), count); length = int(count);
        }
        require(length <= r.config.i("request_bytes") - q.output_bytes, "generation output byte limit exceeded", -8);
        q.output_bytes += length;
        auto complete = rxllama_utf8_prefix(row.partial);
        require(complete >= 0, "generation produced invalid UTF-8", -6);
        row.text.append(row.partial, 0, complete); row.partial.erase(0, complete);
        row.tokens.push_back(token); row.next = token;
        if (row.tokens.size() == size_t(r.config.i("output_tokens"))) row.finish = "output_limit";
    }
    require(row.finish.empty() || row.partial.empty(), "generation ended with incomplete UTF-8", -6);
}
void process_generation(VM &vm, Resource &r, int64_t work) {
    auto &q = *r.request;
    require(work > 0, "processing work budget must be positive");
    if (q.state == "cancelled" || q.state == "complete") return;
    require(q.state == "running", "request is not running", -7);
    auto &session = *vm.resources.at(r.parent);
    auto begin = Clock::now();
    try {
        auto ctx = session.context; auto vocab = llama_model_get_vocab(session.model->model);
        const int limit = int(std::min<int64_t>(work, llama_n_ubatch(ctx)));
        if (!q.batch) {
            q.batch = std::make_unique<GenerationBatch>(int(llama_n_ubatch(ctx)));
            if (auto memory = llama_get_memory(ctx)) llama_memory_clear(memory, true);
        }
        auto &batch = q.batch->value; batch.n_tokens = 0;
        bool prefill = q.prefill_row < q.rows.size();
        if (prefill) {
            while (q.prefill_row < q.rows.size() && batch.n_tokens < limit) {
                auto &tokens = q.rows[q.prefill_row].tokens;
                bool last = q.prefill_pos + 1 == tokens.size();
                q.batch->add(tokens[q.prefill_pos], int(q.prefill_pos), int(q.prefill_row), last);
                if (last) { ++q.prefill_row; q.prefill_pos = 0; } else ++q.prefill_pos;
            }
        } else {
            // Round-robin permits work budgets below the active row count while
            // normal budgets retain one batched decode for all live sequences.
            for (size_t visited = 0; visited < q.rows.size() && batch.n_tokens < limit; ++visited) {
                auto row = q.decode_row; q.decode_row = (q.decode_row + 1) % q.rows.size();
                auto &generated = q.generated[row];
                if (generated.finish.empty())
                    q.batch->add(generated.next, generated.position++, int(row), true);
            }
        }
        if (batch.n_tokens) {
            auto decode_begin = vm.probes_enabled ? Clock::now() : Clock::time_point{};
            int status = llama_decode(ctx, batch); llama_synchronize(ctx);
            auto decode_end = vm.probes_enabled ? Clock::now() : Clock::time_point{};
            ++q.decode_calls; q.last_work_tokens = batch.n_tokens;
            q.maximum_work_tokens = std::max(q.maximum_work_tokens, q.last_work_tokens);
            if (prefill) ++q.prefill_calls; else ++q.token_calls;
            require(status == 0, "generation inference failed", -6);
            for (int j = 0; j < batch.n_tokens; ++j) if (batch.logits[j]) {
                auto row = size_t(batch.seq_id[j][0]);
                auto next = generation_greedy(ctx, vocab, j);
                if (prefill) q.generated[row].next = next;
                else generation_sample(r, row, vocab, next);
            }
            if (prefill && q.prefill_row == q.rows.size())
                for (size_t row = 0; row < q.rows.size(); ++row) generation_sample(r, row, vocab, q.generated[row].next);
            if (vm.probes_enabled) {
                vm.probes.setup_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(decode_begin - begin).count();
                vm.probes.decode_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(decode_end - decode_begin).count();
                vm.probes.normalize_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - decode_end).count();
            }
        }
        if (q.prefill_row == q.rows.size() && std::all_of(q.generated.begin(), q.generated.end(),
                [](const GenerationRow &row) { return !row.finish.empty(); })) {
            q.state = "complete"; ++session.completed_requests;
            if (vm.probes_enabled) ++vm.probes.requests;
            release_active(vm, r);
        }
    } catch (const std::exception &e) {
        llama_synchronize(session.context); q.state = "failed"; q.error = e.what();
        for (auto &row : q.generated) if (row.finish.empty()) { row.finish = "error"; row.partial.clear(); }
        release_active(vm, r); throw;
    }
    q.process_ms += std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
}
void read_text(VM &vm, Resource &r, int64_t index, rxllama_result &out) {
    auto &q = *r.request;
    require(q.generation, "generation operation on an embedding request", -4);
    require(q.state != "building", "generation request has not been submitted", -7);
    require(index > 0 && size_t(index) <= q.generated.size(), "generation row index out of range");
    auto &row = q.generated[size_t(index - 1)];
    vm.text.swap(row.text); row.text.clear();
    out.text_length = vm.text.size(); out.integer = int64_t(row.tokens.size() - row.read_tokens);
    out.token_ids = row.tokens.empty() ? nullptr : row.tokens.data() + row.read_tokens;
    row.read_tokens = row.tokens.size(); out.finish = row.finish.c_str(); out.row = index;
}
