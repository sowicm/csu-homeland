
#include "TestBrowser.h"
#include "SCGIApp.h"
#include "SCGIOut.h"

void TestBrowser::attach(SCGIApp* app) const
{
    app->insert("/testbrowser", proc);
}

void TestBrowser::proc(const char*, SCGIEnv&, SCGIIn&, SCGIOut& out)
{
    out.headerContentType("text/html");
    out.inc("../html/testbrowser.html");
}
