
/**
 * Original source:
 * 	https://www.mitchr.me/SS/exampleCode/AUPG/pcre_example.c.html
 *
 *
 * */

#ifndef WITHOUT_PCRE

#include "pcre_util.h"

int regex_destroy(Regex regex)
{
	if(NULL == regex)
	{
		return 0;
	}

	pcre_free(regex->regex);

	if(NULL != regex->opt_regex)
	{
	#ifdef PCRE_CONFIG_JIT
		pcre_free_study(regex->opt_regex);
	#else
		pcre_free(regex->opt_regex);
	#endif
	}

	return 0;
}

/* OPTIONS (second argument) (||'ed together) can be:
     PCRE_ANCHORED       -- Like adding ^ at start of pattern.
     PCRE_CASELESS       -- Like m//i
     PCRE_DOLLAR_ENDONLY -- Make $ match end of string regardless of \n's
                            No Perl equivalent.
     PCRE_DOTALL         -- Makes . match newlins too.  Like m//s
     PCRE_EXTENDED       -- Like m//x
     PCRE_EXTRA          --
     PCRE_MULTILINE      -- Like m//m
     PCRE_UNGREEDY       -- Set quantifiers to be ungreedy.  Individual quantifiers
                            may be set to be greedy if they are followed by "?".
     PCRE_UTF8           -- Work with UTF8 strings.
*/
int regex_compile(Regex regex, const char* expr, int options, const char** error)
{
	int pcreErrorOffset;

	// First, the regex string must be compiled.
	regex->regex = pcre_compile(expr, 0, error, &pcreErrorOffset, NULL);

	// pcre_compile returns NULL on error, and sets pcreErrorOffset & pcreErrorStr
	if(regex->regex == NULL)
	{
		return 1;
	}
	const char* pcreErrorStr;

	// Optimize the regex
	regex->opt_regex = pcre_study(regex->regex, 0, &pcreErrorStr);

	/* pcre_study() returns NULL for both errors and when it can not optimize the regex.  The last argument is how one checks for
	  errors (it is NULL if everything works, and points to an error string otherwise. */
	if(pcreErrorStr != NULL)
	{
		if(NULL != error)
		{
			*error = pcreErrorStr;
		}

		regex_destroy(regex);
		return 1;
	}

	return 0;
}

/**
 * Returns PCRE_ERROR_*, 0 on success.
 * */
int regex_match(RegexMatcher dst, Regex regex, const char* input)
{
	int pcreExecRet;

	/* Try to find the regex in aLineToMatch, and report results. */
	pcreExecRet = pcre_exec
	(
		regex->regex,
		regex->opt_regex,
		input,
		strlen(input),			// length of string
		0,						// Start looking at this point
		0,						// OPTIONS
		dst->res_vec,
		REGEX_GROUP_MAX_LENGTH	// Length of subStrVec
	);

	dst->regex = regex;
	dst->str = input;

	if(pcreExecRet < 0)
	{
		// Something bad happened..
		return pcreExecRet;
	}
	else
	{
		if(pcreExecRet == 0)
		{
			// Set rc to the max number of substring matches possible.
			pcreExecRet = REGEX_GROUP_MAX_LENGTH;
		} /* end if */
	}

	dst->group_count = pcreExecRet;

	return 0;
}

int regex_get_group(RegexMatcher match, int grp, const char** dst)
{
	int ret = pcre_get_substring(match->str, match->res_vec, match->group_count, grp, dst);
	if(ret < 0)
	{
		return ret;
	}
	return 0;
}

int regex_get_named_group(RegexMatcher match, const char* group_name, const char** dst)
{
	int ret = pcre_get_named_substring
	(
		match->regex->regex,
		match->str,
		match->res_vec,
		match->group_count,
		group_name,
		dst
	);

	if(ret < 0)
	{
		return ret;
	}
	return 0;
}

void regex_free_group(const char* dst)
{
	pcre_free_substring(dst);
}

#endif
