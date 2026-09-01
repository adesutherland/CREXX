#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "crexxpa.h"
#include "rxvmintp.h"
#include "rxvmvars.h"

static void set_disable_env(const char* value) {
#ifdef _WIN32
    _putenv_s("CREXX_RXPA_DISABLE_UTF8_CHECKS", value ? value : "");
#else
    if (value) {
        setenv("CREXX_RXPA_DISABLE_UTF8_CHECKS", value, 1);
    } else {
        unsetenv("CREXX_RXPA_DISABLE_UTF8_CHECKS");
    }
#endif
}

static int expect_signal(const char* label, value* signal, rxinteger expected) {
    if (signal->int_value == expected) return 0;
    fprintf(stderr,
            "%s: expected signal %jd, got %jd\n",
            label,
            (intmax_t)expected,
            (intmax_t)signal->int_value);
    return 1;
}

static void invalid_return_function(
    rxinteger args,
    rxpa_attribute_value* argv,
    rxpa_attribute_value ret,
    rxpa_attribute_value signal) {

    static char invalid_utf8[] = { (char)0xff, '\0' };
    (void)args;
    (void)argv;
    (void)signal;
    set_string((value*)ret, invalid_utf8, 1);
}

static void invalid_nested_argument_function(
    rxinteger args,
    rxpa_attribute_value* argv,
    rxpa_attribute_value ret,
    rxpa_attribute_value signal) {

    static char invalid_utf8[] = { (char)0xff, '\0' };
    value* root;
    (void)ret;
    (void)signal;
    if (args < 1 || !argv || !argv[0]) return;
    root = (value*)argv[0];
    set_num_attributes(root, 1);
    set_string(root->attributes[0], invalid_utf8, 1);
}

static void valid_nested_argument_function(
    rxinteger args,
    rxpa_attribute_value* argv,
    rxpa_attribute_value ret,
    rxpa_attribute_value signal) {

    value* root;
    (void)ret;
    (void)signal;
    if (args < 1 || !argv || !argv[0]) return;
    root = (value*)argv[0];
    set_num_attributes(root, 1);
    set_null_string(root->attributes[0], "valid text");
}

static void mutate_valid_non_ascii_argument_function(
    rxinteger args,
    rxpa_attribute_value* argv,
    rxpa_attribute_value ret,
    rxpa_attribute_value signal) {

    value* text;
    (void)ret;
    (void)signal;
    if (args < 1 || !argv || !argv[0]) return;
    text = (value*)argv[0];
    if (text->string_length > 0u) text->string_value[0] = 'E';
}

int main(void) {
    value ret;
    value signal;
    value arg;
    value* argv[1];
    int failures = 0;

    value_init(&ret);
    value_init(&signal);
    value_init(&arg);
    argv[0] = &arg;

    set_disable_env(NULL);

    rxvm_callfunc((void*)invalid_return_function, 0, NULL, &ret, &signal);
    failures += expect_signal("invalid return", &signal, SIGNAL_UNICODE_ERROR);

    value_zero(&ret);
    value_zero(&signal);
    value_zero(&arg);

    rxvm_callfunc((void*)invalid_nested_argument_function, 1, argv, &ret, &signal);
    failures += expect_signal("invalid nested argument", &signal, SIGNAL_UNICODE_ERROR);

    value_zero(&ret);
    value_zero(&signal);
    value_zero(&arg);

    rxvm_callfunc((void*)valid_nested_argument_function, 1, argv, &ret, &signal);
    failures += expect_signal("valid nested argument", &signal, SIGNAL_NONE);

#ifndef NUTF8
    value_zero(&ret);
    value_zero(&signal);
    value_zero(&arg);

    set_string(&arg, "e\xcc\x81", 3u);
    mark_string_normalization_certificates(
        &arg, RXFLAG_LANG_NORMAL_FORM_MASK);
    rxvm_callfunc((void*)mutate_valid_non_ascii_argument_function,
                  1, argv, &ret, &signal);
    failures += expect_signal("valid direct native mutation",
                              &signal, SIGNAL_NONE);
    if (arg.status.all_type_flags & RXFLAG_LANG_NORMAL_FORM_MASK) {
        fprintf(stderr,
                "valid direct native mutation retained normalization certificates\n");
        failures++;
    }
#endif

    value_zero(&ret);
    value_zero(&signal);
    value_zero(&arg);

    set_disable_env("1");
    rxvm_callfunc((void*)invalid_return_function, 0, NULL, &ret, &signal);
    failures += expect_signal("disabled invalid return check", &signal, SIGNAL_NONE);
    set_disable_env(NULL);

    clear_value(&ret);
    clear_value(&signal);
    clear_value(&arg);
    return failures ? 1 : 0;
}
