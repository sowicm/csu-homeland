
#include "Feedback.h"
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SPost.h"

void Feedback::attach(SCGIApp* app) const
{
    app->insert("/enlist_submit", procEnlist);
    app->insert("/suggestion_submit", procSuggestion);
}

void Feedback::procEnlist(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SPost post(in, env);
    out.puts("\r\n");
    sqlMutex.lock();
    sql.queryf("call enlist_submit(%s,%s,%s,%s,%s,%s)", post["email"], post["stuid"], post["phone"], post["txt"], env.RemoteAddr(), env.UserAgent());
    sql.clear_res();
    sqlMutex.unlock();
}

void Feedback::procSuggestion(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SPost post(in, env);
    out.puts("\r\n");
    sqlMutex.lock();
    sql.queryf("call suggestion_submit(%s,%s,%s,%s)", post["email"], post["txt"], env.RemoteAddr(), env.UserAgent());
    sqlMutex.unlock();
}
