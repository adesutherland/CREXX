/* Product-owned target selection. Historical lab defines remain build aliases;
 * an ELF toolchain's inherited Linux macros never select native OS services. */
#ifndef CREXX_PLATFORM_CONFIG_H
#define CREXX_PLATFORM_CONFIG_H
#if defined(CREXX_CMS_ELF) || defined(__MAINFRAME_LAB_VM370_4381__) || defined(__MAINFRAME_LAB_CMS20_ESA31__)
#define CREXX_PLATFORM_CMS 1
#endif
#if defined(CREXX_TSO_ELF) || defined(__MAINFRAME_LAB_TSO31__) || defined(__MAINFRAME_LAB_TSO64__)
#define CREXX_PLATFORM_TSO 1
#endif
#if defined(CREXX_PLATFORM_CMS) && defined(CREXX_PLATFORM_TSO)
#error Select exactly one cREXX native platform
#endif
#if defined(CREXX_PLATFORM_CMS)
#define CREXX_CMS_ELF 1
#endif
#if defined(CREXX_PLATFORM_TSO)
#define CREXX_TSO_ELF 1
#endif
#if defined(CREXX_PLATFORM_CMS) || defined(CREXX_PLATFORM_TSO)
#define CREXX_MAINFRAME_ELF 1
#endif
#endif
