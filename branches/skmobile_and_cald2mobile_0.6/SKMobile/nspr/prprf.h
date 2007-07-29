#ifndef prprf_h___
#define prprf_h___

/*
** API for PR printf like routines. Supports the following formats
**	%d - decimal
**	%u - unsigned decimal
**	%x - unsigned hex
**	%X - unsigned uppercase hex
**	%o - unsigned octal
**	%hd, %hu, %hx, %hX, %ho - 16-bit versions of above
**	%ld, %lu, %lx, %lX, %lo - 32-bit versions of above
**	%lld, %llu, %llx, %llX, %llo - 64 bit versions of above
**	%s - string
**	%c - character
**	%p - pointer (deals with machine dependent pointer size)
**	%f - float
**	%g - float
*/
#include "prtypes.h"
#include "prio.h"
#include <stdio.h>
#include <stdarg.h>

PR_BEGIN_EXTERN_C

/*
** sprintf into a fixed size buffer. Guarantees that a NUL is at the end
** of the buffer. Returns the length of the written output, NOT including
** the NUL, or (PRUint32)-1 if an error occurs.
*/
#define PR_snprintf					_snprintf

/*
** sprintf into a PR_MALLOC'd buffer. Return a pointer to the malloc'd
** buffer on success, NULL on failure. Call "PR_smprintf_free" to release
** the memory returned.
*/
NSPR_API(char*) PR_smprintf(const char *fmt, ...);

/*
** Free the memory allocated, for the caller, by PR_smprintf
*/
NSPR_API(void) PR_smprintf_free(char *mem);

/*
** "append" sprintf into a PR_MALLOC'd buffer. "last" is the last value of
** the PR_MALLOC'd buffer. sprintf will append data to the end of last,
** growing it as necessary using realloc. If last is NULL, PR_sprintf_append
** will allocate the initial string. The return value is the new value of
** last for subsequent calls, or NULL if there is a malloc failure.
*/
NSPR_API(char*) PR_sprintf_append(char *last, const char *fmt, ...);

/*
** va_list forms of the above.
*/
#define PR_vsnprintf	_vsnprintf

NSPR_API(char*) PR_vsmprintf(const char *fmt, va_list ap);
NSPR_API(char*) PR_vsprintf_append(char *last, const char *fmt, va_list ap);

PR_END_EXTERN_C

#endif /* prprf_h___ */
