#include <stdio.h>
#include <string.h>

void print_hash(const char* key) {
    int h = 0;
    int len = strlen(key);
    for (int i = 0; i < len; i++) {
        h = h * 31 + key[i];
    }
    int rem = h % 256;
    if (rem < 0) rem = -rem;
    printf("Hash for '%s': h=%d, bucket=%d\n", key, h, rem + 1);
}

int main() {
    print_hash("mykey");
    print_hash("hello");
    print_hash("test");
    print_hash("key1");
    print_hash("longstring_to_test_hash_function_123");
    return 0;
}
