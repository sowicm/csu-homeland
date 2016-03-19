
#ifndef UPAGE_H
#define UPAGE_H
#include "SCGIProc.h"

class UPage : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(cmatch& mr, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // UPAGE_H
