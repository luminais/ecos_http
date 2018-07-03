
#ifndef _json_inttypes_h_
#define _json_inttypes_h_

#include "json_config.h"

#if defined(_MSC_VER) && _MSC_VER < 1700

/* Anything less than Visual Studio C++ 10 is missing stdint.h and inttypes.h */
typedef __int32 int32_t;
#define INT32_MIN    ((int32_t)_I32_MIN)
#define INT32_MAX    ((int32_t)_I32_MAX)
typedef __int64 int64_t;
#define INT64_MIN    ((int64_t)_I64_MIN)
#define INT64_MAX    ((int64_t)_I64_MAX)
#define PRId64 "I64d"
#define SCNd64 "I64d"

#else
#define PRId64 "lld"
#define SCNd64 "lld"

#ifdef JSON_C_HAVE_INTTYPES_H
#include <inttypes.h>
#endif
/* inttypes.h includes stdint.h */
#ifndef INT32_MIN
# define INT32_MIN              (-2147483647-1)
#endif

#ifndef INT32_MAX
# define INT32_MAX              (2147483647)
#endif


#  define __INT64_C(c)  c ## LL
#  define __UINT64_C(c) c ## ULL

# define INT64_MIN              (-__INT64_C(9223372036854775807)-1)
# define INT64_MAX              (__INT64_C(9223372036854775807))


#endif

#endif
