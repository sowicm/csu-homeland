
#include "Search.h"
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIIn.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SVars.h"
#include "SGet.h"
#include "user.h"

void Search::attach(SCGIApp* app) const
{
    app->insert("/search", proc);
}

void Search::proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    if (*s == '?')
    {
        SCookie cookie(env, out);
        User    u(cookie, env);
        if (!u.loggedin)
        {
            out.printf("Location: http://%s\r\n\r\n", env.Host());
            return;
        }
        out.headerContentType("text/html");
        debuglog("search: %s\n", s);
        SVars vars;
        u.gets();
        if (u.step == S_STEP)
        {
            vars["title"].assign(u8("中南大学家园网 - 搜索结果"));
            vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_search.css\" media=\"all\" />";
            vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_search.js\"></script>";

            out.inc("../html/header.html", &vars);
            out.inc("../html/nav.html", &vars);
            out.inc("../html/p_search_h.html", &vars);

            bool allow = true;
            if (!u.email_validated)
            {
                SMySQLRes res;
                sqlMutex.lock();
                sql.queryf("select uid from log_search_stu where uid=%d", u.uid.c_str());
                res = sql.store_res();
                sqlMutex.unlock();
                auto row = res.fetch_row();
                if (row)
                {
                    allow = false;
                }
            }

            if (allow)
            {
                SGet get(env);
                const char* name = get["n"];
                if (name != SGet::null)
                {
                    SMySQLRes res;
                    sqlMutex.lock();
                    if (sql.queryf("call search(%s,%d,%s,%s)", name, u.uid.c_str(), env.RemoteAddr(), env.UserAgent()))
                    {
                        res = sql.store_res();
                        sql.clear_res();
                    }
                    sqlMutex.unlock();
                    MYSQL_ROW row;
                    char *pcur;
                    while (row = res.fetch_row())
                    {
                        vars["did"] = row[0];
                        vars["name"] = row[1];

                        pcur = row[2];
                        char*& academy = row[2];
                        while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
                            ++pcur;
                        if (!*pcur)
                            continue;
                        char *grade = pcur;
                        while (*pcur && (*pcur >= '0' && *pcur <= '9'))
                            ++pcur;
                        if (!*pcur)
                            continue;
                        *pcur = 0;
                        vars["grade"] = grade;
                        grade[0] = 0;
                        vars["academy"] = academy;
                        ++pcur;
                        if (!*pcur)
                            continue;
                        ++pcur;
                        if (!*pcur)
                            continue;
                        ++pcur;
                        if (!*pcur)
                            continue;
                        char *pro = pcur;
                        while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
                            ++pcur;
                        if (!*pcur)
                            continue;
                        *pcur = 0;
                        vars["pro"] = pro;
                        out.inc("../html/li_search_result.html", &vars);
                    }
                }
            }
            else
            {
                out.puts(u8("未通过邮箱验证的用户，限使用1次搜索！"));
            }
            out.inc("../html/p_search_f.html", &vars);
            out.inc("../html/footer.html", &vars);
        }
    }
    else
    {
        SCookie cookie(env, out);
        User    u(cookie, env);
        if (!u.loggedin)
        {
            out.printf("Location: http://%s\r\n\r\n", env.Host());
            return;
        }
        if (u.get_int("step") != S_STEP)
        {
            out.puts("Location: home\r\n\r\n");
            return;
        }

        SVars vars;
        out.headerContentType("text/html");
        vars["title"].assign(u8("中南大学家园网 - 搜索"));
        vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_search.css\" media=\"all\" />";
        vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_search.js\"></script>";

        out.inc("../html/header.html", &vars);
        out.inc("../html/nav.html", &vars);
        out.inc("../html/p_search_h.html", &vars);
        out.inc("../html/p_search_f.html", &vars);
        out.inc("../html/footer.html", &vars);
    }
}
