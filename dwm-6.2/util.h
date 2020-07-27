/* See LICENSE file for copyright and license details. */

//#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MAX(A, B) \
   ({ __auto_type __A = (A); \
       __auto_type  __B = (B); \
     __A > __B ? __A : __B; })
//#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define MIN(A, B) \
   ({ __auto_type __A = (A); \
       __auto_type  __B = (B); \
     __A < __B ? __A : __B; })
//#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define BETWEEN(X, A, B) \
   ({ __auto_type __X = (X); \
       __auto_type __A = (A); \
        __auto_type __B = (B); \
     __A <= __X && __X <= __B; })
#define SWAP(A, B) \
   ({ __auto_type C = A; \
       A = B; B = C; })

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
