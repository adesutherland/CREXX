/* Session-owned terminal mechanism. UI policy lives in Level G, not here. */
#include "rxconsole_core.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
static SRWLOCK owner_lock=SRWLOCK_INIT;
#define LOCK() AcquireSRWLockExclusive(&owner_lock)
#define UNLOCK() ReleaseSRWLockExclusive(&owner_lock)
#else
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
static pthread_mutex_t owner_lock=PTHREAD_MUTEX_INITIALIZER;
#define LOCK() pthread_mutex_lock(&owner_lock)
#define UNLOCK() pthread_mutex_unlock(&owner_lock)
#endif

struct rxcon_terminal {
    int modes, columns, rows, disconnected;
    uint64_t fragment_since;
    rxcon_decoder decoder;
#ifdef _WIN32
    HANDLE input, output;
    DWORD input_mode, output_mode;
    UINT output_codepage;
    int buttons, repeats;
    WCHAR high_surrogate;
    rxcon_event repeated;
#else
    int input, output;
    pid_t owner;
    struct termios saved;
#endif
};
/* A physical terminal is exclusive even when several VM sessions exist. */
static rxcon_terminal *active_terminal;

uint64_t rxcon_clock(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC,&now)!=0) return 0;
    return (uint64_t)now.tv_sec*1000u+(uint64_t)now.tv_nsec/1000000u;
#endif
}
int rxcon_size(rxcon_terminal *t,int *columns,int *rows) {
    if (!t || !columns || !rows) return RXCON_INVALID;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(t->output,&info)) return RXCON_IO;
    *columns=info.srWindow.Right-info.srWindow.Left+1;
    *rows=info.srWindow.Bottom-info.srWindow.Top+1;
#else
    struct winsize size;
    if (ioctl(t->output,TIOCGWINSZ,&size)!=0) return RXCON_IO;
    *columns=size.ws_col; *rows=size.ws_row;
#endif
    return 0;
}
int rxcon_write(rxcon_terminal *t,const char *bytes,size_t length) {
    if (!t || (!bytes && length)) return RXCON_INVALID;
    if (length>1048576) return RXCON_LIMIT;
#ifndef _WIN32
    if (getpid()!=t->owner || tcgetpgrp(t->input)!=getpgrp()) return RXCON_BUSY;
#endif
    while (length) {
#ifdef _WIN32
        DWORD written=0;
        if (!WriteFile(t->output,bytes,(DWORD)length,&written,NULL) || !written) return RXCON_IO;
#else
        ssize_t written=write(t->output,bytes,length);
        if (written<0 && errno==EINTR) continue;
        if (written<=0) return RXCON_IO;
#endif
        bytes+=written; length-=(size_t)written;
    }
    return 0;
}
static int sequence(rxcon_terminal *t,const char *text) {
    return rxcon_write(t,text,strlen(text));
}
int rxcon_modes(rxcon_terminal *t,int modes) {
    int rc;
    if (!t || modes<0 || (modes&~15)) return RXCON_INVALID;
    rc=sequence(t,"\033[?1002l\033[?1003l\033[?1006l\033[?2004l");
    if (rc<0) return rc;
    if ((modes&RXCON_ALT)!=(t->modes&RXCON_ALT)) {
        rc=sequence(t,(modes&RXCON_ALT)?"\033[?1049h":"\033[?1049l");
        if (rc<0) return rc;
    }
    /* Record the alternate-screen transition even if a later output fails. */
    t->modes=modes;
#ifndef _WIN32
    if (modes&(RXCON_BUTTON_MOUSE|RXCON_HOVER_MOUSE)) {
        rc=sequence(t,(modes&RXCON_HOVER_MOUSE)?"\033[?1006h\033[?1003h":"\033[?1006h\033[?1002h");
        if (rc<0) return rc;
    }
#endif
    if (modes&RXCON_BRACKET_PASTE) return sequence(t,"\033[?2004h");
    return 0;
}
int rxcon_profile_capabilities(void) {
    /* raw=1, ANSI=2, resize=4, mouse=8, bracketed paste=16, UTF-8=32.
     * Terminal protocol features describe the selected VT profile, not proof
     * that every intervening multiplexer forwards a requested event. */
#ifdef _WIN32
    return 1|2|4|8|32; /* Native console input does not preserve paste boundaries. */
#else
    return 1|2|4|8|16|32;
#endif
}
int rxcon_capabilities(rxcon_terminal *t) {
    return t ? rxcon_profile_capabilities() : RXCON_INVALID;
}
int rxcon_open(rxcon_terminal **result,int modes) {
    rxcon_terminal *t; int rc=RXCON_NOTTY;
    if (!result || modes<0 || (modes&~15)) return RXCON_INVALID;
    *result=NULL;
    LOCK();
    if (active_terminal) { UNLOCK();return RXCON_BUSY; }
    t=(rxcon_terminal *)calloc(1,sizeof(*t));
    if (!t) { UNLOCK();return RXCON_IO; }
#ifdef _WIN32
    {
        HANDLE process=GetCurrentProcess(); DWORD flags;
        t->input=INVALID_HANDLE_VALUE;t->output=INVALID_HANDLE_VALUE;
        if (!DuplicateHandle(process,GetStdHandle(STD_INPUT_HANDLE),process,&t->input,0,FALSE,DUPLICATE_SAME_ACCESS) ||
            !DuplicateHandle(process,GetStdHandle(STD_OUTPUT_HANDLE),process,&t->output,0,FALSE,DUPLICATE_SAME_ACCESS) ||
            !GetConsoleMode(t->input,&t->input_mode) || !GetConsoleMode(t->output,&t->output_mode)) goto failed;
        flags=ENABLE_EXTENDED_FLAGS|ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT;
        if (!SetConsoleMode(t->input,flags)) goto failed;
        if (!SetConsoleMode(t->output,t->output_mode|ENABLE_PROCESSED_OUTPUT|ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            (void)SetConsoleMode(t->input,t->input_mode);goto failed;
        }
        t->output_codepage=GetConsoleOutputCP();
        if (!SetConsoleOutputCP(CP_UTF8)) {
            (void)SetConsoleMode(t->input,t->input_mode);
            (void)SetConsoleMode(t->output,t->output_mode);goto failed;
        }
    }
#else
    {
        struct termios raw;struct stat input_stat,output_stat;const char *term=getenv("TERM");
        t->input=-1;t->output=-1;t->owner=getpid();
        if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ||
            (term && !strcmp(term,"dumb"))) goto failed;
        if (tcgetpgrp(STDIN_FILENO)!=getpgrp()) { rc=RXCON_BUSY;goto failed; }
        if (fstat(STDIN_FILENO,&input_stat)!=0 || fstat(STDOUT_FILENO,&output_stat)!=0 ||
            input_stat.st_rdev!=output_stat.st_rdev) goto failed;
        t->input=dup(STDIN_FILENO);t->output=dup(STDOUT_FILENO);
        if (t->input<0 || t->output<0) goto failed;
        if (fcntl(t->input,F_SETFD,FD_CLOEXEC)<0 || fcntl(t->output,F_SETFD,FD_CLOEXEC)<0 ||
            tcgetattr(t->input,&t->saved)!=0) goto failed;
        raw=t->saved;
        raw.c_iflag &= ~(BRKINT|ICRNL|INPCK|ISTRIP|IXON|INLCR|IGNCR);
        raw.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
        raw.c_cflag = (raw.c_cflag & ~(CSIZE|PARENB))|CS8;
        raw.c_cc[VMIN]=1; raw.c_cc[VTIME]=0;
        if (tcsetattr(t->input,TCSANOW,&raw)!=0) goto failed;
    }
#endif
    rxcon_decoder_init(&t->decoder);
    active_terminal=t;UNLOCK();
    rc=rxcon_modes(t,modes);
    if (rc<0) { rxcon_close(t);return rc; }
    (void)rxcon_size(t,&t->columns,&t->rows);
    *result=t;return 0;
failed:
#ifdef _WIN32
    if (t->input!=INVALID_HANDLE_VALUE) CloseHandle(t->input);
    if (t->output!=INVALID_HANDLE_VALUE) CloseHandle(t->output);
#else
    if (t->input>=0) close(t->input);
    if (t->output>=0) close(t->output);
#endif
    free(t);UNLOCK();return rc;
}
int rxcon_close(rxcon_terminal *t) {
    int rc=0;
    if (!t) return RXCON_INVALID;
    LOCK();
    if (t!=active_terminal) { UNLOCK();return RXCON_INVALID; }
#ifndef _WIN32
    if (getpid()==t->owner && tcgetpgrp(t->input)==getpgrp()) {
#endif
        rc=sequence(t,"\033[?1002l\033[?1003l\033[?1006l\033[?2004l\033[0m\033[?25h");
        if (t->modes&RXCON_ALT) { if (sequence(t,"\033[?1049l")<0) rc=RXCON_IO; }
#ifdef _WIN32
        if (!SetConsoleMode(t->input,t->input_mode)) rc=RXCON_IO;
        if (!SetConsoleMode(t->output,t->output_mode)) rc=RXCON_IO;
        if (!SetConsoleOutputCP(t->output_codepage)) rc=RXCON_IO;
    CloseHandle(t->input);CloseHandle(t->output);
#else
        while (tcsetattr(t->input,TCSANOW,&t->saved)!=0) {
            if (errno==EINTR) continue;
            rc=RXCON_IO;break;
        }
    }
    /* Never restore a replacement stdin or mutate a background terminal. */
    close(t->input);close(t->output);
#endif
    active_terminal=NULL;free(t);UNLOCK();return rc;
}
#ifdef _WIN32
static int windows_event(rxcon_terminal *t,rxcon_event *e) {
    INPUT_RECORD input;DWORD count;KEY_EVENT_RECORD *key;unsigned modifiers;const char *name=NULL;
    if (!ReadConsoleInputW(t->input,&input,1,&count) || !count) return RXCON_IO;
    if (input.EventType==WINDOW_BUFFER_SIZE_EVENT) return 0; /* Query viewport, not backing buffer. */
    if (input.EventType==MOUSE_EVENT) {
        MOUSE_EVENT_RECORD *mouse=&input.Event.MouseEvent;int buttons=0,changed,i;
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (!(t->modes&(RXCON_BUTTON_MOUSE|RXCON_HOVER_MOUSE))) return 0;
        if (mouse->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) buttons|=1;
        if (mouse->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED) buttons|=2;
        if (mouse->dwButtonState&RIGHTMOST_BUTTON_PRESSED) buttons|=4;
        e->fields[0]=mouse->dwMousePosition.X;e->fields[1]=mouse->dwMousePosition.Y;
        if (GetConsoleScreenBufferInfo(t->output,&info)) {
            e->fields[0]-=info.srWindow.Left;e->fields[1]-=info.srWindow.Top;
        }
        modifiers=mouse->dwControlKeyState;
        e->fields[4]=((modifiers&SHIFT_PRESSED)?1:0)|((modifiers&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))?2:0)|
                     ((modifiers&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))?4:0);
        if (mouse->dwEventFlags&(MOUSE_WHEELED|MOUSE_HWHEELED)) {
            e->kind=RXCON_WHEEL;
            e->fields[(mouse->dwEventFlags&MOUSE_HWHEELED)?8:9]=
                ((mouse->dwEventFlags&MOUSE_HWHEELED)?1:-1)*(short)HIWORD(mouse->dwButtonState)/WHEEL_DELTA;
        } else {
            if ((mouse->dwEventFlags&MOUSE_MOVED) && !buttons && !(t->modes&RXCON_HOVER_MOUSE)) return 0;
            changed=buttons^t->buttons;e->kind=RXCON_MOUSE;
            e->fields[5]=buttons;e->fields[7]=3;
            for (i=0;i<3;++i) if (changed&(1<<i)) {
                e->fields[6]=i+1;e->fields[7]=(buttons&(1<<i))?1:2;break;
            }
            t->buttons=buttons;
        }
        return e->kind;
    }
    if (input.EventType!=KEY_EVENT) return 0;
    key=&input.Event.KeyEvent;if (!key->bKeyDown) return 0;
    modifiers=key->dwControlKeyState;
    e->fields[4]=((modifiers&SHIFT_PRESSED)?1:0)|((modifiers&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))?2:0)|
                 ((modifiers&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))?4:0);
    e->fields[7]=1;
    switch(key->wVirtualKeyCode) {
    case VK_ESCAPE:name="Escape";break;case VK_RETURN:name="Enter";break;
    case VK_TAB:name="Tab";break;case VK_BACK:name="Backspace";break;
    case VK_UP:name="Up";break;case VK_DOWN:name="Down";break;
    case VK_LEFT:name="Left";break;case VK_RIGHT:name="Right";break;
    case VK_HOME:name="Home";break;case VK_END:name="End";break;
    case VK_PRIOR:name="PageUp";break;case VK_NEXT:name="PageDown";break;
    case VK_DELETE:name="Delete";break;case VK_INSERT:name="Insert";break;
    }
    if (name) { e->kind=RXCON_KEY;strcpy(e->text,name); }
    else if (key->wVirtualKeyCode>=VK_F1 && key->wVirtualKeyCode<=VK_F12) {
        e->kind=RXCON_KEY;wsprintfA(e->text,"F%d",key->wVirtualKeyCode-VK_F1+1);
    } else {
        WCHAR chars[2];int n=1;WCHAR ch=key->uChar.UnicodeChar;
        if (!ch) return 0;
        if (ch==3) {e->kind=RXCON_INTERRUPT;return e->kind;}
        if (ch<32) {e->kind=RXCON_KEY;e->text[0]=(char)('a'+ch-1);e->fields[4]|=RXCON_CTRL;}
        else {
            if (ch>=0xd800 && ch<=0xdbff) {t->high_surrogate=ch;return 0;}
            chars[0]=ch;
            if (ch>=0xdc00 && ch<=0xdfff && t->high_surrogate) {
                chars[0]=t->high_surrogate;chars[1]=ch;n=2;
            }
            t->high_surrogate=0;
            if (!WideCharToMultiByte(CP_UTF8,0,chars,n,e->text,sizeof(e->text)-1,NULL,NULL)) return RXCON_IO;
            e->kind=(e->fields[4]&RXCON_ALTKEY)?RXCON_KEY:RXCON_TEXT;
        }
    }
    if (key->wRepeatCount>1) {t->repeated=*e;t->repeats=key->wRepeatCount-1;}
    return e->kind;
}
#endif
int rxcon_poll(rxcon_terminal *t,int timeout,rxcon_event *e) {
    uint64_t start,now;int rc,columns,rows,wait_ms;
    if (!t || !e || timeout < -1) return RXCON_INVALID;
    memset(e,0,sizeof(*e));start=rxcon_clock();
    if (t->disconnected) {e->kind=RXCON_DISCONNECT;return e->kind;}
    for (;;) {
        now=rxcon_clock();
        if (rxcon_size(t,&columns,&rows)==0 && (columns!=t->columns || rows!=t->rows)) {
            t->columns=columns;t->rows=rows;e->fields[2]=columns;e->fields[3]=rows;
            e->kind=RXCON_RESIZE;return e->kind;
        }
#ifdef _WIN32
        if (t->repeats) {*e=t->repeated;--t->repeats;if(e->kind==RXCON_KEY)e->fields[7]=2;return e->kind;}
#else
        {
            pid_t foreground;
            if(getpid()!=t->owner)return RXCON_BUSY;
            errno=0;foreground=tcgetpgrp(t->input);
            if(foreground<0 && (errno==EIO || errno==ENOTTY || errno==ENXIO)) {
                t->disconnected=1;e->kind=RXCON_DISCONNECT;return e->kind;
            }
            if(foreground!=getpgrp())return RXCON_BUSY;
        }
        rc=rxcon_decode(&t->decoder,e,t->fragment_since && now-t->fragment_since>=40);
        if (rc) {t->fragment_since=0;return rc;}
        if (t->decoder.used && !t->decoder.pasting && !t->fragment_since) t->fragment_since=now;
#endif
        wait_ms=25; /* Size polling avoids replacing process-wide SIGWINCH handlers. */
        if (timeout>=0) {
            uint64_t elapsed=now-start;
            if (elapsed>=(uint64_t)timeout) wait_ms=0;
            else if ((uint64_t)wait_ms>(uint64_t)timeout-elapsed) wait_ms=(int)((uint64_t)timeout-elapsed);
        }
#ifdef _WIN32
        {
            DWORD wait=WaitForSingleObject(t->input,(DWORD)wait_ms);
            if (wait==WAIT_FAILED) return RXCON_IO;
            if (wait==WAIT_OBJECT_0) {rc=windows_event(t,e);if(rc)return rc;memset(e,0,sizeof(*e));}
        }
#else
        {
            struct pollfd fd;unsigned char bytes[4096];ssize_t length;
            fd.fd=t->input;fd.events=POLLIN;fd.revents=0;
            rc=poll(&fd,1,wait_ms);
            /* A handled SIGWINCH or unrelated host signal is not Ctrl-C. */
            if (rc<0) {if(errno==EINTR)continue;return RXCON_IO;}
            if (rc>0) {
                if (fd.revents&POLLIN) {
                    length=read(t->input,bytes,sizeof(bytes));
                    if (length<0 && errno==EINTR) continue;
                    if (length<=0) {t->disconnected=1;e->kind=RXCON_DISCONNECT;return e->kind;}
                    rc=rxcon_feed(&t->decoder,bytes,(size_t)length);
                    if (rc<0) return rc;
                    /* Decode before respecting timeout=0 after a successful read. */
                    rc=rxcon_decode(&t->decoder,e,0);if(rc){t->fragment_since=0;return rc;}
                    if(t->decoder.used && !t->decoder.pasting && !t->fragment_since)t->fragment_since=rxcon_clock();
                } else if (fd.revents&(POLLHUP|POLLERR|POLLNVAL)) {
                    t->disconnected=1;e->kind=RXCON_DISCONNECT;return e->kind;
                }
            }
        }
#endif
        if (timeout>=0 && rxcon_clock()-start>=(uint64_t)timeout) return 0;
    }
}
