
#include "bsearch.h"

void * bsearch (
    const void *key,
    const void *base,
    size_t num,
    size_t width,
    int (*compare)(const void *, const void *)
    )
{
    char *lo = (char *)base;
    char *hi = (char *)base + (num - 1) * width;
    char *mid;
    size_t half;
    int result;

    /*
    We allow a NULL key here because it breaks some older code and because we do not dereference
    this ourselves so we can't be sure that it's a problem for the comparison function
    */
    while (lo <= hi)
    {
        if ((half = num / 2) != 0)
        {
            mid = lo + (num & 1 ? half : (half - 1)) * width;
            if (!(result = compare(key, mid)))
                return(mid);
            else if (result < 0)
            {
                hi = mid - width;
                num = num & 1 ? half : half-1;
            }
            else
            {
                lo = mid + width;
                num = half;
            }
        }
        else if (num)
            return (compare(key, lo) ? 0 : lo);
        else
            break;
    }

    return 0;
}