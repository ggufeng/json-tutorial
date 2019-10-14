#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v,
	const char* target, lept_type type)
{
	EXPECT(c, target[0]);
	const char* t = &target[1];
	while (*t != '\0')
	{
		if (*t != c->json[0])
			return LEPT_PARSE_INVALID_VALUE;
		c->json++;
		t++;
		
	}
	v->type = type;
	return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_is_digit(const char c) {
	return c >= '0' && c <= '9';
}

static const char* lept_skip_digits(const char* c)
{
	while(lept_is_digit(*c))
	{
		c++;
	}
	return c;
}

static int lept_is_non_negative_valid(const char* p)
{
	// check int
	if (*p == '0')
		p++;
	else if (lept_is_digit(*p))
	{
		p = lept_skip_digits(p);
	}
	else
		return 0;

	// has frac?
	if (*p == '.') 
	{
		p++;
		if (!lept_is_digit(*p))
			return 0;
		p = lept_skip_digits(p);
	} 

	// has exp?
	if (*p == 'e' || *p == 'E')
	{
		p++;
		if (*p == '-' || *p == '+')
			p++;
		if (!lept_is_digit(*p))
			return 0;
		p = lept_skip_digits(p);
	}

	// now should be end of the string.
	if (*p == '\0') 
		return 1;
	else
		return 0;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* validate number */
	const char* p = c->json;
	if (*p == '-')
		p++;

	if (!lept_is_non_negative_valid(p))
		return LEPT_PARSE_INVALID_VALUE;

    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
