/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include "platform.h"
#include "rxcp_frontend.h"
#if defined(CREXX_MAINFRAME_ELF)
#include <mainframe_text.h>
#endif
int main(int argc, char **argv) {
#if defined(CREXX_MAINFRAME_ELF)
    mainframe_set_text_conversion(0);
#endif
    platform_install_signal_handlers();
    return rxcp_driver_main(argc, argv, &rxcp_classic_frontend);
}
