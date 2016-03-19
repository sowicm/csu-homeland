
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SVars.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "user.h"

namespace {
void proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    if (*s++ == '?')
    {
        if (*s == 'p')
        {
            SCookie cookie(env, out);
            User    u(cookie, env);
            out.headerContentType("text/html");
            if (!u.loggedin || u.get_int("step") != S_STEP)
                return;
            int p;
            if (sscanf(s, "p=%d", &p) < 1)
                return;
            SMySQLRes res;
            sqlMutex.lock();
            if (sql.query("select count(*) from says"))
            {
                res = sql.store_res();
                sql.clear_res();
            }
            sqlMutex.unlock();
            if (!res)
                return;
            auto row = res.fetch_row();
            int numSays;
            sscanf(row[0], "%d", &numSays);
            out.puts("<div class=\"says\"><ol>");
            if (numSays != 0)
            {
                char start[64];
                sprintf(start, "%d", (p - 1) * 20);
                int numPages = ((numSays - 1) / 20) + 1;
                if (p > 0 && p <= numPages)
                {
                    SVars vars;
                    sqlMutex.lock();
                    if (sql.queryf("select * from v_says order by says_id DESC limit %d,20", start))
                    {
                        res = sql.store_res();
                        sql.clear_res();
                    }
                    sqlMutex.unlock();
                    while (row = res.fetch_row())
                    {
                        vars["tid"] = row[0];
                        vars["did"] = row[1];
                        vars["name"] = row[2];
                        vars["content"] = move( htmlescape(row[3]) );
                        vars["time"] = row[4];
                        vars["numre"] = row[5];
                        out.inc("../html/li_say.html", &vars);
                    }

                    if (numPages > 1)
                    {
                    /*
                            <div class="pagebar">
                            <span>上一页</span>
                            <span>1</span>
                            <span class="pagenumber">2</span>
                            <span class="pagenumber">3</span>
                            <span>...</span>
                            <span class="pagenumber">8</span>
                            <span class="nextpage">下一页</span>
                            <span>到</span>
                            <input />
                            <span>页</span>
                            <button class="button">确定</button>
                            </div>
                    */
                            out.puts("<div class=\"pagebar\">");
                        if (p > 1)
                            out.puts( u8("<span class=\"prevpage\">上一页</span>") );
                        int i;
                        for (i = 1; i < p; ++i)
                            out.printf("<span class=\"pagenumber\">%d</span>", i);
                        out.printf("<span class=\"curpage\">%d</span>", i);
                        for (++i; i <= numPages; ++i)
                            out.printf("<span class=\"pagenumber\">%d</span>", i);
                        if (p < numPages)
                            out.puts( u8("<span class=\"nextpage\">下一页</span>") );
                        out.puts( u8("<span>到</span><input /><span>页</span><button>确定</button></div>") );
                    }
                }
            }
            out.puts("</ol></div><script type=\"text/javascript\">onSaysLoad();</script>");
        }
        else if (*s == 't')
        {
            SCookie cookie(env, out);
            User    u(cookie, env);
            out.headerContentType("text/html");
            if (!u.loggedin || u.get_int("step") != S_STEP)
                return;
            int t, p;
            if (sscanf(s, "t=%d", &t) < 1)
                return;
            s += 2;
            char *pcur = (char*)s;
            while (*pcur && *pcur != '&')
                ++pcur;
            if (*pcur != '&')
                return;
            *pcur = '\0';
            if (*++pcur != 'p')
                return;
            if (*++pcur != '=')
                return;
            ++pcur;
            if (sscanf(pcur, "%d", &p) < 1)
                return;
            SMySQLRes res;
            sqlMutex.lock();
            if (sql.queryf("select replies_count from says where says_id=%d", s))
            {
                res = sql.store_res();
                sql.clear_res();
            }
            sqlMutex.unlock();
            if (!res)
                return;
            auto row = res.fetch_row();
            int numReplies;
            sscanf(row[0], "%d", &numReplies);
            out.puts("<ol>");
            if (numReplies != 0)
            {
                char start[64];
                sprintf(start, "%d", (p - 1) * 20);
                int numPages = ((numReplies - 1) / 20) + 1;
                if (p > 0 && p <= numPages)
                {
                    SVars vars;
                    sqlMutex.lock();
                    if (sql.queryf("select * from v_replies where says_id=%d limit %d,20", s, start))
                    {
                        res = sql.store_res();
                        sql.clear_res();
                    }
                    sqlMutex.unlock();
                    while (row = res.fetch_row())
                    {
                        vars["did"] = row[1];
                        vars["name"] = row[2];
                        vars["content"] = move( htmlescape(row[3]) );
                        vars["time"] = row[4];
                        out.inc("../html/li_say_reply.html", &vars);
                    }
                }
            }
            out.puts("</ol><script type=\"text/javascript\">onRepliesLoad();</script>");
        }
    }
}
}

void Guestbook(SCGIApp* app)
{
    app->insert("/guestbook", proc);
}
