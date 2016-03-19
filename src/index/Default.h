
#ifndef DEFAULT_H
#define DEFAULT_H
#include "SCGIProc.h"

class Default : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char*, SCGIEnv&, SCGIIn&, SCGIOut& out);
};

#endif // DEFAULT_H
