
#include "UPage.h"
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIIn.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SVars.h"
#include "user.h"

void UPage::attach(SCGIApp* app) const
{
    app->match("/u/([a-zA-Z0-9]+)", proc);
}

void UPage::proc(cmatch& mr, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    if (!u.loggedin)
    {
        out.printf("Location: http://%s\r\n\r\n", env.Host());
        return;
    }
    out.headerContentType("text/html");
    if (u.get_int("step") != S_STEP)
        return;

    const char* did = mr[1].first;
    SMySQLRes res;
    sqlMutex.lock();
    sql.queryf("call visit(%s,%d,%s,%s)", did, u.uid.c_str(), env.RemoteAddr(), env.UserAgent());
    res = sql.store_res();
    sql.clear_res();
    sqlMutex.unlock();
    auto row = res.fetch_row();
    if (row)
    {
        SVars vars;
        vars["title"].assign(u8("中南大学家园网 - ")).append(row[0]);
        vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_u.css\" media=\"all\" />";
        vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_u.js\"></script>";

        vars["did"] = did;
        vars["name"] = row[0];
        vars["sex"] =  (row[1][0] == '1' ? u8("男") : u8("女"));

        auto tt = time(0);
        auto lt = localtime(&tt);
        int cyear = lt->tm_year + 1900;
        char age[32];
        int byear = (row[2][0] - '0') * 1000 + (row[2][1] - '0') * 100 + (row[2][2] - '0') * 10 + (row[2][3] - '0');
        sprintf(age, "%d", cyear - byear);
        vars["age"] = age;
        vars["native"] = row[3];
        vars["nation"] = row[4];
        vars["address"] = row[5];
        vars["zone"] = row[6];
        char *pcur = row[7];
        char*& academy = row[7];
        while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
            ++pcur;

        if (!*pcur)
            return;
        char *grade = pcur;
        while (*pcur && (*pcur >= '0' && *pcur <= '9'))
            ++pcur;
        if (!*pcur)
            return;
        *pcur = 0;
        vars["grade"] = grade;
        grade[0] = 0;
        vars["academy"] = academy;
        ++pcur;
        if (!*pcur)
            return;
        ++pcur;
        if (!*pcur)
            return;
        ++pcur;
        if (!*pcur)
            return;
        char *pro = pcur;
        while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
            ++pcur;
        if (!*pcur)
            return;
        *pcur = 0;
        vars["pro"] = pro;
        ++pcur;
        if (!*pcur)
            return;
        ++pcur;
        if (!*pcur)
            return;
        vars["class"] = pcur;

        int cnt;
        sscanf(row[9], "%d", &cnt);
        vars["scorep"] = row[9];
        if (cnt == 0)
            vars["score"] = "0";
        else
        {
            int sum;
            sscanf(row[8], "%d", &sum);
            double score = sum;
            score /= cnt;
            char sscore[16];
            if (score > 10.0)
                score = 10.0;
            sprintf(sscore, "%.1lf", score);
            vars["score"] = sscore;
        }

        out.inc("../html/header.html", &vars);
        out.inc("../html/nav.html", &vars);
        out.inc("../html/p_u.html", &vars);
        out.inc("../html/footer.html", &vars);
    }
    res.free();
}
