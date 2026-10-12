#if defined(RTEMS_BUILD) && defined(STATIC_BUILD)
#include "static_alloc.h"
#include "test_suite.h"

#include <stdint.h>

static void test_static_alloc_pool_accounting(void) {
    h_static_alloc_init();

    void *grammar = h_grammar_allocator->alloc(h_grammar_allocator, 1);
    void *parse = h_parse_allocator->alloc(h_parse_allocator, 1);

    g_assert_nonnull(grammar);
    g_assert_nonnull(parse);
    g_assert_cmpuint((uintptr_t)grammar % 8u, ==, 0);
    g_assert_cmpuint((uintptr_t)parse % 8u, ==, 0);
    g_assert_cmpuint(h_grammar_pool_used(), ==, 8);
    g_assert_cmpuint(h_parse_pool_used(), ==, 8);
    g_assert_cmpuint(h_parse_pool_high_water(), ==, 8);

    h_parse_pool_reset();
    g_assert_cmpuint(h_parse_pool_used(), ==, 0);
    g_assert_cmpuint(h_parse_pool_high_water(), ==, 8);
}

void register_static_alloc_tests(void) {
    g_test_add_func("/core/static_alloc/pool_accounting", test_static_alloc_pool_accounting);
}
#endif /* defined(RTEMS_BUILD) && defined(STATIC_BUILD) */
