#include "static_alloc.h"
#include "test_suite.h"

#include <stdint.h>

#ifndef RTEMS_BUILD
static void test_static_alloc_rejects_alignment_overflow(void) {
    if (g_test_subprocess()) {
        h_static_alloc_init();
        h_grammar_allocator->alloc(h_grammar_allocator, SIZE_MAX);
        g_error("static allocator accepted a SIZE_MAX request");
    }

    g_test_trap_subprocess(NULL, 0, (GTestSubprocessFlags)0);
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*pool request too large*");
}

static void test_static_alloc_rejects_capacity_addition_overflow(void) {
    if (g_test_subprocess()) {
        h_static_alloc_init();
        h_grammar_allocator->alloc(h_grammar_allocator, 1);
        h_grammar_allocator->alloc(h_grammar_allocator, SIZE_MAX - 7);
        g_error("static allocator accepted an overflowing capacity request");
    }

    g_test_trap_subprocess(NULL, 0, (GTestSubprocessFlags)0);
    g_test_trap_assert_failed();
    g_test_trap_assert_stderr("*pool exhausted*");
}

void register_static_alloc_tests(void) {
    g_test_add_func("/core/static_alloc/alignment_overflow",
                    test_static_alloc_rejects_alignment_overflow);
    g_test_add_func("/core/static_alloc/capacity_addition_overflow",
                    test_static_alloc_rejects_capacity_addition_overflow);
}
#endif
