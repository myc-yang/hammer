/*
 * Edits made to add new combinator
 * Copyright (c) 2025 Riverside Research
 */

#include "../trace.h"
#include "parser_internal.h"

typedef struct {
    const HParser *p;
    const char *key;
} HStoredValue;

/* Allocate the value parser's environment and its key in one block so the
 * parser owns the key for its complete lifetime. */
static HStoredValue *new_stored_value(HAllocator *mm__, const HParser *p, const char *key) {
    size_t key_size = 0;
    if (key) {
        size_t key_length = strlen(key);
        if (key_length > SIZE_MAX - sizeof(HStoredValue) - 1)
            h_platform_errx(1, "value name allocation size overflow");
        key_size = key_length + 1;
    }
    HStoredValue *env = h_alloc(mm__, sizeof(*env) + key_size);

    env->p = p;
    if (key) {
        char *owned_key = (char *)(env + 1);
        memcpy(owned_key, key, key_size);
        env->key = owned_key;
    } else {
        env->key = NULL;
    }

    return env;
}

/* Stash an HParseResult into a symbol table, so that it can be
   retrieved and used later. */

static HParseResult *parse_put_value(void *env, HParseState *state) {
    HStoredValue *s = (HStoredValue *)env;
    size_t at = state->input_stream.pos + state->input_stream.index;
    if (!s->p || !s->key) {
        h_trace_note_failure(state->input_stream.trace, H_PARSE_ERROR_REUSED_NAME,
                             "value storage requires both a parser and a name", at, at);
        return NULL;
    }
    if (!h_symbol_get(state, s->key)) {
        HParseResult *tmp = h_do_parse(s->p, state);
        if (tmp) {
            h_symbol_put(state, s->key, tmp);
        }
        return tmp;
    }
    h_trace_note_failure(state->input_stream.trace, H_PARSE_ERROR_REUSED_NAME,
                         "a value is already stored under that name", at, at);
    return NULL;
}

static const HParserVtable put_vt = {
    .name = "h_put_value",
    .parse = parse_put_value,
    .isValidRegular = h_false,
    .isValidCF = h_false,
    .higher = true,
};

HParser *h_put_value(const HParser *p, const char *name) {
    return h_put_value__m(&system_allocator, p, name);
}

HParser *h_put_value__m(HAllocator *mm__, const HParser *p, const char *name) {
    return h_new_parser(mm__, &put_vt, new_stored_value(mm__, p, name));
}

/* Retrieve a stashed result from the symbol table. */

static HParseResult *parse_get_value(void *env, HParseState *state) {
    HStoredValue *s = (HStoredValue *)env;
    size_t at = state->input_stream.pos + state->input_stream.index;
    if (s->p || !s->key) {
        h_trace_note_failure(state->input_stream.trace, H_PARSE_ERROR_NO_VALUE,
                             "value lookup requires a name", at, at);
        return NULL;
    }
    HParseResult *result = h_symbol_get(state, s->key);
    if (!result)
        h_trace_note_failure(state->input_stream.trace, H_PARSE_ERROR_NO_VALUE,
                             "no value is stored under that name", at, at);
    return result;
}

static const HParserVtable get_vt = {
    .name = "h_get_value",
    .parse = parse_get_value,
    .isValidRegular = h_false,
    .isValidCF = h_false,
    .higher = true,
};

HParser *h_get_value(const char *name) { return h_get_value__m(&system_allocator, name); }

HParser *h_get_value__m(HAllocator *mm__, const char *name) {
    return h_new_parser(mm__, &get_vt, new_stored_value(mm__, NULL, name));
}

/*
  Retrieve a stashed symbol result from the symbol table
  Remove the retrieved result from the symbol table
*/

static HParseResult *parse_free_value(void *env, HParseState *state) {
    HStoredValue *s = (HStoredValue *)env;
    size_t at = state->input_stream.pos + state->input_stream.index;
    if (s->p || !s->key) {
        h_trace_note_failure(state->input_stream.trace, H_PARSE_ERROR_NO_VALUE,
                             "value removal requires a name", at, at);
        return NULL;
    }
    HParseResult *stored_result = h_symbol_get(state, s->key);
    if (!stored_result) {
        h_trace_note_failure(state->input_stream.trace, H_PARSE_ERROR_NO_VALUE,
                             "no value is stored under that name", at, at);
        return NULL;
    }
    h_symbol_free(state, s->key);
    return stored_result;
}

static const HParserVtable free_vt = {
    .name = "h_free_value",
    .parse = parse_free_value,
    .isValidRegular = h_false,
    .isValidCF = h_false,
    .higher = true,
};

HParser *h_free_value(const char *name) { return h_free_value__m(&system_allocator, name); }

HParser *h_free_value__m(HAllocator *mm__, const char *name) {
    return h_new_parser(mm__, &free_vt, new_stored_value(mm__, NULL, name));
}

bool h_is_get_value_parser(const HParser *parser) {
    return parser && (parser->vtable == &free_vt || parser->vtable == &get_vt);
}

bool h_is_put_value_parser(const HParser *parser) { return parser && (parser->vtable == &put_vt); }
