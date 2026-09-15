/* MIT. Tiny, fixed-workload smoke against the same packaged engine as RXPA. */
#include "bridge.h"
#include "llama.h"
#include "ggml-backend.h"
#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

static void require(bool ok, const char *why) {
    if (!ok) throw std::runtime_error(why);
}
struct Host {
    void *vm = rxllama_vm_create();
    Host() {
        require(vm != nullptr, "create bridge host");
        std::array<rxllama_argument, 6> args{};
        rxllama_result result{};
        require(rxllama_call(vm, RXLLAMA_CONFIG_CREATE, args.data(), &result) == 0, "create configuration");
        args[0].handle = result.handle;
        if (rxllama_call(vm, RXLLAMA_RUNTIME_OPEN, args.data(), &result) != 0) {
            rxllama_call(vm, RXLLAMA_DIAGNOSTIC, args.data(), &result);
            std::string message = result.message;
            rxllama_vm_destroy(vm); vm = nullptr;
            throw std::runtime_error(message);
        }
    }
    ~Host() { rxllama_vm_destroy(vm); }
};
struct Batch {
    llama_batch batch = llama_batch_init(8, 0, 1);
    ~Batch() { llama_batch_free(batch); }
    void add(llama_token token, llama_pos pos, llama_seq_id row) {
        const int i = batch.n_tokens++;
        batch.token[i] = token; batch.pos[i] = pos;
        batch.n_seq_id[i] = 1; batch.seq_id[i][0] = row; batch.logits[i] = 1;
    }
};
using Model = std::unique_ptr<llama_model, decltype(&llama_model_free)>;
using Context = std::unique_ptr<llama_context, decltype(&llama_free)>;

static void exercise(const char *path, ggml_backend_dev_t gpu, const std::string &name) {
    std::cerr << "SMOKE_STAGE: " << name << " model load\n";
    auto mp = llama_model_default_params();
    ggml_backend_dev_t placement[] = {gpu, nullptr};
    mp.devices = placement; mp.n_gpu_layers = gpu ? 999 : 0;
    Model model(llama_model_load_from_file(path, mp), llama_model_free);
    require(model != nullptr, "load scratch fixture");
    require(llama_model_n_embd_out(model.get()) == 256, "fixture dimension");
    const int vocab = llama_vocab_n_tokens(llama_model_get_vocab(model.get()));
    require(vocab == 128, "fixture vocabulary");
    auto cp = llama_context_default_params();
    cp.n_ctx = 256; cp.n_batch = 8; cp.n_ubatch = 8; cp.n_seq_max = 2;
    cp.n_threads = 2; cp.n_threads_batch = 2;
    cp.pooling_type = LLAMA_POOLING_TYPE_NONE;
    std::cerr << "SMOKE_STAGE: " << name << " private contexts\n";
    Context generation(llama_init_from_model(model.get(), cp), llama_free);
    cp.embeddings = true;
    Context embedding(llama_init_from_model(model.get(), cp), llama_free);
    require(generation && embedding, "private contexts sharing one model");
    std::vector<llama_token> first;
    for (int repeat = 0; repeat < 2; ++repeat) {
        std::cerr << "SMOKE_STAGE: " << name << " generation repeat " << repeat << '\n';
        llama_memory_clear(llama_get_memory(generation.get()), true);
        Batch prefill;
        prefill.add(3, 0, 0); prefill.add(4, 1, 0);
        prefill.add(5, 0, 1); prefill.add(6, 1, 1);
        require(llama_decode(generation.get(), prefill.batch) == 0, "two-row generation prefill");
        std::vector<llama_token> sampled;
        for (int step = 0; step < 4; ++step) {
            Batch next;
            for (int row = 0; row < 2; ++row) {
                const float *logits = llama_get_logits_ith(generation.get(), step ? row : row * 2 + 1);
                require(logits != nullptr, "generation logits");
                llama_token best = 0;
                for (int token = 0; token < vocab; ++token) {
                    require(std::isfinite(logits[token]), "finite logits");
                    if (logits[token] > logits[best]) best = token;
                }
                sampled.push_back(best); next.add(best, step + 2, row);
            }
            require(llama_decode(generation.get(), next.batch) == 0, "bounded two-row generation");
        }
        if (!repeat) first = sampled;
        else require(sampled == first, "repeat request isolation");
    }
    Batch tokens;
    std::cerr << "SMOKE_STAGE: " << name << " embeddings\n";
    tokens.add(3, 0, 0); tokens.add(4, 1, 0);
    tokens.add(5, 0, 1); tokens.add(6, 1, 1);
    require(llama_decode(embedding.get(), tokens.batch) == 0, "embedding extraction");
    for (int row = 0; row < 2; ++row) {
        const float *values = llama_get_embeddings_ith(embedding.get(), row * 2 + 1);
        require(values != nullptr, "embedding output");
        double squared = 0;
        for (int i = 0; i < 256; ++i) {
            require(std::isfinite(values[i]), "finite embeddings");
            squared += double(values[i]) * values[i];
        }
        require(squared > 0, "nonzero embeddings");
    }
    std::cout << "PASS: packaged engine " << name
              << " repeated two-row generation and finite embeddings\n";
}

int main(int argc, char **argv) {
    try {
        require(argc == 2, "usage: release_engine_smoke FIXTURE");
        std::cerr << "SMOKE_STAGE: runtime discovery\n";
        Host host; // Use production discovery and verified package paths.
        std::cerr << "SMOKE_STAGE: runtime ready\n";
        exercise(argv[1], nullptr, "CPU");
        int gpus = 0;
        for (size_t i = 0; i < ggml_backend_dev_count(); ++i) {
            auto dev = ggml_backend_dev_get(i);
            const auto type = ggml_backend_dev_type(dev);
            if (type != GGML_BACKEND_DEVICE_TYPE_GPU && type != GGML_BACKEND_DEVICE_TYPE_IGPU) continue;
            exercise(argv[1], dev, ggml_backend_dev_name(dev)); ++gpus;
        }
        std::cout << "PASS: release engine smoke; actual_gpu_devices=" << gpus << '\n';
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n'; return 1;
    }
}
