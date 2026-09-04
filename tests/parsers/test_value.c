#include "glue.h"
#include "hammer.h"
#include "internal.h"
#include "test_suite.h"

#include <glib.h>

// Test value.c: parse_put edge cases (line 26)
static void test_value_put_edge_cases(gconstpointer backend) {
    HParserBackend be = (HParserBackend)GPOINTER_TO_INT(backend);

    HParser *put_null_parser = h_put_value__m(&system_allocator, NULL, "test_key");
    h_compile(put_null_parser, be, NULL);
    HParseResult *res = h_parse(put_null_parser, (const uint8_t *)"abc", 3);
    g_check_cmp_ptr(res, ==, NULL);

    HParser *p = h_ch('a');
    HParser *put_null_key = h_put_value__m(&system_allocator, p, NULL);
    h_compile(put_null_key, be, NULL);
    HParseResult *res2 = h_parse(put_null_key, (const uint8_t *)"a", 1);
    g_check_cmp_ptr(res2, ==, NULL);

    HParser *put1 = h_put_value(p, "existing_key");
    HParser *put2 = h_put_value(p, "existing_key");
    HParser *seq = h_sequence(put1, put2, NULL);
    h_compile(seq, be, NULL);
    HParseResult *res3 = h_parse(seq, (const uint8_t *)"aa", 2);
    if (res3) {
        h_parse_result_free(res3);
    }
}

// Test value.c: parse_get edge cases (line 54)
static void test_value_get_edge_cases(gconstpointer backend) {
    HParserBackend be = (HParserBackend)GPOINTER_TO_INT(backend);

    HParser *get_null_key = h_get_value__m(&system_allocator, NULL);
    h_compile(get_null_key, be, NULL);
    HParseResult *res = h_parse(get_null_key, (const uint8_t *)"", 0);
    g_check_cmp_ptr(res, ==, NULL);
}

// Test value.c: parse_free edge cases (line 86)
static void test_value_free_edge_cases(gconstpointer backend) {
    HParserBackend be = (HParserBackend)GPOINTER_TO_INT(backend);

    HParser *free_null_key = h_free_value__m(&system_allocator, NULL);
    h_compile(free_null_key, be, NULL);
    HParseResult *res = h_parse(free_null_key, (const uint8_t *)"", 0);
    g_check_cmp_ptr(res, ==, NULL);
}

static void test_value_string_keys_are_content_based(gconstpointer backend) {
    HParserBackend be = (HParserBackend)GPOINTER_TO_INT(backend);

    char put_key[] = "hello";
    char get_key[] = "hello";
    HParser *get_parser =
        h_sequence(h_put_value(h_ch('a'), put_key), h_get_value(get_key), NULL);
    h_compile(get_parser, be, NULL);
    HParseResult *res = h_parse(get_parser, (const uint8_t *)"a", 1);
    g_check_cmp_ptr(res, !=, NULL);
    if (res)
        h_parse_result_free(res);

    char free_put_key[] = "hello";
    char free_key[] = "hello";
    HParser *free_parser =
        h_sequence(h_put_value(h_ch('a'), free_put_key), h_free_value(free_key), NULL);
    h_compile(free_parser, be, NULL);
    res = h_parse(free_parser, (const uint8_t *)"a", 1);
    g_check_cmp_ptr(res, !=, NULL);
    if (res)
        h_parse_result_free(res);

    char remove_put_key[] = "hello";
    char remove_key[] = "hello";
    char after_remove_key[] = "hello";
    HParser *free_then_get = h_sequence(h_put_value(h_ch('a'), remove_put_key),
                                        h_free_value(remove_key),
                                        h_get_value(after_remove_key), NULL);
    h_compile(free_then_get, be, NULL);
    res = h_parse(free_then_get, (const uint8_t *)"a", 1);
    g_check_cmp_ptr(res, ==, NULL);
}

void register_value_tests(void) {
    g_test_add_data_func("/core/parser/packrat/value_put_edge_cases", GINT_TO_POINTER(PB_PACKRAT),
                         test_value_put_edge_cases);
    g_test_add_data_func("/core/parser/packrat/value_get_edge_cases", GINT_TO_POINTER(PB_PACKRAT),
                         test_value_get_edge_cases);
    g_test_add_data_func("/core/parser/packrat/value_free_edge_cases", GINT_TO_POINTER(PB_PACKRAT),
                         test_value_free_edge_cases);
    g_test_add_data_func("/core/parser/packrat/value_string_keys_are_content_based",
                         GINT_TO_POINTER(PB_PACKRAT), test_value_string_keys_are_content_based);
}
