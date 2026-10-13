extern int package_dependency(void);
#ifdef _WIN32
__declspec(dllexport)
#endif
int package_root(void) { return package_dependency(); }
