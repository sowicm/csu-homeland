
#ifndef LOGIN_H
#define LOGIN_H
#include "SCGIProc.h"

class Login : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;\
    static void                 procLogin(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procLogout(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // LOGIN_H
