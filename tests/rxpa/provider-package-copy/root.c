extern int package_dependency(void);
#ifdef PACKAGE_TRANSITIVE_ROOT
extern int package_branch(void);
#endif
#ifdef _WIN32
__declspec(dllexport)
#endif
int package_root(void) {
#ifdef PACKAGE_TRANSITIVE_ROOT
    return package_dependency() + package_branch();
#else
    return package_dependency();
#endif
}
