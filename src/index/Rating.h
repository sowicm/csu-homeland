
#ifndef RATING_H
#define RATING_H
#include "SCGIProc.h"

class Rating : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // RATING_H
