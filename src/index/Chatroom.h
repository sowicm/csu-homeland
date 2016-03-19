
#ifndef CHATROOM_H
#define CHATROOM_H
#include "SCGIProc.h"

class Chatroom : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procComet(const char*, SCGIEnv& env, SCGIIn&, SCGIOut& out);
};

#endif // CHATROOM_H
