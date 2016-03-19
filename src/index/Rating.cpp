
#include "Rating.h"
#include "SCGIApp.h"
#include "SCookie.h"
#include "SCGIIn.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SPost.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "user.h"
using std::string;

void Rating::attach(SCGIApp* app) const
{
    app->insert("/rating", proc);
}

void Rating::proc(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    out.headerContentType("text/plain");
    if (!u.loggedin)
    {
        out.putchar('0');
        return;
    }

    SPost post(in, env);
    const char* did    = post["did"];
    const char* score  = post["score"];
    int check;
    if (did == SPost::null || score == SPost::null || sscanf(score, "%d", &check) < 1 || check < 0 || check > 10)
        return;

    string stuid;
    SMySQLRes res;
    sqlMutex.lock();
    if (sql.queryf("select stuid from si_stu where did=%s", did))
    {
        res = sql.store_res();
        sql.clear_res();
    }
    sqlMutex.unlock();
    auto row = res.fetch_row();
    if (!row)
        return;
    stuid = row[0];
    res.free();

    sqlMutex.lock();
    sql.queryf("select id from ratings where uid=%d and stuid=%s and TO_DAYS(NOW())=TO_DAYS(time_prc) limit 1", u.uid.c_str(), stuid.c_str());
    res = sql.store_res();
    sqlMutex.unlock();
    row = res.fetch_row();
    if (row)
    {
        out.putchar('2');
        return;
    }

    sqlMutex.lock();
    sql.queryf("call rating(%d,%s,%d,%s,%s)", u.uid.c_str(), stuid.c_str(), score, env.RemoteAddr(), env.UserAgent());
    sql.clear_res();
    sqlMutex.unlock();
    out.putchar('1');
}
