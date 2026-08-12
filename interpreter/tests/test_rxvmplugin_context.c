/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmintp.h"
#include "rxvmplugin_framework.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct plugin_shared {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE condition;
#else
    pthread_mutex_t mutex;
    pthread_cond_t condition;
#endif
    int ready;
    int release_workers;
    int second_destroyed;
    int failures;
    decplugin *plugins[2];
    void *private_contexts[2];
} plugin_shared;

typedef struct plugin_thread {
    plugin_shared *shared;
    int index;
} plugin_thread;

static void plugin_lock(plugin_shared *shared) {
#ifdef _WIN32
    EnterCriticalSection(&shared->mutex);
#else
    (void)pthread_mutex_lock(&shared->mutex);
#endif
}

static void plugin_unlock(plugin_shared *shared) {
#ifdef _WIN32
    LeaveCriticalSection(&shared->mutex);
#else
    (void)pthread_mutex_unlock(&shared->mutex);
#endif
}

static void plugin_wait(plugin_shared *shared) {
#ifdef _WIN32
    SleepConditionVariableCS(&shared->condition, &shared->mutex, INFINITE);
#else
    (void)pthread_cond_wait(&shared->condition, &shared->mutex);
#endif
}

static void plugin_broadcast(plugin_shared *shared) {
#ifdef _WIN32
    WakeAllConditionVariable(&shared->condition);
#else
    (void)pthread_cond_broadcast(&shared->condition);
#endif
}

static void plugin_thread_run(plugin_thread *thread) {
    plugin_shared *shared = thread->shared;
    rxvm_context context;
    numeric_context numeric;
    decplugin *plugin;
    size_t expected_digits = thread->index ? 18u : 9u;
    int expected_signal = thread->index ? RXSIGNAL_CONVERSION_ERROR : RXSIGNAL_DIVISION_BY_ZERO;
    rxvm_worker_transition_result begin_result;
    rxvm_worker_transition_result end_result = RXVM_WORKER_TRANSITION_INVALID_STATE;
    int prepare_result = -1;
    int i;

    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    begin_result = rxvm_worker_begin_execution(&context.worker);
    if (begin_result == RXVM_WORKER_TRANSITION_OK) {
        prepare_result = rxvmplugin_instance_set_prepare(
                &context.plugin_instances, RXVM_PLUGIN_DECIMAL);
        end_result = rxvm_worker_end_execution(&context.worker);
    }
    if (begin_result != RXVM_WORKER_TRANSITION_OK ||
        prepare_result != 0 || end_result != RXVM_WORKER_TRANSITION_OK) {
        plugin_lock(shared);
        shared->failures++;
        shared->ready++;
        plugin_broadcast(shared);
        plugin_unlock(shared);
        rxfremod(&context);
        return;
    }

    plugin = (decplugin *)rxvmplugin_instance_set_get(
            &context.plugin_instances, RXVM_PLUGIN_DECIMAL);
    memset(&numeric, 0, sizeof(numeric));
    numeric.digits = (int)expected_digits;
    numeric.fuzz = thread->index;
    numeric.form = thread->index
            ? NUMERIC_FORM_ENGINEERING : NUMERIC_FORM_SCIENTIFIC;
    numeric.casetype = thread->index ? CASE_UPPER : CASE_LOWER;
    numeric.standard = thread->index
            ? NUMERIC_STANDARD_CLASSIC : NUMERIC_STANDARD_COMMON;
    plugin->num_context = &numeric;
    plugin->base.signal_number = expected_signal;
    plugin->syncNumericContext(plugin);

    plugin_lock(shared);
    shared->plugins[thread->index] = plugin;
    shared->private_contexts[thread->index] = plugin->base.private_context;
    shared->ready++;
    plugin_broadcast(shared);
    while (!shared->release_workers) plugin_wait(shared);
    plugin_unlock(shared);

    for (i = 0; i < 1000; i++) {
        plugin->syncNumericContext(plugin);
        if (plugin->num_context != &numeric ||
            plugin->getDigits(plugin) != expected_digits ||
            plugin->base.signal_number != expected_signal) {
            plugin_lock(shared);
            shared->failures++;
            plugin_unlock(shared);
            break;
        }
    }

    if (thread->index == 1) {
        rxfremod(&context);
        plugin_lock(shared);
        shared->second_destroyed = 1;
        plugin_broadcast(shared);
        plugin_unlock(shared);
    } else {
        plugin_lock(shared);
        while (!shared->second_destroyed) plugin_wait(shared);
        plugin_unlock(shared);
        if (plugin->num_context != &numeric ||
            plugin->getDigits(plugin) != expected_digits ||
            plugin->base.signal_number != expected_signal) {
            plugin_lock(shared);
            shared->failures++;
            plugin_unlock(shared);
        }
        rxfremod(&context);
    }
}

#ifdef _WIN32
static DWORD WINAPI plugin_thread_entry(LPVOID opaque) {
    plugin_thread_run((plugin_thread *)opaque);
    return 0;
}
#else
static void *plugin_thread_entry(void *opaque) {
    plugin_thread_run((plugin_thread *)opaque);
    return NULL;
}
#endif

static rxvm_plugin *failing_factory(void) {
    return NULL;
}

int main(int argc, char **argv) {
    plugin_shared shared;
    plugin_thread threads[2];
    size_t factory_count;
    size_t first_dynamic_count;
    size_t second_dynamic_count;
    int first_dynamic_rc;
    int second_dynamic_rc;
    rxvm_plugin *manual_compatibility_instance;
    int i;
#ifdef _WIN32
    HANDLE handles[2];
#else
    pthread_t handles[2];
#endif

    memset(&shared, 0, sizeof(shared));
#ifdef _WIN32
    InitializeCriticalSection(&shared.mutex);
    InitializeConditionVariable(&shared.condition);
#else
    (void)pthread_mutex_init(&shared.mutex, NULL);
    (void)pthread_cond_init(&shared.condition, NULL);
#endif

    CALL_PLUGIN_INITIALIZER(decnumber);
    factory_count = rxvmplugin_catalogue_count();
    CALL_PLUGIN_INITIALIZER(decnumber);
    if (factory_count != 1u || rxvmplugin_catalogue_count() != factory_count) {
        fprintf(stderr, "Duplicate manual registration changed the catalogue\n");
        return 1;
    }
    manual_compatibility_instance = get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    register_rxvmplugin("failing", failing_factory);
    if (rxvmplugin_catalogue_count() != factory_count) {
        fprintf(stderr, "Failing factory published a partial descriptor\n");
        return 1;
    }
    if (load_rxvmplugin(NULL, "rxvm_plugin_that_does_not_exist") != -1 ||
        rxvmplugin_catalogue_count() != factory_count) {
        fprintf(stderr, "Failed load changed the catalogue\n");
        return 1;
    }
    if (argc != 4) {
        fprintf(stderr, "Expected plugin directory, file name and registration name\n");
        return 1;
    }
    first_dynamic_rc = load_rxvmplugin(argv[1], argv[2]);
    first_dynamic_count = rxvmplugin_catalogue_count();
    second_dynamic_rc = load_rxvmplugin(argv[1], argv[2]);
    second_dynamic_count = rxvmplugin_catalogue_count();
    if (first_dynamic_rc != 0 ||
        first_dynamic_count != factory_count + 1u ||
        second_dynamic_rc != 0 ||
        second_dynamic_count != factory_count + 1u ||
        !find_rxvmplugin(argv[3], RXVM_PLUGIN_DECIMAL) ||
        get_rxvmplugin(RXVM_PLUGIN_DECIMAL) == manual_compatibility_instance) {
        fprintf(stderr,
                "Dynamic registration transaction failed: rc=%d/%d count=%zu/%zu expected=%zu\n",
                first_dynamic_rc, second_dynamic_rc,
                first_dynamic_count, second_dynamic_count,
                factory_count + 1u);
        return 1;
    }

    for (i = 0; i < 2; i++) {
        threads[i].shared = &shared;
        threads[i].index = i;
#ifdef _WIN32
        handles[i] = CreateThread(NULL, 0, plugin_thread_entry, &threads[i], 0, NULL);
        if (!handles[i]) return 1;
#else
        if (pthread_create(&handles[i], NULL, plugin_thread_entry, &threads[i]) != 0) return 1;
#endif
    }

    plugin_lock(&shared);
    while (shared.ready < 2) plugin_wait(&shared);
    if (!shared.plugins[0] || !shared.plugins[1] ||
        shared.plugins[0] == shared.plugins[1] ||
        shared.private_contexts[0] == shared.private_contexts[1] ||
        rxvmplugin_live_instance_count() != 2u) {
        shared.failures++;
    }
    clear_rxvmplugin_factories();
    if (rxvmplugin_catalogue_count() != 0u ||
        rxvmplugin_live_instance_count() != 2u) {
        shared.failures++;
    }
    shared.release_workers = 1;
    plugin_broadcast(&shared);
    plugin_unlock(&shared);

    for (i = 0; i < 2; i++) {
#ifdef _WIN32
        WaitForSingleObject(handles[i], INFINITE);
        CloseHandle(handles[i]);
#else
        (void)pthread_join(handles[i], NULL);
#endif
    }

    if (rxvmplugin_live_instance_count() != 0u) shared.failures++;

#ifdef _WIN32
    DeleteCriticalSection(&shared.mutex);
#else
    (void)pthread_cond_destroy(&shared.condition);
    (void)pthread_mutex_destroy(&shared.mutex);
#endif

    if (shared.failures) {
        fprintf(stderr, "RXVM provider context ownership test failed: %d\n",
                shared.failures);
        return 1;
    }
    printf("RXVM provider instances are isolated across two live contexts\n");
    return 0;
}
