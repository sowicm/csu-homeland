
#ifndef STRLESS_H
#define STRLESS_H
#include <functional>

struct strless : std::binary_function<const char*, const char*, bool>
{
    bool operator() (const char* x, const char* y) const
    {
        return strcmp(x, y) < 0;
    }
};

#endif // STRLESS_H