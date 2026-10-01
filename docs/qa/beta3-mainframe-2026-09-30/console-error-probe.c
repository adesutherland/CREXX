#include <stdio.h>
#include <errno.h>
#include "platform.h"
typedef struct { char *string_value; size_t string_length; long int_value; } value;
static value target, text;
static value *op1R = &target, *op2R = &text;
#define VM_ADVANCE(n)
#define DEBUG(...)
#define DISPATCH return
static int observed_signal;
#define RXSIGNAL_UNICODE_ERROR 9
#define RXSIGNAL_NOTREADY 15
#define SET_SIGNAL_MSG(signal, message) do { observed_signal = signal; } while (0)
#define RXVM_UTF8_ONLY(...) __VA_ARGS__
#define RXVM_BYTE_ONLY(...)
#include "mainframe_handler_selectors.h"
#undef FIXTURE_FWRITE_REG_REG
#define FIXTURE_FWRITE_REG_REG(...) static void run_fwrite(void) { __VA_ARGS__ }
#define RXVM_HANDLER(name, ...) FIXTURE_##name(__VA_ARGS__)
#define RXVM_PRIVATE_HANDLER(...)
#include "rxvmhandlers_system.inc"
int main(void) {
  int error, stream_error;
  long size;
  if (!freopen("/tmp/crexx-mainframe-integration-20260930/console-error-output", "wb", stdout)) return 2;
  target.int_value = (long)stdout;
  text.string_value = "\xe2\x82\xac"; text.string_length = 3;
  errno = 0; run_fwrite(); error = errno; stream_error = ferror(stdout);
  fflush(stdout); size = ftell(stdout);
  fprintf(stderr, "FWRITE euro: errno=%d EILSEQ=%d ferror=%d bytes=%ld signal=%d; handler dispatched\n", error, EILSEQ, stream_error, size, observed_signal);
  return !(error == EILSEQ && !stream_error && size == 0 && observed_signal == 9);
}
