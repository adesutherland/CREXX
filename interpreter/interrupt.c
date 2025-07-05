//
// Created by Adrian Sutherland on 05/04/2025.
//


/* TODO - We need to redefine CREXX Signals for external os events as follows:

    // --- User/System Initiated Interruption & Termination ---

    // CREXX_EVENT_INTERRUPT:
    //   Meaning: A polite request from the user (e.g., Ctrl+C) to stop current activity
    //            and/or initiate a graceful shutdown. The application should attempt
    //            to clean up and exit, but can choose to ignore or finish current tasks.
    //   OS Mapping: Windows: CTRL_C_EVENT
    //               POSIX:   SIGINT
    CREXX_EVENT_INTERRUPT,

    // CREXX_EVENT_BREAK:
    //   Meaning: A more insistent request from the user (e.g., Ctrl+Break) to halt
    //            execution and/or initiate an immediate graceful shutdown. This is
    //            often used if the application is unresponsive to an INTERRUPT.
    //            The application should prioritize stopping quickly over completing
    //            non-critical tasks.
    //   OS Mapping: Windows: CTRL_BREAK_EVENT
    //               POSIX:   SIGQUIT (Note: POSIX SIGQUIT often implies a core dump,
    //                        which this event does not require or imply on CREXX/Windows.)
    CREXX_EVENT_BREAK,

    // CREXX_EVENT_SHUTDOWN:
    //   Meaning: A critical request from the OS or a control system to perform a
    //            graceful shutdown. This is typically due to the user closing the console
    //            window, logging off, or the entire system shutting down/rebooting.
    //            The application MUST perform all necessary cleanup (saving data,
    //            flushing logs, releasing resources) and exit within a short grace period
    //            to avoid forceful termination by the OS.
    //   OS Mapping: Windows: CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
    //               POSIX:   SIGTERM (typically sent programmatically, not via console UI)
    CREXX_EVENT_SHUTDOWN,

    // --- Application Control & Management ---

    // CREXX_EVENT_RELOAD_CONFIG:
    //   Meaning: A request to reload the application's configuration settings from
    //            its source (e.g., a file) without restarting the application.
    //            The application should attempt to apply new settings safely and
    //            continue operation.
    //   OS Mapping: Windows: Custom IPC (e.g., RESTful API, Named Pipe, UDP listener)
    //               POSIX:   SIGHUP (common convention for daemons to reload config)
    CREXX_EVENT_RELOAD_CONFIG,
*/


#define _POSIX_C_SOURCE 200809L /* Request POSIX features */
#include <signal.h>
#include <stddef.h>  /* For NULL */
#include <string.h>  /* For memset */

/* Include VM signal definitions */
#include "rxvmintp.h" /* For RXSIGNAL_* codes, and set / clear interrupt functions */

#ifdef _WIN32
    #include <windows.h> /* For platform specifics if needed later */
    /* Windows doesn't have some POSIX signals */
    #define SIGHUP  -1 /* Not applicable */
    #define SIGPIPE -1 /* Not applicable */
    #define SIGUSR1 -1 /* Not applicable */
    #define SIGUSR2 -1 /* Not applicable */
    #define SIGCHLD -1 /* Not applicable */
    #define MAX_OS_SIGNALS 32 /* Max OS signal number we might store */
#else
    #include <unistd.h>  /* POSIX specifics */
    /* Define MAX_OS_SIGNALS for POSIX based on common values like NSIG */
    /* NSIG itself isn't guaranteed, use a reasonable upper bound */
    #define MAX_OS_SIGNALS 128
#endif

/* --- Global Variables --- */

/* Storage for original signal dispositions */
#ifdef _WIN32
    /* Type for the signal handler function */
    typedef void (*OsSignalHandlerFunc)(int);
    static OsSignalHandlerFunc g_original_os_handlers[MAX_OS_SIGNALS];
#else
    static struct sigaction g_original_os_actions[MAX_OS_SIGNALS];
#endif

/* Tracks which VM signals have our handler active (indexed by RXSIGNAL_* code) */
/* 0=inactive, 1=active handler, -1=set to ignore by us */
static volatile sig_atomic_t g_handler_active[RXSIGNAL_MAX];

/* --- Mapping between VM Signals and OS Signals --- */
typedef struct {
    int vm_signal;
    int os_signal; /* OS signal number, or -1 if not directly mappable */
} VmOsSignalMap;

/* Define the mapping - adjust as needed! */
static const VmOsSignalMap g_signal_map[] = {
    /* Instruction/Syntax Errors */
    { RXSIGNAL_ERROR,               SIGILL }, /* Treat syntax error as illegal operation */

    /* Termination / User Interaction */
    { RXSIGNAL_TERM,               SIGTERM },
#ifndef _WIN32
    /* POSIX-specific Signals Mappings */
    { RXSIGNAL_POSIX_INT,          SIGINT  },
    { RXSIGNAL_QUIT,               SIGQUIT },
    { RXSIGNAL_POSIX_HUP,          SIGHUP  },
    { RXSIGNAL_POSIX_USR1,         SIGUSR1 },
    { RXSIGNAL_POSIX_USR2,         SIGUSR2 },
    { RXSIGNAL_POSIX_CHLD,         SIGCHLD },
    { RXSIGNAL_NOTREADY,           SIGPIPE }, /* Often related to broken pipes */
#else
    /* Windows-specific Signal Mappings */
    { RXSIGNAL_TERM,          CTRL_CLOSE_EVENT  },
    { RXSIGNAL_QUIT,          CTRL_BREAK_EVENT },
{ RXSIGNAL_POSIX_INT,         CTRL_C_EVENT  },
#endif
    /* Note: RXSIGNAL_KILL (SIGKILL) cannot be caught */
    /* Note: RXSIGNAL_FAILURE, etc. do not map directly */
};
static const size_t g_signal_map_size = sizeof(g_signal_map) / sizeof(g_signal_map[0]);

/* Helper to get OS signal from VM signal */
static int get_os_signal(int vm_signal) {
    size_t i;
    for (i = 0; i < g_signal_map_size; ++i) {
        if (g_signal_map[i].vm_signal == vm_signal) {
            return g_signal_map[i].os_signal;
        }
    }
    return -1; /* Not found or not mapped */
}

/* Helper to get VM signal from OS signal */
static int get_vm_signal(int os_signal) {
    size_t i;
    /* This mapping back might be ambiguous - first match wins here */
    for (i = 0; i < g_signal_map_size; ++i) {
        if (g_signal_map[i].os_signal == os_signal) {
            return g_signal_map[i].vm_signal;
        }
    }
    return RXSIGNAL_MAX; /* Unmapped is an error - RXSIGNAL_MAX is ignored */
}

/* --- Master Signal Handler --- */

/*
 * This handler is called for any OS signal we intercept.
 * It must be simple and only use async-signal-safe functions.
 * It translates the OS signal to a VM signal and sets a bit in the global flag mask.
 */
#ifdef _WIN32
static BOOL WINAPI vm_master_signal_handler(DWORD signum) {
#else
static void vm_master_signal_handler(int signum /* OS Signal Number */) {
#endif
    int vm_signal;

    /* Translate OS signal back to VM signal */
    vm_signal = get_vm_signal((int)(signum));

    /* Check if the VM signal code is valid for the bitmask */
    if (vm_signal > RXSIGNAL_NONE && vm_signal < RXSIGNAL_MAX) {
        raise_signal(vm_signal); /* Set the interrupt signal */
#ifdef _WIN32
        return TRUE; /* Indicate we handled it */
#endif
    }

    /* Re-arm handler if needed? */
    /* With sigaction (POSIX) - Not needed if SA_RESETHAND isn't set (default) */
#ifdef _WIN32
    return FALSE; /* Indicate we did not handle it */
#endif
}

/* --- Public API Functions --- */

/**
 * @brief Enables handling for a specific VM interrupt code.
 * Translates the VM code to an OS signal and registers the master handler.
 * Does nothing if the VM code doesn't map to a catchable OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to enable.
 * @return 0 on success or if no action needed, -1 on failure to register handler.
 */
int enable_interrupt(int vm_signal) {
    int os_signal;
#ifndef _WIN32
    struct sigaction sa_new; /* POSIX */
#else
    int handler_installed; /* Windows */
#endif

    /* Validate VM signal code */
    if (vm_signal <= RXSIGNAL_NONE || vm_signal >= RXSIGNAL_MAX) {
        return 0; /* Invalid code, treat as no-op success */
    }

    /* Check if already active */
    if (g_handler_active[vm_signal] == 1) {
        return 0;
    }

    /* Get the corresponding OS signal */
    os_signal = get_os_signal(vm_signal);

    /* Check if mappable and valid OS signal */
    if (os_signal < 0 || os_signal >= MAX_OS_SIGNALS) {
        return 0; /* Not mappable, treat as no-op success */
    }

    /* Register the handler */
#ifndef _WIN32
    /* --- POSIX: Use sigaction --- */
    struct sigaction sa_old;

    if (g_handler_active[vm_signal] == -1) {
        /* Already set to ignore by us, so don't save original */

        /* Query and store the original action *before* setting the new one */
        if (sigaction(os_signal, NULL, &sa_old) == 0) {
            memcpy(&g_original_os_actions[os_signal], &sa_old, sizeof(struct sigaction));
        } else {
            /* perror("Warning: sigaction query failed"); */
            /* Initialize original to SIG_DFL equivalent */
            memset(&g_original_os_actions[os_signal], 0, sizeof(struct sigaction));
            g_original_os_actions[os_signal].sa_handler = SIG_DFL;
            sigemptyset(&g_original_os_actions[os_signal].sa_mask);
        }
    }

    /* Set up our new action */
    memset(&sa_new, 0, sizeof(sa_new));
    sa_new.sa_handler = vm_master_signal_handler;
    sigemptyset(&sa_new.sa_mask); /* Block no other signals during handler */
    sa_new.sa_flags = SA_RESTART;  /* Restart interrupted syscalls */

    /* Install our handler */
    if (sigaction(os_signal, &sa_new, NULL) == -1) {
        perror("Error: sigaction enable failed");
        return -1; /* Failure */
    }
#endif

    /* Mark as active */
    g_handler_active[vm_signal] = 1;
    return 0; /* Success */
}

/**
 * @brief Sets the specific VM interrupt code to be ignored, meaning the linked OS signal is ignored.
 * Does nothing if the VM code doesn't map to a catchable OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to ignore.
 * return 0 on success or if no action is needed, -1 on failure to register handler.
 */
int ignore_interrupt(int vm_signal) {
   int os_signal;
#ifndef _WIN32
    struct sigaction sa_new; /* POSIX */
#endif

    /* Validate VM signal code */
    if (vm_signal <= RXSIGNAL_NONE || vm_signal >= RXSIGNAL_MAX) {
        return 0; /* Invalid code, treat as no-op success */
    }

    /* Check if already ignored by us */
    if (g_handler_active[vm_signal] == -1) {
        return 0;
    }

    /* Get the corresponding OS signal */
    os_signal = get_os_signal(vm_signal);

    /* Check if mappable and valid OS signal */
    if (os_signal < 0 || os_signal >= MAX_OS_SIGNALS) {
        return 0; /* Not mappable, treat as no-op success */
    }

    /* Register the handler */
#ifndef _WIN32
    /* --- POSIX: Use sigaction --- */
    struct sigaction sa_old;

    if (g_handler_active[vm_signal] == 1) {
        /* Already set to active by us, so don't save original */

        /* Query and store the original action *before* setting the new one */
        if (sigaction(os_signal, NULL, &sa_old) == 0) {
            memcpy(&g_original_os_actions[os_signal], &sa_old, sizeof(struct sigaction));
        } else {
            /* perror("Warning: sigaction query failed"); */
            /* Initialize original to SIG_DFL equivalent */
            memset(&g_original_os_actions[os_signal], 0, sizeof(struct sigaction));
            g_original_os_actions[os_signal].sa_handler = SIG_DFL;
            sigemptyset(&g_original_os_actions[os_signal].sa_mask);
        }
    }

    /* Set up our new action to ignore */
    memset(&sa_new, 0, sizeof(sa_new));
    sa_new.sa_handler = SIG_IGN; /* Set to ignore */
    sigemptyset(&sa_new.sa_mask); /* Block no other signals during handler */
    sa_new.sa_flags = SA_RESTART;  /* Restart interrupted syscalls */

    if (sigaction(os_signal, &sa_new, NULL) == -1) {
        /* perror("Error: sigaction enable failed"); */
        return -1; /* Failure */
    }
#endif

    /* Mark as ignored by us */
    g_handler_active[vm_signal] = -1;
    return 0; /* Success */
}

/**
 * @brief Restores handling for a specific VM interrupt code.
 * Restores the original signal handler for the corresponding OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to disable.
 * @return 0 on success or if no action needed, -1 on failure to restore handler.
 */
int restore_interrupt(int vm_signal) {
    int os_signal;

    /* Validate VM signal code */
    if (vm_signal <= RXSIGNAL_NONE || vm_signal >= RXSIGNAL_MAX) {
        return 0;
    }

    /* Check if inactive */
    if (!g_handler_active[vm_signal]) {
        return 0;
    }

    /* Get the corresponding OS signal */
    os_signal = get_os_signal(vm_signal);

    /* Check if mappable and valid */
    if (os_signal < 0 || os_signal >= MAX_OS_SIGNALS) {
        /* This case should ideally not happen if it was active, but check anyway */
        g_handler_active[vm_signal] = 0; /* Mark inactive */
        return 0;
    }

    /* Restore original handler */
#ifndef _WIN32
    /* --- POSIX: Restore original action --- */
    if (sigaction(os_signal, &g_original_os_actions[os_signal], NULL) == -1) {
        /* perror("Error: sigaction disable failed"); */
        /* Let's mark inactive as we tried. */
        g_handler_active[vm_signal] = 0;
        return -1; /* Failure */
    }
#endif

    /* Mark as inactive */
    g_handler_active[vm_signal] = 0;
    return 0; /* Success */
}

/**
 * @brief Initializes the VM signal handling system.
 * Clears flags, initializes storage, and prepares for enabling interrupts.
 * Should be called once at VM startup.
 * @return 0 on success.
 */
int initialize_vm_signals(void) {
    int i;

    /* Initialize tracking and storage */
    for (i = 0; i < RXSIGNAL_MAX; ++i) {
        g_handler_active[i] = 0;
    }
    for (i = 0; i < MAX_OS_SIGNALS; ++i) {
#ifdef _WIN32
        g_original_os_handlers[i] = NULL;
#else
        memset(&g_original_os_actions[i], 0, sizeof(struct sigaction));
#endif
    }

#ifdef _WIN32
    /* Install our handler */
    if (!SetConsoleCtrlHandler(vm_master_signal_handler, TRUE))
    {
        fprintf(stderr, "ERROR: Could not set console control handler. GetLastError: %lu\n", GetLastError());
        return -1; // Indicate failure to set up handler
    }
#endif
    return 0;
}

/**
 * @brief Cleans up signal handlers, restoring originals for active ones.
 * Intended to be called via atexit or manually before VM shutdown.
 */
void cleanup_vm_signals(void) {
    int i;
#ifdef _WIN32
    /* Install our handler */
    if (!SetConsoleCtrlHandler(vm_master_signal_handler, FALSE))
    {
        fprintf(stderr, "ERROR: Could not unload console control handler. GetLastError: %lu\n", GetLastError());
    }
#endif
    for (i = 0; i < RXSIGNAL_MAX; ++i) {
        if (g_handler_active[i]) {
            restore_interrupt(i); /* Attempt to restore */
        }
    }
}
