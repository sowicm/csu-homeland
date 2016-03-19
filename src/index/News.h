
#ifndef NEWS_H
#define NEWS_H
#include "SCGIProc.h"

class News : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 proc(cmatch& mr, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};


#endif // NEWS_H
