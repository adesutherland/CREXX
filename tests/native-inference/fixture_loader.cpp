// QA launcher for the unchanged upstream fixture generator's save path.
// That path does not discover dynamically built backends before creating a model.
#include "ggml-backend.h"
#include <iostream>
#include <vector>

int llama_arch_fixture_main(int argc, char ** argv);

int main(int argc, char ** argv) {
    if (argc < 3 || ggml_backend_load(argv[1]) == nullptr) {
        std::cerr << "Usage: fixture_generator CPU_BACKEND_FILE upstream-generator-options...\n";
        return 1;
    }
    std::vector<char *> forwarded{argv[0]};
    for (int i = 2; i < argc; ++i) forwarded.push_back(argv[i]);
    forwarded.push_back(nullptr);
    return llama_arch_fixture_main(argc-1,forwarded.data());
}
