
#include "Login.h"
#include "algorithm/md5.h"
#include "SCGIEnv.h"
#include "SCGIIn.h"
#include "SCGIOut.h"
#include "SCGIApp.h"
#include "SPost.h"
#include "SMySQL.h"
#include "debuglog.h"
#include "sqlMutex.h"
#include "SCookie.h"
#include "user.h"

void Login::attach(SCGIApp* app) const
{
    app->insert("/login", procLogin);
    app->insert("/logout", procLogout);
}

void Login::procLogin(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SPost post(in, env);
    const char* id  = post["id"];
    const char* pwd = post["pwd"];
    const char* r   = post["r"];  // remember
    char ret;

    // 4 fun
    if (strstr(pwd, " or ") || strstr(id, " or "))
    {
        ret = '2';
    }
    else if (id == SPost::null || pwd == SPost::null || r == SPost::null)
    {
        ret = '0';
    }
    else
    {
        bool b = false;
        SMySQLRes res;
        sqlMutex.lock();
        if (sql.queryf("call user_login(%s,%s,%s,%s,%s)", id, pwd, md5(pwd).c_str(), env.RemoteAddr(), env.UserAgent()))
        {
            res = sql.store_res();
            sql.clear_res();
        }
        sqlMutex.unlock();
        if (res)
        {
            auto row = res.fetch_row();
            if (row != nullptr)
            {
                b = true;
                debuglog("cookie[");
                SCookie cookie(env, out);
                debuglog("]\nuser[");
                User u(cookie, env, false);
                debuglog("]\n");
                u.uid = row[0];
                debuglog("save[");
                u.save(r[0] == '1');
                debuglog("]\n");
            }
        }
        ret = (b ? '1' : '0');
    }
    out.headerContentType("text/plain");
    out.putchar(ret);
}

void Login::procLogout(const char*, SCGIEnv& env, SCGIIn&, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    u.destroy();
    out.puts("Location: ../\r\n\r\n");
}
