/* cREXX License (MIT). Shared manifest validation for loader and packager. */
#pragma once
#include <filesystem>
#include <atomic>
#include <fstream>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>
extern "C" {
#include "rxsha256.h"
}
namespace rxllama_package {
namespace fs = std::filesystem;
using json = nlohmann::json;
inline std::string hash(const fs::path &p, const std::atomic<bool> *cancel = nullptr) {
    std::ifstream file(p, std::ios::binary);
    if (!file) throw std::runtime_error("cannot open package/model file");
    rx_sha256_context c; rx_sha256_init(&c);
    char data[32768];
    while (file) { if (cancel && *cancel) throw std::runtime_error("file hashing cancelled"); file.read(data, sizeof(data)); if (!rx_sha256_update(&c, data, size_t(file.gcount())))
        throw std::runtime_error("file hash overflow"); }
    if (!file.eof()) throw std::runtime_error("file read failed");
    unsigned char digest[32]; rx_sha256_final(&c, digest);
    std::string result; static const char digits[] = "0123456789abcdef";
    for (auto b : digest) { result += digits[b >> 4]; result += digits[b & 15]; }
    return result;
}
inline fs::path relative(const fs::path &root, const std::string &name) {
    if (name.empty() || name.find_first_of("\\\r\n\t:") != std::string::npos)
        throw std::runtime_error("invalid package path");
    auto p = fs::u8path(name);
    if (p.is_absolute()) throw std::runtime_error("absolute package path");
    for (const auto &part : p) if (part == ".." || part == ".") throw std::runtime_error("package path traversal");
    auto base = fs::canonical(root); auto full = fs::canonical(root / p);
    auto rel = full.lexically_relative(base);
    if (rel.empty() || *rel.begin() == "..") throw std::runtime_error("package symlink escapes root");
    return full;
}
inline json read(const fs::path &path) {
    if (fs::file_size(path) > 1024 * 1024) throw std::runtime_error("oversized package manifest");
    std::ifstream file(path); auto j = json::parse(file);
    if (j.at("version") != 1 || j.at("platform") != CREXX_LLAMA_PLATFORM || j.at("arch") != CREXX_LLAMA_ARCH)
        throw std::runtime_error("unsupported package version/platform/architecture");
    return j;
}
inline fs::path verified(const fs::path &root, const json &entry) {
    auto file = relative(root, entry.at("path").get<std::string>());
    if (hash(file) != entry.at("sha256").get<std::string>()) throw std::runtime_error("package dependency hash mismatch");
    return file;
}
}
