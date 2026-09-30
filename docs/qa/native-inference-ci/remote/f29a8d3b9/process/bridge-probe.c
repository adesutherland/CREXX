#include <stdio.h>
extern int rxllama_test_probe_cycle(void);
int main(void) {
    puts("BRIDGE_PROBE_BEGIN"); fflush(stdout);
    int result = rxllama_test_probe_cycle();
    printf("BRIDGE_PROBE_END: %d\n", result);
    return result;
}
