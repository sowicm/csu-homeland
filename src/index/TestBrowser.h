
#ifndef TESTBROWSER_H
#define TESTBROWSER_H
#include "SCGIProc.h"

class TestBrowser : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char*, SCGIEnv&, SCGIIn&, SCGIOut& out);
};


#endif // TESTBROWSER_H
