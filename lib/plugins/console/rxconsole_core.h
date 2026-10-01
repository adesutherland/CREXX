/* Low-level console mechanism. No VM, UI, widget or application dependencies. */
#ifndef CREXX_RXCONSOLE_CORE_H
#define CREXX_RXCONSOLE_CORE_H
#include <stddef.h>
#include <stdint.h>

enum { RXCON_KEY=1, RXCON_TEXT, RXCON_PASTE, RXCON_RESIZE, RXCON_MOUSE,
       RXCON_WHEEL, RXCON_INTERRUPT, RXCON_DISCONNECT };
enum { RXCON_INVALID=-1, RXCON_NOTTY=-2, RXCON_BUSY=-3, RXCON_IO=-4,
       RXCON_LIMIT=-5, RXCON_UNSUPPORTED=-6 };
enum { RXCON_ALT=1, RXCON_BUTTON_MOUSE=2, RXCON_HOVER_MOUSE=4,
       RXCON_BRACKET_PASTE=8 };
enum { RXCON_SHIFT=1, RXCON_ALTKEY=2, RXCON_CTRL=4 };
#define RXCON_TEXT_MAX 16384
/* fields: x,y,columns,rows,modifiers,buttons,button,phase,scroll_x,scroll_y.
 * C indexes are zero based; the public Level B array is one based. */
typedef struct rxcon_event {
    int kind;
    int fields[10];
    char text[RXCON_TEXT_MAX+1];
} rxcon_event;
typedef struct rxcon_decoder {
    unsigned char input[RXCON_TEXT_MAX+64];
    size_t used, paste_used;
    char paste[RXCON_TEXT_MAX+1];
    int pasting, overflow, buttons;
} rxcon_decoder;
void rxcon_decoder_init(rxcon_decoder *decoder);
int rxcon_feed(rxcon_decoder *decoder, const void *bytes, size_t length);
/* expire_escape resolves a lone Escape or incomplete sequence after a deadline. */
int rxcon_decode(rxcon_decoder *decoder, rxcon_event *event, int expire_escape);
typedef struct rxcon_terminal rxcon_terminal;
int rxcon_open(rxcon_terminal **terminal, int modes);
int rxcon_close(rxcon_terminal *terminal);
int rxcon_modes(rxcon_terminal *terminal, int modes);
int rxcon_size(rxcon_terminal *terminal, int *columns, int *rows);
int rxcon_poll(rxcon_terminal *terminal, int timeout_ms, rxcon_event *event);
int rxcon_write(rxcon_terminal *terminal, const char *bytes, size_t length);
int rxcon_capabilities(rxcon_terminal *terminal);
int rxcon_profile_capabilities(void);
uint64_t rxcon_clock(void);
#endif
