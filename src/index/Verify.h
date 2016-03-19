
#ifndef VERIFY_H
#define VERIFY_H
#include "SCGIProc.h"

class Verify : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char* s, SCGIEnv&, SCGIIn&, SCGIOut& out);
    static void                 procSend(const char*, SCGIEnv&, SCGIIn&, SCGIOut& out);
    static void                 procSkip(const char*, SCGIEnv&, SCGIIn&, SCGIOut& out);
};

#endif // VERIFY_H
