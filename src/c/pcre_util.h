
#ifndef PCRE_UTILS_H_
#define PCRE_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <pcre.h>

#define REGEX_GROUP_MAX_LENGTH 20

struct compiled_regex
{
	pcre* regex;
	pcre_extra* opt_regex;
};

typedef struct compiled_regex* Regex;

struct regex_matcher
{
	Regex regex;
	const char* str;
	int group_count;
	int res_vec[REGEX_GROUP_MAX_LENGTH];
};

typedef struct regex_matcher* RegexMatcher;


int regex_destroy(Regex regex);
int regex_compile(Regex regex, const char* expr, int options, const char** error);

int regex_match(RegexMatcher dst, Regex regex, const char* input);

int regex_get_group(RegexMatcher match, int grp, const char** dst);

int regex_get_named_group(RegexMatcher match, const char* group_name, const char** dst);

void regex_free_group(const char* dst);

#endif
