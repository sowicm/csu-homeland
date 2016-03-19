
#ifndef PROFILE_H
#define PROFILE_H
#include "SCGIProc.h"

class Profile : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // PROFILE_H
