/* See LICENSE file for copyright and license details. */

//#define MAX(A, B)               ((A) > (B) ? (A) : (B))
//#define MIN(A, B)               ((A) < (B) ? (A) : (B))
//#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define MAX(A, B)               ({ \
                                        __auto_type __A = (A); \
                                        __auto_type  __B = (B); \
\
                                        __A > __B ? __A : __B; \
                                })
#define MIN(A, B)               ({ \
                                        __auto_type __A = (A); \
                                        __auto_type  __B = (B); \
\
                                        __A < __B ? __A : __B; \
                                })
#define BETWEEN(X, A, B)        ({ \
                                        __auto_type __X = (X); \
\
                                        (A) <= __X && __X <= (B); \
                                })
#define SWAP(A, B)              ({ \
                                        __auto_type __A = (A); \
\
                                        (A) = (B); \
                                        (B) = __A; \
                                })

#define STR_HELPER(X)           #X
#define STR(X)                  STR_HELPER(X)

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
