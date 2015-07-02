
#ifndef _DD_TEST_UTIL_H__
#define _DD_TEST_UTIL_H__
#include <string>

#include <sys/time.h>

// when len <= prefixLen
//   test_str(adfadf, 3) == adf
// when len < 20
//   test_str(adf, 13) == adfa:67890123
// when len >= 20
//   test_str(asdf, 45) = asdf:=====(20_56789)(30_56789)12345
std::string test_str(const std::string& str_prefix, const size_t str_len);

/* get system time */
static inline void itimeofday(long *sec, long *usec)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec) *sec = time.tv_sec;
    if (usec) *usec = time.tv_usec;
}

/* get clock in millisecond 64 */
static inline uint64_t iclock64(void)
{
    long s, u;
    uint64_t value;
    itimeofday(&s, &u);
    value = ((uint64_t)s) * 1000 + (u / 1000);
    return value;
}

#endif
