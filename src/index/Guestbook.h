
#ifndef GUESTBOOK_H
#define GUESTBOOK_H
#include "SCGIProc.h"

class Guestbook : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // GUESTBOOK_H
