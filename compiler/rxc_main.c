// This is the rxc main stub that calls the main function of the compiler.
// It exists to allow all the compiler code to be compiled into a library

// The main function of the compiler
int rxcmain(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    return rxcmain(argc, argv);
}