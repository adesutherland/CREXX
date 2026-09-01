#include <stdio.h>

#include <rxbin.h>
#include <rxgraph.h>

int main(void) {
    printf("rxbin=%s graph_serial=%u\n", BIN_VERSION, RX_GRAPH_SERIAL_VERSION);
    return 0;
}
