/* MIT. The same package path must reach C APIs intact in C++17 and C++20. */
#include "package.hpp"
#include <iostream>

int main() {
    const std::string paths[] = {
        "models/plain.gguf",
        "models/Gr\xc3\xbc\xc3\x9f" "e-\xf0\x9f\x98\x80.gguf"
    };
    for (const auto &expected : paths) {
        const auto path = std::filesystem::u8path(expected);
        if (rxllama_package::utf8_path(path) != expected) {
            std::cerr << "FAIL: UTF-8 package path bytes changed\n";
            return 1;
        }
    }
    std::cout << "PASS: UTF-8 package path bytes\n";
    return 0;
}
