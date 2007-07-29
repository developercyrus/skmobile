#pragma once

#define PL_strfree			free
#define PL_strlen			strlen
#define	PL_strcmp			strcmp
#define PL_strncmp			strncmp
#define PL_strchr			strchr
#define PL_strcpy			strcpy
#define	PL_strstr			strstr
#define PL_strncpy			strncpy
#define PL_strcasecmp		_stricmp
#define PL_strncasecmp		_strnicmp
#define PL_strdup			_strdup
#define PL_strrchr			strrchr

PRUint32 PL_strnlen(const char *str, PRUint32 max);

char * PL_strndup(const char *s, PRUint32 max);