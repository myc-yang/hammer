/* This tells the compiler to treat the entire file as a system header, which suppresses warnings for GCC extensions.
   This is needed to avoid error: #include_next is a GCC extension [-Werror]  */

#ifdef __GNUC__
#pragma GCC system_header
#endif

#if defined(_MSC_VER)
/* find or insert datum into search tree */
void *tsearch(const void *vkey, void **vrootp, int (*compar)(const void *, const void *));

/* delete node with given key */
void *tdelete(const void *vkey, void **vrootp, int (*compar)(const void *, const void *));

/* Walk the nodes of a tree */
void twalk(const void *vroot, void (*action)(const void *, VISIT, int));

#else
#if defined(__GNUC__) || defined(__clang__)
#include_next <search.h>
#else
#include <search.h>
#endif
#endif
