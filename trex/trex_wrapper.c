/* Some wrapper functions to make binding with
 * Tumbleweed easier.
 */

#include "trex.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

TREX_API char* trex_getsubexp_string(TRex* exp, int n)
{
	TRexMatch match;
	char* result;

	if(trex_getsubexp(exp, n, &match))
	{
		result = malloc(sizeof(char) * match.len+1);
		strncpy(result, match.begin, match.len);
		printf("match: %d\n", match.len);

		return result;
	}
	else
		return strdup("");
}
