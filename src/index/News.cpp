
#include "News.h"
#include <regex>
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIIn.h"
#include "SCGIOut.h"

void News::attach(SCGIApp* app) const
{
    app->match("/news/(\\d+)", proc);
}

void News::proc(cmatch& mr, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    out.headerContentType("text/html");
    char path[261];
    sprintf(path, "../data/news/%s.html", mr[1].first);
    out.inc(path);
}
