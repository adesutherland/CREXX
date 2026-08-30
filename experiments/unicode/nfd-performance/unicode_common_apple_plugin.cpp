/*
 * Experimental CoreFoundation controls for the shared Unicode runtime panel.
 *
 * Each exported procedure fixes one operation before entering the timed Level B
 * loop.  The call pays the ordinary RXPA UTF-8 copy-out/return path, native
 * CoreFoundation transformation, UTF-8 materialization, and allocation costs.
 * RXPA strings are currently NUL terminated, so this remains a bounded control
 * for the retained Unicode corpora rather than a proposed general API.
 */

#include <CoreFoundation/CoreFoundation.h>

#include <cstdlib>
#include <cstring>

#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT

namespace {

enum class Operation {
    Nfd,
    Nfkd,
    Nfc,
    Nfkc,
    FullFold,
    TurkicFullFold
};

CFLocaleRef turkic_locale()
{
    static CFLocaleRef locale =
        CFLocaleCreate(kCFAllocatorDefault, CFSTR("tr_TR"));
    return locale;
}

void apply_operation(CFMutableStringRef value, Operation operation)
{
    switch (operation) {
        case Operation::Nfd:
            CFStringNormalize(value, kCFStringNormalizationFormD);
            break;
        case Operation::Nfkd:
            CFStringNormalize(value, kCFStringNormalizationFormKD);
            break;
        case Operation::Nfc:
            CFStringNormalize(value, kCFStringNormalizationFormC);
            break;
        case Operation::Nfkc:
            CFStringNormalize(value, kCFStringNormalizationFormKC);
            break;
        case Operation::FullFold:
            CFStringFold(value, kCFCompareCaseInsensitive, nullptr);
            break;
        case Operation::TurkicFullFold:
            CFStringFold(value, kCFCompareCaseInsensitive, turkic_locale());
            break;
    }
}

void transform_procedure(
    rxinteger _numargs,
    rxpa_attribute_value *_arg,
    rxpa_attribute_value _return,
    rxpa_attribute_value _signal,
    Operation operation)
{
    const char *input;
    CFStringRef source;
    CFMutableStringRef transformed;
    CFIndex utf16_units;
    CFIndex maximum_utf8_bytes;
    char *output;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "one UTF-8 string expected")

    input = GETSTRING(ARG(0));
    if (input == nullptr)
        RETURNSIGNAL(SIGNAL_FAILURE, "RXPA string copy-out failed")

    source = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8 *>(input),
        static_cast<CFIndex>(std::strlen(input)),
        kCFStringEncodingUTF8,
        false);
    if (source == nullptr)
        RETURNSIGNAL(SIGNAL_UNICODE_ERROR, "CoreFoundation rejected UTF-8 input")

    transformed = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, source);
    CFRelease(source);
    if (transformed == nullptr)
        RETURNSIGNAL(SIGNAL_FAILURE, "CoreFoundation mutable string allocation failed")

    apply_operation(transformed, operation);
    utf16_units = CFStringGetLength(transformed);
    maximum_utf8_bytes = CFStringGetMaximumSizeForEncoding(
        utf16_units, kCFStringEncodingUTF8);
    if (maximum_utf8_bytes < 0) {
        CFRelease(transformed);
        RETURNSIGNAL(SIGNAL_FAILURE, "CoreFoundation UTF-8 size calculation failed")
    }

    output = static_cast<char *>(
        std::malloc(static_cast<size_t>(maximum_utf8_bytes) + 1u));
    if (output == nullptr) {
        CFRelease(transformed);
        RETURNSIGNAL(SIGNAL_FAILURE, "UTF-8 output allocation failed")
    }

    if (!CFStringGetCString(
            transformed, output, maximum_utf8_bytes + 1,
            kCFStringEncodingUTF8)) {
        std::free(output);
        CFRelease(transformed);
        RETURNSIGNAL(SIGNAL_UNICODE_ERROR, "CoreFoundation UTF-8 output failed")
    }

    SETSTRING(RETURN, output);
    std::free(output);
    CFRelease(transformed);
    RESETSIGNAL
}

void predicate_procedure(
    rxinteger _numargs,
    rxpa_attribute_value *_arg,
    rxpa_attribute_value _return,
    rxpa_attribute_value _signal,
    Operation operation)
{
    const char *input;
    CFStringRef source;
    CFMutableStringRef transformed;
    Boolean normalized;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "one UTF-8 string expected")

    input = GETSTRING(ARG(0));
    if (input == nullptr)
        RETURNSIGNAL(SIGNAL_FAILURE, "RXPA string copy-out failed")

    source = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8 *>(input),
        static_cast<CFIndex>(std::strlen(input)),
        kCFStringEncodingUTF8,
        false);
    if (source == nullptr)
        RETURNSIGNAL(SIGNAL_UNICODE_ERROR, "CoreFoundation rejected UTF-8 input")

    transformed = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, source);
    if (transformed == nullptr) {
        CFRelease(source);
        RETURNSIGNAL(SIGNAL_FAILURE, "CoreFoundation mutable string allocation failed")
    }

    apply_operation(transformed, operation);
    normalized = CFEqual(source, transformed);
    CFRelease(transformed);
    CFRelease(source);
    SETINT(RETURN, normalized ? 1 : 0);
    RESETSIGNAL
}

}  // namespace

static char nfd_name[] = "unicode_common_apple.normalize_nfd";
static char nfkd_name[] = "unicode_common_apple.normalize_nfkd";
static char nfc_name[] = "unicode_common_apple.normalize_nfc";
static char nfkc_name[] = "unicode_common_apple.normalize_nfkc";
static char is_nfd_name[] = "unicode_common_apple.is_nfd";
static char is_nfkd_name[] = "unicode_common_apple.is_nfkd";
static char is_nfc_name[] = "unicode_common_apple.is_nfc";
static char is_nfkc_name[] = "unicode_common_apple.is_nfkc";
static char full_fold_name[] = "unicode_common_apple.fold_full";
static char turkic_full_fold_name[] = "unicode_common_apple.fold_turkic_full";
static char level_option[] = "b";
static char return_type[] = ".string";
static char predicate_return_type[] = ".int";
static char arguments[] = "source = .string";

PROCEDURE(normalize_nfd)
{
    transform_procedure(_numargs, _arg, _return, _signal, Operation::Nfd);
}

PROCEDURE(normalize_nfkd)
{
    transform_procedure(_numargs, _arg, _return, _signal, Operation::Nfkd);
}

PROCEDURE(normalize_nfc)
{
    transform_procedure(_numargs, _arg, _return, _signal, Operation::Nfc);
}

PROCEDURE(normalize_nfkc)
{
    transform_procedure(_numargs, _arg, _return, _signal, Operation::Nfkc);
}

PROCEDURE(is_nfd)
{
    predicate_procedure(_numargs, _arg, _return, _signal, Operation::Nfd);
}

PROCEDURE(is_nfkd)
{
    predicate_procedure(_numargs, _arg, _return, _signal, Operation::Nfkd);
}

PROCEDURE(is_nfc)
{
    predicate_procedure(_numargs, _arg, _return, _signal, Operation::Nfc);
}

PROCEDURE(is_nfkc)
{
    predicate_procedure(_numargs, _arg, _return, _signal, Operation::Nfkc);
}

PROCEDURE(fold_full)
{
    transform_procedure(_numargs, _arg, _return, _signal, Operation::FullFold);
}

PROCEDURE(fold_turkic_full)
{
    transform_procedure(
        _numargs, _arg, _return, _signal, Operation::TurkicFullFold);
}

LOADFUNCS
ADDPROC(normalize_nfd, nfd_name, level_option, return_type, arguments);
ADDPROC(normalize_nfkd, nfkd_name, level_option, return_type, arguments);
ADDPROC(normalize_nfc, nfc_name, level_option, return_type, arguments);
ADDPROC(normalize_nfkc, nfkc_name, level_option, return_type, arguments);
ADDPROC(is_nfd, is_nfd_name, level_option, predicate_return_type, arguments);
ADDPROC(is_nfkd, is_nfkd_name, level_option, predicate_return_type, arguments);
ADDPROC(is_nfc, is_nfc_name, level_option, predicate_return_type, arguments);
ADDPROC(is_nfkc, is_nfkc_name, level_option, predicate_return_type, arguments);
ADDPROC(fold_full, full_fold_name, level_option, return_type, arguments);
ADDPROC(
    fold_turkic_full, turkic_full_fold_name,
    level_option, return_type, arguments);
ENDLOADFUNCS
