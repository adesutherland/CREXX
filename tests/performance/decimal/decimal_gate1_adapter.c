/*
 * DECIMAL-01 Gate 1 decplugin adapter payload.
 *
 * Sampling and statistics remain owned by the Level B matrix runner. This
 * executable performs fixed work, validates a deterministic checksum and does
 * no internal timing.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxvmplugin_framework.h"
#include "platform.h"
#include "rxbin.h"
#include "rxvmvars.h"

static int parse_positive_count(const char *text, size_t *result) {
    char *end = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed == 0) return 1;
    *result = (size_t)parsed;
    return (unsigned long)*result != parsed;
}

static void init_value(value *target) {
    value_init(target);
}

static void clear_decimal_value(value *target) {
    clear_value(target);
}

static int copy_decimal_value(value *target, const value *source) {
    size_t length = rxvm_value_decimal_length(source);
    void *storage;

    if (source->decimal_value == NULL || length == 0) {
        rxvm_value_set_decimal_length(target, 0u);
        return 0;
    }
    storage = rxvm_value_reserve_decimal(target, length);
    if (storage == NULL) return 1;
    memcpy(storage, source->decimal_value, length);
    return 0;
}

static int plugin_failed(const decplugin *plugin) {
    return plugin->base.signal_number != 0;
}

static int run_arithmetic(decplugin *plugin, size_t iterations, char *checksum) {
    value left;
    value right;
    value result;
    size_t i;

    init_value(&left);
    init_value(&right);
    init_value(&result);
    plugin->decimalFromString(plugin, &left, "12345.6789");
    plugin->decimalFromString(plugin, &right, "3.125");
    if (plugin_failed(plugin)) goto fail;

    for (i = 0; i < iterations; ++i) {
        plugin->decimalAdd(plugin, &result, &left, &right);
        plugin->decimalSub(plugin, &result, &left, &right);
        plugin->decimalMul(plugin, &result, &left, &right);
        plugin->decimalDiv(plugin, &result, &left, &right);
        if (plugin_failed(plugin)) goto fail;
    }
    plugin->decimalToString(plugin, &result, checksum);
    if (plugin_failed(plugin)) goto fail;
    clear_decimal_value(&result);
    clear_decimal_value(&right);
    clear_decimal_value(&left);
    return 0;

fail:
    clear_decimal_value(&result);
    clear_decimal_value(&right);
    clear_decimal_value(&left);
    return 1;
}

static int run_conversion(decplugin *plugin, size_t iterations, char *checksum) {
    static const char *const inputs[] = {
        "12345.6789", "0.0000123456789", "9.87654321e+12", "-314159.265"
    };
    value number;
    size_t i;
    size_t total = 0;

    init_value(&number);
    for (i = 0; i < iterations; ++i) {
        plugin->decimalFromString(plugin, &number, inputs[i % 4]);
        if (plugin_failed(plugin)) goto fail;
        plugin->decimalToString(plugin, &number, checksum);
        if (plugin_failed(plugin)) goto fail;
        total += strlen(checksum);
    }
    (void)snprintf(checksum, plugin->getRequiredStringSize(plugin, NULL), "%zu", total);
    clear_decimal_value(&number);
    return 0;

fail:
    clear_decimal_value(&number);
    return 1;
}

static int run_compare(decplugin *plugin, size_t iterations, char *checksum) {
    value equal_left;
    value equal_right;
    value early_low;
    value early_high;
    value late_low;
    value late_high;
    size_t i;
    size_t hits = 0;

    init_value(&equal_left);
    init_value(&equal_right);
    init_value(&early_low);
    init_value(&early_high);
    init_value(&late_low);
    init_value(&late_high);
    plugin->decimalFromString(plugin, &equal_left, "12345.6789");
    plugin->decimalFromString(plugin, &equal_right, "12345.6789");
    plugin->decimalFromString(plugin, &early_low, "12345.6789");
    plugin->decimalFromString(plugin, &early_high, "22345.6789");
    plugin->decimalFromString(plugin, &late_low, "12345.6781");
    plugin->decimalFromString(plugin, &late_high, "12345.6782");
    if (plugin_failed(plugin)) goto fail;

    for (i = 0; i < iterations; ++i) {
        if (plugin->decimalCompare(plugin, &equal_left, &equal_right) == 0) hits += 1;
        if (plugin->decimalCompare(plugin, &early_low, &early_high) < 0) hits += 2;
        if (plugin->decimalCompare(plugin, &late_low, &late_high) < 0) hits += 4;
        if (plugin_failed(plugin)) goto fail;
    }
    (void)snprintf(checksum, plugin->getRequiredStringSize(plugin, NULL), "%zu", hits);
    clear_decimal_value(&late_high);
    clear_decimal_value(&late_low);
    clear_decimal_value(&early_high);
    clear_decimal_value(&early_low);
    clear_decimal_value(&equal_right);
    clear_decimal_value(&equal_left);
    return 0;

fail:
    clear_decimal_value(&late_high);
    clear_decimal_value(&late_low);
    clear_decimal_value(&early_high);
    clear_decimal_value(&early_low);
    clear_decimal_value(&equal_right);
    clear_decimal_value(&equal_left);
    return 1;
}

static int run_copy_clear(decplugin *plugin, size_t iterations, char *checksum) {
    value source;
    value copy;
    size_t i;
    size_t bytes = 0;

    init_value(&source);
    init_value(&copy);
    plugin->decimalFromString(plugin, &source, "12345.6789");
    if (plugin_failed(plugin)) goto fail;
    for (i = 0; i < iterations; ++i) {
        if (copy_decimal_value(&copy, &source) != 0) goto fail;
        bytes += rxvm_value_decimal_length(&copy);
        clear_decimal_value(&copy);
    }
    (void)snprintf(checksum, plugin->getRequiredStringSize(plugin, NULL), "%zu", bytes);
    clear_decimal_value(&source);
    return 0;

fail:
    clear_decimal_value(&copy);
    clear_decimal_value(&source);
    return 1;
}

static int run_sync(decplugin *plugin, size_t iterations, char *checksum) {
    size_t i;
    size_t total = 0;

    for (i = 0; i < iterations; ++i) {
        plugin->syncNumericContext(plugin);
        if (plugin_failed(plugin)) return 1;
        total += plugin->getDigits(plugin);
    }
    (void)snprintf(checksum, plugin->getRequiredStringSize(plugin, NULL), "%zu", total);
    return 0;
}

int main(int argc, char **argv) {
    char *provider_name;
    char *standard_name;
    char *mode;
    size_t digits;
    size_t iterations;
    numeric_context context;
    decplugin *plugin;
    char *checksum;
    int result;

    if (argc != 6) {
        fprintf(stderr, "FAIL: use PROVIDER common|classic DIGITS MODE ITERATIONS\n");
        return 2;
    }
    provider_name = argv[1];
    standard_name = argv[2];
    mode = argv[4];
    if (parse_positive_count(argv[3], &digits) != 0 ||
        parse_positive_count(argv[5], &iterations) != 0) {
        fprintf(stderr, "FAIL: DIGITS and ITERATIONS must be positive integers\n");
        return 2;
    }
    if (strcmp(standard_name, "common") != 0 &&
        strcmp(standard_name, "classic") != 0) {
        fprintf(stderr, "FAIL: standard must be common or classic\n");
        return 2;
    }
    if (load_rxvmplugin(".", provider_name) != 0) {
        fprintf(stderr, "FAIL: could not load provider %s\n", provider_name);
        return 1;
    }
    plugin = (decplugin *)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (plugin == NULL) {
        fprintf(stderr, "FAIL: provider did not register a decimal plugin\n");
        return 1;
    }

    context.digits = digits;
    context.fuzz = 0;
    context.form = NUMERIC_FORM_SCIENTIFIC;
    context.casetype = CASE_LOWER;
    context.standard = strcmp(standard_name, "classic") == 0
        ? NUMERIC_STANDARD_CLASSIC : NUMERIC_STANDARD_COMMON;
    plugin->num_context = &context;
    plugin->syncNumericContext(plugin);
    if (plugin_failed(plugin)) {
        fprintf(stderr, "FAIL: provider rejected numeric context\n");
        plugin->base.free((rxvm_plugin *)plugin);
        return 1;
    }

    checksum = (char *)malloc(plugin->getRequiredStringSize(plugin, NULL));
    if (checksum == NULL) {
        fprintf(stderr, "FAIL: checksum allocation\n");
        plugin->base.free((rxvm_plugin *)plugin);
        return 1;
    }
    checksum[0] = '\0';

    if (strcmp(mode, "arithmetic") == 0)
        result = run_arithmetic(plugin, iterations, checksum);
    else if (strcmp(mode, "conversion") == 0)
        result = run_conversion(plugin, iterations, checksum);
    else if (strcmp(mode, "compare") == 0)
        result = run_compare(plugin, iterations, checksum);
    else if (strcmp(mode, "copy-clear") == 0)
        result = run_copy_clear(plugin, iterations, checksum);
    else if (strcmp(mode, "sync") == 0)
        result = run_sync(plugin, iterations, checksum);
    else {
        fprintf(stderr, "FAIL: unknown MODE\n");
        free(checksum);
        plugin->base.free((rxvm_plugin *)plugin);
        return 2;
    }

    if (result != 0) {
        fprintf(stderr, "FAIL: provider operation signalled: %d %s\n",
                plugin->base.signal_number,
                plugin->base.signal_string == NULL ? "" : plugin->base.signal_string);
        free(checksum);
        plugin->base.free((rxvm_plugin *)plugin);
        return 1;
    }
    printf("DECIMAL-ADAPTER provider=%s standard=%s digits=%zu mode=%s "
           "iterations=%zu checksum=%s\n",
           provider_name, standard_name, digits, mode, iterations, checksum);
    printf("PASS: DECIMAL-01 Gate 1 adapter payload\n");
    free(checksum);
    plugin->base.free((rxvm_plugin *)plugin);
    return 0;
}
