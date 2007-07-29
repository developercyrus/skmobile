#include "prtypes.h"
#include "plstr.h"

PRUint32 PL_strnlen(const char *str, PRUint32 max)
{
	register const char *s;

	if( (const char *)0 == str ) return 0;
	for( s = str; max && *s; s++, max-- )
		;

	return (PRUint32)(s - str);
}


char * PL_strndup(const char *s, PRUint32 max)
{
	char *rv;
	size_t l;

	if( (const char *)0 == s )
		s = "";

	l = PL_strnlen(s, max);

	rv = (char *)malloc(l+1);
	if( (char *)0 == rv ) return rv;

	(void)memcpy(rv, s, l);
	rv[l] = '\0';

	return rv;
}