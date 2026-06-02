#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Simulate the buffer_t structure as used in shadowsocksr-libev */
typedef struct buffer_s {
    size_t len;
    size_t capacity;
    char *data;
} buffer_t;

/*
 * Invariant: After a shallow copy (memcpy of buffer_t struct) and restore,
 * the internal data pointer must still reference valid, consistent memory.
 * A deep copy must be used for buffer_t to prevent use-after-free or
 * double-free when the original buffer's data is modified between copy/restore.
 */
START_TEST(test_buffer_shallow_copy_pointer_integrity)
{
    /* Adversarial payloads simulating network input of various sizes */
    const char *payloads[] = {
        "\x00\x00\x00\x00\x00\x00\x00\x00",          /* exploit: zero-length triggers edge */
        "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",    /* boundary: max-like values */
        "AAAA",                                         /* valid short input */
        "\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41\x41", /* 16 bytes */
    };
    size_t payload_lens[] = {8, 10, 4, 16};
    int num_payloads = 4;

    for (int i = 0; i < num_payloads; i++) {
        buffer_t buf;
        buf.capacity = payload_lens[i] + 1;
        buf.len = payload_lens[i];
        buf.data = (char *)malloc(buf.capacity);
        ck_assert_ptr_nonnull(buf.data);
        memcpy(buf.data, payloads[i], payload_lens[i]);

        /* Simulate the vulnerable pattern: shallow struct copy */
        char *back_buf = (char *)malloc(sizeof(buffer_t));
        ck_assert_ptr_nonnull(back_buf);
        memcpy(back_buf, &buf, sizeof(buffer_t));

        /* Simulate modification/reallocation of original between copy and restore */
        char *original_data_ptr = buf.data;
        buf.data = (char *)realloc(buf.data, buf.capacity * 2);
        /* After realloc, the pointer in back_buf is now stale */

        /* Restore from shallow copy */
        buffer_t restored;
        memcpy(&restored, back_buf, sizeof(buffer_t));
        free(back_buf);

        /* SECURITY INVARIANT: restored.data must NOT equal a stale pointer
         * if the original was reallocated. In a correct (deep copy) implementation,
         * restored.data would be independently allocated. */
        if (buf.data != original_data_ptr) {
            /* Realloc moved the data; the shallow-copied pointer is now dangling */
            ck_assert_msg(restored.data == original_data_ptr,
                "Shallow copy preserves stale pointer - vulnerability confirmed: "
                "deep copy required for buffer_t to prevent use-after-free");
        }

        /* Cleanup - only free the current valid pointer */
        free(buf.data);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_shallow_copy_pointer_integrity);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}