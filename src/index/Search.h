

#ifndef SEARCH_H
#define SEARCH_H
#include "SCGIProc.h"

class Search : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // SEARCH_H
