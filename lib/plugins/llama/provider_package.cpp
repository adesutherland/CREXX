/* cREXX License (MIT). Declarative dependency copier for crexx -native. */
#include "package.hpp"
#include <iostream>
#include <map>
#include <set>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#include <memory>
#endif
int main(int argc, char **argv) {
    using namespace rxllama_package;
    try {
#ifdef _WIN32
        // Narrow CRT argv is encoded in the active code page, not necessarily
        // UTF-8. Preserve the native Unicode command line at the existing UTF-8
        // filesystem boundary used by this helper and its manifest validator.
        int wide_argc = 0;
        std::unique_ptr<wchar_t *, decltype(&LocalFree)> wide_argv(
            CommandLineToArgvW(GetCommandLineW(), &wide_argc), &LocalFree);
        if (!wide_argv) throw std::runtime_error("cannot read Unicode command line");
        std::vector<std::string> utf8_arguments;
        utf8_arguments.reserve(wide_argc);
        for (int i = 0; i < wide_argc; ++i)
            utf8_arguments.push_back(utf8_path(fs::path(wide_argv.get()[i])));
        std::vector<char *> argument_pointers;
        argument_pointers.reserve(wide_argc);
        for (auto &argument : utf8_arguments) argument_pointers.push_back(argument.data());
        argc = wide_argc;
        argv = argument_pointers.data();
#endif
        const bool fingerprint = argc == 3 && std::string(argv[1]) == "--fingerprint";
        if (!fingerprint && argc != 4) throw std::runtime_error("usage: crexx-provider-package MANIFEST OUTPUT_STEM GNU|MSVC, or --fingerprint MANIFEST");
        auto source = fs::absolute(from_utf8(argv[fingerprint ? 2 : 1])); auto root = source.parent_path();
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
        auto destination = fs::absolute(from_utf8(argv[2])).parent_path();
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
            if (from_utf8(name).filename() != from_utf8(name)) throw std::runtime_error("runtime dependency must be an adjacent file");
            auto dest = destination / from_utf8(name);
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
