/* Release 1 .int ABI, parser, and checked-arithmetic contract. */

#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "rxvalue.h"
#include "crexxpa.h"
#include "rxbin.h"

static int failures;

static void expect(int condition, const char *message) {
    if (condition) return;
    fprintf(stderr, "%s\n", message);
    failures++;
}

static int parse_exact(const char *text, rxinteger *value) {
    char *end;
    end = (char *)text;
    return rxinteger_parse(text, &end, value) == 0 && *end == 0;
}

int main(void) {
    rxinteger value;
    rxinteger result;
    char buffer[32];

    failures = 0;
    expect(sizeof(rxinteger) == 8, ".int ABI is not 64-bit");
    expect(sizeof(((bin_code *)0)->iconst) == 8, "RXBIN integer operand is not 64-bit");
    expect(RXINTEGER_MAX == INT64_MAX, ".int maximum is not INT64_MAX");
    expect(RXINTEGER_MIN == INT64_MIN, ".int minimum is not INT64_MIN");

    expect(parse_exact("9223372036854775807", &value) && value == RXINTEGER_MAX,
           "failed to parse .int maximum");
    expect(parse_exact("-9223372036854775808", &value) && value == RXINTEGER_MIN,
           "failed to parse .int minimum");
    expect(!parse_exact("9223372036854775808", &value),
           "accepted positive .int overflow");
    expect(!parse_exact("-9223372036854775809", &value),
           "accepted negative .int overflow");

    snprintf(buffer, sizeof(buffer), "%" RXINTEGER_PRI, RXINTEGER_MIN);
    expect(strcmp(buffer, "-9223372036854775808") == 0,
           "failed to format .int minimum");
    snprintf(buffer, sizeof(buffer), "%" RXINTEGER_PRI, RXINTEGER_MAX);
    expect(strcmp(buffer, "9223372036854775807") == 0,
           "failed to format .int maximum");

    expect(rxinteger_checked_add(RXINTEGER_MAX - 1, 1, &result) && result == RXINTEGER_MAX,
           "checked addition rejected maximum");
    expect(!rxinteger_checked_add(RXINTEGER_MAX, 1, &result),
           "checked addition accepted overflow");
    expect(rxinteger_checked_sub(RXINTEGER_MIN + 1, 1, &result) && result == RXINTEGER_MIN,
           "checked subtraction rejected minimum");
    expect(!rxinteger_checked_sub(RXINTEGER_MIN, 1, &result),
           "checked subtraction accepted overflow");
    expect(rxinteger_checked_mul(INT64_C(3037000499), INT64_C(3037000499), &result),
           "checked multiplication rejected in-range product");
    expect(rxinteger_checked_mul(RXINTEGER_MIN, 1, &result) && result == RXINTEGER_MIN,
           "checked multiplication rejected minimum");
    expect(!rxinteger_checked_mul(RXINTEGER_MAX, 2, &result),
           "checked multiplication accepted overflow");
    expect(!rxinteger_checked_mul(RXINTEGER_MIN, -1, &result),
           "checked multiplication accepted negative-limit overflow");
    expect(!rxinteger_checked_neg(RXINTEGER_MIN, &result),
           "checked negation accepted overflow");
    expect(rxinteger_checked_pow(2, 62, &result) &&
               result == INT64_C(4611686018427387904),
           "checked power rejected in-range result");
    expect(!rxinteger_checked_pow(2, 63, &result),
           "checked power accepted overflow");

    return failures ? 1 : 0;
}
