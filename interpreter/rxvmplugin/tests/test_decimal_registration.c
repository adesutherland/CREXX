/* Independent ABI gate, value-only synchronization and transaction controls. */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvmplugin_framework.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL decimal ABI line %d: %s\n", __LINE__, #condition); ++failures; \
} } while (0)

static void destroy_old(rxvm_plugin *plugin) { free(plugin); }
static rxvm_plugin *old_decimal(void) {
    rxvm_plugin *plugin = (rxvm_plugin *)calloc(1u, sizeof(*plugin));
    if (plugin) { plugin->type = RXVM_PLUGIN_DECIMAL; plugin->free = destroy_old; }
    return plugin;
}
static rxvm_plugin *missing_sync_decimal(void) {
    decplugin *plugin = (decplugin *)calloc(1u, sizeof(*plugin));
    if (plugin) { plugin->base.type = RXVM_PLUGIN_DECIMAL; plugin->base.free = destroy_old; }
    return (rxvm_plugin *)plugin;
}

static int same_context(const numeric_context *left, const numeric_context *right) {
    return left->digits == right->digits && left->fuzz == right->fuzz &&
        left->form == right->form && left->casetype == right->casetype &&
        left->standard == right->standard;
}

static void check_sync(decplugin *plugin, int limited_precision) {
    numeric_context first = {9, 0, NUMERIC_FORM_SCIENTIFIC, CASE_LOWER, NUMERIC_STANDARD_COMMON};
    numeric_context second = first;
    numeric_context original = first;
    numeric_context defaults = {DEFAULT_NUMERIC_DIGITS, DEFAULT_NUMERIC_FUZZ,
        DEFAULT_NUMERIC_FORM, DEFAULT_NUMERIC_CASE, NUMERIC_STANDARD_COMMON};
    char diagnostic[] = "preserve sync diagnostic";
    void *private_context = plugin->base.private_context;
    value number = {0};
    size_t expected_default = (size_t)DEFAULT_NUMERIC_DIGITS;
    if (limited_precision && expected_default > (size_t)LDBL_DIG)
        expected_default = (size_t)LDBL_DIG;

    plugin->base.signal_number = RXSIGNAL_OTHER;
    plugin->base.signal_string = diagnostic;
    plugin->num_context = &first;
    plugin->syncNumericContext(plugin);
    CHECK(plugin->getDigits(plugin) == 9u);
    CHECK(same_context(&first, &original));
    CHECK(plugin->num_context == &first && plugin->base.private_context == private_context);
    CHECK(plugin->base.signal_number == RXSIGNAL_OTHER && plugin->base.signal_string == diagnostic);

    /* Equal values at a different address, and repeated sync, are equivalent. */
    plugin->num_context = &second;
    plugin->syncNumericContext(plugin);
    plugin->syncNumericContext(plugin);
    CHECK(plugin->getDigits(plugin) == 9u && same_context(&second, &original));
    second.digits = 11;
    second.standard = NUMERIC_STANDARD_CLASSIC;
    plugin->syncNumericContext(plugin);
    CHECK(plugin->getDigits(plugin) == 11u && second.digits == 11 &&
          second.standard == NUMERIC_STANDARD_CLASSIC);

    plugin->num_context = NULL;
    plugin->syncNumericContext(plugin);
    CHECK(plugin->getDigits(plugin) == expected_default);
    plugin->num_context = &defaults;
    plugin->syncNumericContext(plugin);
    CHECK(plugin->getDigits(plugin) == expected_default);
    CHECK(plugin->base.signal_number == RXSIGNAL_OTHER && plugin->base.signal_string == diagnostic);
    CHECK(plugin->base.private_context == private_context);

    /* FUZZ comparison temporarily narrows mc_decimal's private precision.
     * Both successful and failed operations must preserve the synchronized
     * arithmetic configuration without an entry/return resynchronization. */
    second.fuzz = 1;
    plugin->num_context = &second;
    plugin->syncNumericContext(plugin);
    plugin->decimalFromInt(plugin, &number, 123);
    CHECK(plugin->base.signal_number == RXSIGNAL_NONE && plugin->getDigits(plugin) == 11u);
    CHECK(plugin->decimalCompareString(plugin, &number, "123") == 0);
    CHECK(plugin->base.signal_number == RXSIGNAL_NONE && plugin->getDigits(plugin) == 11u);
    (void)plugin->decimalCompareString(plugin, &number, "not-a-number");
    CHECK(plugin->base.signal_number == RXSIGNAL_CONVERSION_ERROR && plugin->getDigits(plugin) == 11u);
    CHECK(second.digits == 11 && second.fuzz == 1 && second.standard == NUMERIC_STANDARD_CLASSIC);
    plugin->release_value_storage(&number);
    plugin->num_context = NULL;
    plugin->base.signal_number = RXSIGNAL_NONE;
    plugin->base.signal_string = NULL;
}

int main(int argc, char **argv) {
    size_t count;
    rxvm_plugin *unicode;
    rxvm_decimal_registration_v3 malformed = {
        sizeof(rxvm_decimal_registration_v3), RXVM_DECIMAL_PLUGIN_ABI_V3,
        RXVM_PLUGIN_DECIMAL, RXVM_DECIMAL_SYNC_CONTRACT_V1, "malformed", old_decimal
    };
    rxvmplugin_instance_set instances;
    decplugin *decimal;
    if (argc != 8) return 2;
    clear_rxvmplugin_factories();
    CHECK(rxvmplugin_catalogue_count() == 0u);
    register_rxvmplugin("old-static", old_decimal);
    CHECK(rxvmplugin_catalogue_count() == 0u);
    CHECK(load_rxvmplugin(argv[1], argv[4]) == RXVMPLUGIN_LOAD_INCOMPATIBLE);
    CHECK(rxvmplugin_catalogue_count() == 0u);
    CHECK(load_rxvmplugin(argv[1], argv[5]) == 0);
    unicode = find_rxvmplugin("old-unicode", RXVM_PLUGIN_UNICODE);
    CHECK(unicode != NULL && get_rxvmplugin(RXVM_PLUGIN_DECIMAL) == NULL);
    count = rxvmplugin_catalogue_count();
    CHECK(count == 1u);
    CHECK(load_rxvmplugin(argv[1], argv[6]) == RXVMPLUGIN_LOAD_INCOMPATIBLE);
    CHECK(rxvmplugin_catalogue_count() == count &&
          find_rxvmplugin("old-unicode", RXVM_PLUGIN_UNICODE) == unicode);

    malformed.struct_size--;
    CHECK(register_rxvm_decimal_plugin_v3(&malformed) == RXVMPLUGIN_LOAD_INCOMPATIBLE);
    malformed.struct_size = sizeof(malformed);
    malformed.sync_contract = 0u;
    CHECK(register_rxvm_decimal_plugin_v3(&malformed) == RXVMPLUGIN_LOAD_INCOMPATIBLE);
    malformed.sync_contract = RXVM_DECIMAL_SYNC_CONTRACT_V1;
    malformed.type = RXVM_PLUGIN_UNICODE;
    CHECK(register_rxvm_decimal_plugin_v3(&malformed) == RXVMPLUGIN_LOAD_INCOMPATIBLE);
    malformed.type = RXVM_PLUGIN_DECIMAL;
    /* A claimed new ABI still needs the required decimal entry point. */
    malformed.factory = missing_sync_decimal;
    CHECK(register_rxvm_decimal_plugin_v3(&malformed) == RXVMPLUGIN_LOAD_INCOMPATIBLE);
    CHECK(rxvmplugin_catalogue_count() == count);

    CHECK(load_rxvmplugin(argv[1], argv[2]) == 0);
    decimal = (decplugin *)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    CHECK(decimal != NULL);
    if (decimal) check_sync(decimal, 0);
    rxvmplugin_instance_set_init(&instances);
    CHECK(rxvmplugin_instance_set_prepare(&instances, RXVM_PLUGIN_DECIMAL) == 0);
    CHECK(rxvmplugin_live_instance_count() == 1u);
    CHECK(load_rxvmplugin(argv[1], argv[4]) == RXVMPLUGIN_LOAD_INCOMPATIBLE);
    CHECK(get_rxvmplugin(RXVM_PLUGIN_DECIMAL) == (rxvm_plugin *)decimal);
    CHECK(rxvmplugin_instance_set_get(&instances, RXVM_PLUGIN_DECIMAL) != NULL);

    CHECK(load_rxvmplugin(argv[1], argv[3]) == 0);
    decimal = (decplugin *)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    CHECK(decimal != NULL);
    if (decimal) check_sync(decimal, 1);
    count = rxvmplugin_catalogue_count();
    CHECK(load_rxvmplugin(argv[1], argv[7]) == 0);
    CHECK(rxvmplugin_catalogue_count() == count + 2u &&
          find_rxvmplugin("dualdecimal", RXVM_PLUGIN_DECIMAL) != NULL &&
          find_rxvmplugin("dual-unicode", RXVM_PLUGIN_UNICODE) != NULL);
    clear_rxvmplugin_factories();
    CHECK(rxvmplugin_catalogue_count() == 0u && rxvmplugin_live_instance_count() == 1u);
    decimal = (decplugin *)rxvmplugin_instance_set_get(&instances, RXVM_PLUGIN_DECIMAL);
    if (decimal) check_sync(decimal, 0);
    rxvmplugin_instance_set_destroy(&instances);
    CHECK(rxvmplugin_live_instance_count() == 0u);
    if (!failures) puts("PASS: decimal ABI 3 registration and value-only synchronization");
    return failures ? 1 : 0;
}
