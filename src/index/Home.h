
#ifndef HOME_H
#define HOME_H
#include "SCGIProc.h"

class Home : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};


#endif // HOME_H
