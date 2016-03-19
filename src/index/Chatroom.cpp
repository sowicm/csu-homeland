
#include <time.h>                   // clock
#include <string>
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SPost.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SVars.h"
#include "user.h"
using std::string;

namespace {
void proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SVars   vars;
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

    if (!(*s))
    {
        out.headerContentType("text/html");
        vars["title"].assign(u8("中南大学家园网 - 聊天室"));
        vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_chatroom.css\" media=\"all\" />";
        //vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_chatroom.js\"></script>";

        out.inc("../html/header.html", &vars);
        out.inc("../html/nav.html", &vars);
        out.inc("../html/p_chatroom.html", &vars);
        out.inc("../html/footer.html", &vars);
        return;
    }

    int room;
    if (sscanf(s, "?r=%d", &room) < 1 || room < 0 || room > 100)
    {
        out.puts("Location: home\r\n\r\n");
        return;
    }
    char sroom[5];
    sprintf(sroom, "%d", room);

    out.headerContentType("text/html");

    SMySQLRes res;
    sqlMutex.lock();
    if (sql.queryf("select rid,status from chatroom_users where uid=%d", u.uid.c_str()))
    {
        res = sql.store_res();
        sql.clear_res();
    }
    sqlMutex.unlock();
    auto row = res.fetch_row();
    if (row)
    {
        if (strcmp(row[0], sroom) && strcmp(row[0], "disconnected"))
        {
            out.puts(u8("请先退出当前已登录的聊天室，再登录下一个聊天室"));
            return;
        }
    }
    else
    {
        sqlMutex.lock();
        sql.queryf(
            "insert into chatroom_users (rid,uid,status,joinedtime_prc,lastcomettime_prc,ip,agent)"
            "values (%d,%d,%s,NOW(),NOW(),%s,%s)",
            sroom, u.uid.c_str(), "connected", env.RemoteAddr(), env.UserAgent(), u.uid.c_str());
        sqlMutex.unlock();
    }

    SPost post(in, env);

    auto it = post.find("txt");
    if (it != post.end())
    {
        sqlMutex.lock();
        sql.queryf(
            "insert into chatroom_chatlog (rid,uid,content,time_prc) values (%d,%d,%s,NOW())",
            sroom, u.uid.c_str(), it->second.c_str());
        sqlMutex.unlock();
        return;
    }
    if (post.find("leave") != post.end())
    {
        sqlMutex.lock();
        sql.queryf("delete from chatroom_users where uid=%d", u.uid.c_str());
        sqlMutex.unlock();
        return;
    }

    switch (room)
    {
    case 1:
        vars["title"].assign(u8("中南大学家园网 - 聊天室"));
        vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_chatroom.css\" media=\"all\" />";
        vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_chatroom.js\"></script>";

        out.inc("../html/header.html", &vars);
        out.inc("../html/nav.html", &vars);
        out.inc("../html/p_chatroom_r.html", &vars);
        out.puts("<style>#footer{display:none}</style>");
        out.inc("../html/footer.html", &vars);
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    }
}

void procComet(const char*, SCGIEnv& env, SCGIIn&, SCGIOut& out)
{
    SCookie cookie(env, out);
    User u(cookie, env);

    out.headerContentType("text/html");
    if (!u.loggedin)
        return;

    string lasttime;
    SMySQLRes res;
    sqlMutex.lock();
    if (sql.queryf("select UNIX_TIMESTAMP(lastcomettime_prc) from chatroom_users where uid=%d",
        u.uid.c_str()))
    {
        res = sql.store_res();
        sql.clear_res();
    }
    sqlMutex.unlock();
    auto row = res.fetch_row();
    if (!row)
        return;
    lasttime = row[0];

    auto start = clock();
    bool done = false;
    //while (true)
    //{
        sqlMutex.lock();
        if (lasttime.empty())
            sql.query("select us.sex,u.display_name,l.content,UNIX_TIMESTAMP(time_prc) from chatroom_chatlog l inner join users_stu u on u.id=l.uid inner join si_stu us on us.stuid=u.stuid order by l.time_prc DESC limit 20");
        else
            sql.queryf("select us.sex,u.display_name,l.content,UNIX_TIMESTAMP(time_prc) from chatroom_chatlog l inner join users_stu u on u.id=l.uid inner join si_stu us on us.stuid=u.stuid where UNIX_TIMESTAMP(l.time_prc) > %d limit 20", lasttime.c_str());
        res = sql.store_res();
        sql.clear_res();
        sqlMutex.unlock();
        if (row = res.fetch_row())
        {
            done = true;
            do
            {
#if 1
                out.printf("<li class=\"logitem\"><strong class=\"%s\">%s:</strong>%s</li>",
                    row[0][0] == '1' ? "male" : "female",
                    row[1],
                    htmlescape(row[2]).c_str()
                    );
#else
#if 1
                //puts("<script>parent.callback({a:'msg',male:1,name:'sb.',content:'how are you'});</script>");
                //fflush(stdout);
                cout << "<script>parent.callback({a:'msg',male:1,name:'sb.',content:'how are you'});</script>\r\n" << std::flush;
#elif 0
                cout << "<script>parent.callback({a:'msg',male:" << row[0]
                << ",name:'" << row[1] << "',content:'" << htmlescape(row[2]) << "'});</script>" << std::flush;
#else
                string s = "<script>parent.callback({a:'msg',male:";
                s.append(row[0]).append(",name:'").append(row[1]).append("',content:'").append(htmlescape(row[2])).append("'});</script>");
                cout << s << std::flush;
                fflush(stdout);
#endif
#endif
                lasttime = row[3];
            } while (row = res.fetch_row());
        }
        if (done)
        {
            sqlMutex.lock();
            sql.queryf("update chatroom_users set lastcomettime_prc=FROM_UNIXTIME(%d) where uid=%d", lasttime.c_str(), u.uid.c_str());
            sqlMutex.unlock();
    //        break;
        }
    //    if (clock() - start > 40000)
    //        break;
#ifdef _WIN32
    //    Sleep(1000);
#else
    //    sleep(1);
#endif
    //}
}
}

void Chatroom(SCGIApp* app)
{
    app->insert("/chatroom", proc);
    app->insert("/chatroom/comet", procComet);
}
