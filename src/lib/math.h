#ifndef MATH_H
#define MATH_H

template<typename T, typename S>
static inline T min(const T& x, const S& y) { return x < y ? x : y; }

template<typename T, typename S>
static inline T max(const T& x, const S& y) { return x > y ? x : y; }

template<typename T>
static inline T abs(const T& x) { return x > 0 ? x : -x; }

#endif
