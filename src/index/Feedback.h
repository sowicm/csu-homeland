
#ifndef FEEDBACK_H
#define FEEDBACK_H
#include "SCGIProc.h"

class Feedback : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 procEnlist(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procSuggestion(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif // FEEDBACK_H
