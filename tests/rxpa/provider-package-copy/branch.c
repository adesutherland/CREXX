extern int package_dependency(void);
int package_branch(void) { return package_dependency() + 1; }
