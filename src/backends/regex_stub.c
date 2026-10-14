/*
 * RTEMS supports only the packrat backend.  Generic parser vtables still
 * contain their regular-expression compiler callbacks, so their object files
 * retain references to these internal RVM helpers.  Provide inert helpers to
 * satisfy those references without linking the regex backend itself.
 */
#include "regex.h"

bool h_compile_regex(HRVMProg *prog, const HParser *parser) {
    (void)prog;
    (void)parser;
    return false;
}

uint16_t h_rvm_create_action(HRVMProg *prog, HSVMActionFunc action_func, void *env) {
    (void)prog;
    (void)action_func;
    (void)env;
    return 0;
}

void *h_rvm_alloc(HRVMProg *prog, size_t size) {
    (void)prog;
    (void)size;
    return NULL;
}

uint16_t h_rvm_insert_insn(HRVMProg *prog, HRVMOp op, uint16_t arg) {
    (void)prog;
    (void)op;
    (void)arg;
    return 0;
}

uint16_t h_rvm_get_ip(HRVMProg *prog) {
    (void)prog;
    return 0;
}

void h_rvm_patch_arg(HRVMProg *prog, uint16_t ip, uint16_t new_val) {
    (void)prog;
    (void)ip;
    (void)new_val;
}

bool h_svm_action_make_sequence(HArena *arena, HSVMContext *ctx, void *env) {
    (void)arena;
    (void)ctx;
    (void)env;
    return false;
}

bool h_svm_action_clear_to_mark(HArena *arena, HSVMContext *ctx, void *env) {
    (void)arena;
    (void)ctx;
    (void)env;
    return false;
}
