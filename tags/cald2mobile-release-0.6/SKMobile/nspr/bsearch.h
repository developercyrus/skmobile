#ifndef __SKC_BSEARCH_H_
#define __SKC_BSEARCH_H_

typedef int (*COMPARE_FUNC)(const void *, const void *);

void * bsearch (
	const void *key,
	const void *base,
	size_t nel,
	size_t size,
	COMPARE_FUNC);

#else
#error "Multiple inclusions of bsearch.h"
#endif
