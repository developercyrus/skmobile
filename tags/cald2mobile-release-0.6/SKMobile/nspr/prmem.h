#pragma once

#define PR_Malloc					malloc
#define PR_Calloc					calloc
#define PR_Realloc					realloc
#define PR_Free						free
#define PR_MALLOC(_bytes)			(PR_Malloc((_bytes)))
#define PR_NEW(_struct)				((_struct *) PR_MALLOC(sizeof(_struct)))
#define PR_REALLOC(_ptr, _size)		(PR_Realloc((_ptr), (_size)))
#define PR_CALLOC(_size)			(PR_Calloc(1, (_size)))
#define PR_NEWZAP(_struct)			((_struct*)PR_Calloc(1, sizeof(_struct)))
#define PR_DELETE(_ptr)				{ PR_Free(_ptr); (_ptr) = NULL; }
#define PR_FREEIF(_ptr)				if (_ptr) PR_DELETE(_ptr)