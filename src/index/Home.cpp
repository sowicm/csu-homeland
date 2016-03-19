
#include "Home.h"
#include "algorithm/md5.h"
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SPost.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SVars.h"
#include "user.h"

void Home::attach(SCGIApp* app) const
{
    app->insert("/home", proc);
}

void Home::proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    if (*s != '?')
    {
        SCookie cookie(env, out);
        User    u(cookie, env);
        if (!u.loggedin)
        {
            out.printf("Location: http://%s\r\n\r\n", env.Host());
            return;
        }

        SVars vars;
        u.gets();
        vars["name"] = u.display_name;
        vars["title"].assign(u8("中南大学家园网 - ")).append(u.display_name);

        out.headerContentType("text/html");
        switch (u.step)
        {
        case 0:
        case 3:
            if (!u.agreed)
            {
                vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" />";
                vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_reagree.js\"></script>";
                out.inc("../html/header.html", &vars);
                out.inc("../html/nav.html", &vars);
                out.inc("../html/p_reagree.html", &vars);
                vars["did"] = u.did;
                if (u.avatar_path == "null")
                    out.inc("../html/w_uploadphoto.html", &vars);
                out.inc("../html/footer.html", &vars);
                break;
            }

            vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" /><link rel=\"stylesheet\" type=\"text/css\" href=\"../css/says.css\" media=\"all\" />";
            vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_home.js\"></script><script type=\"text/javascript\" src=\"../js/says.js\"></script>";
            vars["did"] = u.did;
            vars["scorep"] = u.s_scoreCnt;
            vars["score"] = u.s_score;

            out.inc("../html/header.html", &vars);
            out.inc("../html/nav.html", &vars);
            out.inc("../html/p_home.html", &vars);
            if (u.avatar_path == "null")
                out.inc("../html/w_uploadphoto.html", &vars);
            out.inc("../html/footer.html", &vars);
            break;
        case 1:
            vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" />";
            vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_home_step1.js\"></script>";
            out.inc("../html/header.html", &vars);
            out.inc("../html/nav.html", &vars);
            out.inc("../html/p_home_step1.html", &vars);
            out.inc("../html/footer.html", &vars);
            break;
        case 2:
            vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" />";
            vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_home_step2.js\"></script>";
            out.inc("../html/header.html", &vars);
            out.inc("../html/nav.html", &vars);
            out.inc("../html/p_home_step2.html", &vars);
            out.inc("../html/footer.html", &vars);
            break;
        }
    }
    else
    {
        if (streq(s, "?say"))
        {
            SCookie cookie(env, out);
            User    u(cookie, env);
            out.puts("\r\n");
            if (!u.loggedin)
                return;
            SPost post(in, env);
            auto it = post.find("txt");
            if (it != post.end())
            {
                debuglog("say : %s\n", it->second.c_str());
                if (u.get_int("step") == S_STEP)
                {
                    sqlMutex.lock();
                    sql.queryf("call user_say(%d,%s,%s,%s)", u.uid.c_str(), it->second.c_str(), env.RemoteAddr(), env.UserAgent());
                    sql.clear_res();
                    sqlMutex.unlock();
                }
            }
        }
        else if (streq(s, "?reply"))
        {
            SCookie cookie(env, out);
            User    u(cookie, env);
            out.puts("\r\n");
            if (!u.loggedin)
                return;
            SPost post(in, env);
            SPost::iterator tid = post.find("tid");
            SPost::iterator txt = post.find("txt");
            if ((tid = post.find("tid")) != post.end() && ((txt = post.find("txt")) != post.end()))
            {
                if (u.get_int("step") == S_STEP)
                {
                    sqlMutex.lock();
                    sql.queryf("call user_reply(%d,%d,%s,%s,%s)", tid->second.c_str(), u.uid.c_str(), txt->second.c_str(), env.RemoteAddr(), env.UserAgent());
                    sql.clear_res();
                    sqlMutex.unlock();
                }
            }
        }
        else if (streq(s, "?createpass"))
        {
            SCookie cookie(env, out);
            User    u(cookie, env);
            SPost post(in, env);
            const char* pwd = post["pwd"];
            if (pwd == SPost::null || strlen(pwd) < 8)
                return;
            if (u.loggedin && u.get_int("step") == 2)
            {
                sqlMutex.lock();
                sql.queryf("call createpass(%d,%s,%s,%s,%s)",
                    u.uid.c_str(), pwd, md5(pwd).c_str(), env.RemoteAddr(), env.UserAgent());
                sql.clear_res();
                sqlMutex.unlock();
            }
            out.puts("\r\n\r\n");
        }
    }
}
