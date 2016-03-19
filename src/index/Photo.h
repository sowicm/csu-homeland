
#ifndef PHOTO_H
#define PHOTO_H
#include "SCGIProc.h"

class Photo : public SCGIProc
{
public:
    virtual void                attach(SCGIApp* app) const;
    static void                 procShow(cmatch& mr, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procUpload(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
    static void                 procChoose(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out);
};

#endif
