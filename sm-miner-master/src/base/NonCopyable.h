#ifndef NON_COPYABLE_H
#define NON_COPYABLE_H
/*
 * Contains NonCopyable class declaration.
 * 
 * Based on the Boost noncopyable.hpp header file.
 * Copyright (C) Beman Dawes 1999-2003.
 * Distributed under the Boost Software License, Version 1.0.
 * (see http://www.boost.org/LICENSE_1_0.txt).
 */


// Private copy constructor and copy assignment ensure
// classes derived from class NonCopyable cannot be copied.
// Contributed by Dave Abrahams.
//
class NonCopyable
{
// Construction/destruction.
protected:
    NonCopyable()  {}
    ~NonCopyable()  {}

// Prevent copy and assignment.
private:
    NonCopyable(const NonCopyable& );
    NonCopyable& operator=(const NonCopyable& );
};

#endif  // NON_COPYABLE_H
