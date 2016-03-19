
#ifndef OTHERS_H
#define OTHERS_H
#include "SCGIProc.h"

class Others : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 procResendAll(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procAgreement(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procReagree(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procAskFor(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procCheckFor(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procShutdown(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // OTHERS_H
