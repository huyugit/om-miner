#ifndef MIN_MAX_H
#define MIN_MAX_H
/*
 * Defines min/max template functions.
 */

#if defined min || defined max
#undef min
#undef max
#endif


// Returns the lesser of x and y.
template<typename T>
inline const T& min(const T& x, const T& y)
{
    return (y < x) ? y : x;
}

// Returns the greater of x and y.
template<typename T>
inline const T& max(const T& x, const T& y)
{
    return (x > y) ? x : y;
}

#endif  // MIN_MAX_H
