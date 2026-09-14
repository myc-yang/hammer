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

static void test_value_string_keys_are_content_based_and_owned(gconstpointer backend) {
    HParserBackend be = (HParserBackend)GPOINTER_TO_INT(backend);

    char put_key[] = "hello";
    char get_key[] = "hello";
    HParser *get_put_child = h_ch('a');
    HParser *get_put = h_put_value(get_put_child, put_key);
    HParser *get = h_get_value(get_key);
    HParser *get_parser = h_sequence(get_put, get, NULL);
    // Independent mutations make borrowed names compare unequal.
    put_key[0] = 'x';
    get_key[0] = 'y';
    h_compile(get_parser, be, NULL);
    HParseResult *res = h_parse(get_parser, (const uint8_t *)"a", 1);
    g_check_cmp_ptr(res, !=, NULL);
    if (res)
        h_parse_result_free(res);
    h_parser_free(get_parser);
    h_parser_free(get);
    h_parser_free(get_put);
    h_parser_free(get_put_child);

    char free_put_key[] = "hello";
    char free_key[] = "hello";
    HParser *free_put_child = h_ch('a');
    HParser *free_put = h_put_value(free_put_child, free_put_key);
    HParser *free = h_free_value(free_key);
    HParser *free_parser = h_sequence(free_put, free, NULL);
    free_put_key[0] = 'x';
    free_key[0] = 'y';
    h_compile(free_parser, be, NULL);
    res = h_parse(free_parser, (const uint8_t *)"a", 1);
    g_check_cmp_ptr(res, !=, NULL);
    if (res)
        h_parse_result_free(res);
    h_parser_free(free_parser);
    h_parser_free(free);
    h_parser_free(free_put);
    h_parser_free(free_put_child);

    char remove_put_key[] = "hello";
    char remove_key[] = "hello";
    char after_remove_key[] = "hello";
    HParser *remove_put_child = h_ch('a');
    HParser *remove_put = h_put_value(remove_put_child, remove_put_key);
    HParser *remove = h_free_value(remove_key);
    HParser *after_remove = h_get_value(after_remove_key);
    HParser *free_then_get = h_sequence(remove_put, remove, after_remove, NULL);
    remove_put_key[0] = 'x';
    remove_key[0] = 'x';
    after_remove_key[0] = 'x';
    h_compile(free_then_get, be, NULL);
    res = h_parse(free_then_get, (const uint8_t *)"a", 1);
    g_check_cmp_ptr(res, ==, NULL);
    h_parser_free(free_then_get);
    h_parser_free(after_remove);
    h_parser_free(remove);
    h_parser_free(remove_put);
    h_parser_free(remove_put_child);
}

void register_value_tests(void) {
    g_test_add_data_func("/core/parser/packrat/value_put_edge_cases", GINT_TO_POINTER(PB_PACKRAT),
                         test_value_put_edge_cases);
    g_test_add_data_func("/core/parser/packrat/value_get_edge_cases", GINT_TO_POINTER(PB_PACKRAT),
                         test_value_get_edge_cases);
    g_test_add_data_func("/core/parser/packrat/value_free_edge_cases", GINT_TO_POINTER(PB_PACKRAT),
                         test_value_free_edge_cases);
    g_test_add_data_func("/core/parser/packrat/value_string_keys_are_content_based_and_owned",
                         GINT_TO_POINTER(PB_PACKRAT),
                         test_value_string_keys_are_content_based_and_owned);
}
