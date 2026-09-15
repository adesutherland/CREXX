/* cREXX License (MIT). Declarative dependency copier for crexx -native. */
#include "package.hpp"
#include <iostream>
#include <map>
#include <set>
int main(int argc, char **argv) {
    using namespace rxllama_package;
    try {
        const bool fingerprint = argc == 3 && std::string(argv[1]) == "--fingerprint";
        if (!fingerprint && argc != 4) throw std::runtime_error("usage: crexx-provider-package MANIFEST OUTPUT_STEM GNU|MSVC, or --fingerprint MANIFEST");
        auto source = fs::absolute(fs::u8path(argv[fingerprint ? 2 : 1])); auto root = source.parent_path();
        auto manifest = read(source);
        const auto provider = manifest.at("provider").get<std::string>();
        if (provider.empty() || provider.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos || source.filename() != provider + ".native.json")
            throw std::runtime_error("invalid provider identity");
        const std::set<std::string> keys = {"version","provider","platform","arch","engine","link_libraries","runtime_files"};
        for (auto it = manifest.begin(); it != manifest.end(); ++it) if (!keys.count(it.key())) throw std::runtime_error("unknown package field");
        // Revalidate actual bytes before a project-cache hit, without copying.
        for (const auto &entry : manifest.at("runtime_files")) (void)verified(root, entry);
        std::vector<std::string> links;
        for (const auto &entry : manifest.at("link_libraries")) links.push_back(utf8_path(verified(root, entry)));
        if (fingerprint) { std::cout << hash(source) << '\n'; return 0; }
        if (std::string(argv[3]) != "GNU" && std::string(argv[3]) != "MSVC") throw std::runtime_error("unsupported compiler style");
        auto destination = fs::absolute(fs::u8path(argv[2])).parent_path();
        std::map<std::string, std::string> owned;
        const auto installed_manifest = destination / source.filename();
        if (fs::exists(installed_manifest)) {
            auto previous = read(installed_manifest);
            if (previous.at("provider") != provider) throw std::runtime_error("destination provider conflict");
            for (auto &entry : previous.at("runtime_files")) owned[entry.at("path")] = entry.at("sha256");
        }
        std::vector<std::pair<fs::path, fs::path>> copies;
        for (const auto &entry : manifest.at("runtime_files")) {
            auto path = verified(root, entry); auto name = entry.at("path").get<std::string>();
            // Runtime files are adjacent to the executable on all OSes. This
            // also satisfies Windows' initial dependent-DLL resolution.
            if (fs::u8path(name).filename() != fs::u8path(name)) throw std::runtime_error("runtime dependency must be an adjacent file");
            auto dest = destination / fs::u8path(name);
            if (fs::exists(dest)) {
                auto actual = hash(dest);
                if (actual == entry.at("sha256").get<std::string>()) continue;
                if (!owned.count(name) || owned.at(name) != actual) throw std::runtime_error("unowned runtime dependency collision");
            }
            copies.emplace_back(path, dest);
        }
        // Validate the complete manifest before copying or emitting linker args.
        fs::create_directories(destination);
        for (const auto &copy : copies) fs::copy_file(copy.first, copy.second, fs::copy_options::overwrite_existing);
        if (source != installed_manifest) fs::copy_file(source, installed_manifest, fs::copy_options::overwrite_existing);
        for (auto &path : links) std::cout << path << '\n';
#ifndef _WIN32
# ifdef __APPLE__
        std::cout << "-Wl,-rpath,@loader_path\n";
# else
        std::cout << "-Wl,-rpath,$ORIGIN\n";
# endif
#endif
        return 0;
    } catch (const std::exception &e) { std::cerr << "native provider package: " << e.what() << '\n'; return 2; }
}
