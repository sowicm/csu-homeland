
#include "Others.h"
#include <string>
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SCookie.h"
#include "SVars.h"
#include "mail.h"
#include "user.h"
using std::string;

void Others::attach(SCGIApp* app) const
{
    app->insert("/resendall", procResendAll);
    app->insert("/agreement", procAgreement);
    app->insert("/reagree", procReagree);
    app->insert("/askfor", procAskFor);
    app->insert("/checkfor", procCheckFor);
    app->insert("/shutdown_sowicm", procShutdown);
}

void Others::procResendAll(const char*, SCGIEnv&, SCGIIn& in, SCGIOut& out)
{
    SMySQLRes res;
    sqlMutex.lock();
    if (sql.query("select u.did,u.display_name,v.code,u.email from email_verify_codes v inner join users_stu u on u.id=v.uid limit 100"))
    {
        res = sql.store_res();
        sql.clear_res();
    }
    sqlMutex.unlock();
    MYSQL_ROW row;
    char did[17];
    did[16] = 0;
    string message;
    while (row = res.fetch_row())
    {
        message = "<html><meta http-equiv=\"content-type\"content=\"text/html; charset=UTF-8\"/><body><table><tr><td></td><td style=\"font-size:14px;\">";
        memcpy(did, row[0], 16);
        message += row[1];
        message += "，你好！</td></tr><tr><td></td></tr><tr><td></td></tr><tr><td></td><td style=\"font-size:14px;\">点击以下链接完成邮箱验证并激活在CSU家园网的帐号：</td></tr><tr><td></td></tr><tr><td></td><td><a href=\"http://csujy.com/verify?";
        message += did;
        message += '&';
        message += row[2];
        message += "\">http://csujy.com/verify?";
        message += did;
        message += '&';
        message += row[2];
        message += "</a></td></tr><tr><td></td></tr><tr><td></td><td style=\"font-size:14px;\">如无法点击，请将链接拷贝到浏览器地址栏中直接访问。</td></tr><tr><td></td></tr><tr><td></td><td>---------------------------------------------------------</td></tr><tr><td></td></tr><tr><td></td><td style=\"border-bottom:1px solid #e0e0e0; font-size:12px; line-height:2px;\"height=\"2\">&nbsp;</td></tr><tr><td></td><td style=\"color:#999999;font-size:12px;\">本邮件包含登录信息,<span style=\"font-weight:bold;\">请勿转发他人</span>。系统自动发出,<span style=\"font-weight:bold;\">请勿直接回复</span>。<br/>如有疑问或建议,可发送邮件至<a href=\"mailto:csujy@csujy.com\">csujy@csujy.com</a>。<br/></td></tr></table></body></html>";
        mail("CSU Home Net <noreply@csujy.com>", row[3], "Welcome to CSU Home net, please verify your email", message.c_str());
    }
    out.puts("\r\n");
}

void Others::procAgreement(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    out.headerContentType("text/html");
    SVars vars;
    vars["title"].assign(u8("CSU家园网 - 服务条款"));
    vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_agreement.css\" media=\"all\" />";

    out.inc("../html/header.html", &vars);
    out.inc("../html/p_agreement.html", &vars);
    out.inc("../html/footer.html", &vars);
}

void Others::procReagree(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    out.puts("\r\n");
    if (!u.loggedin)
    {
        out.printf("Location: http://%s\r\n\r\n", env.Host());
        return;
    }
    sqlMutex.lock();
    sql.queryf("update users_stu set agreed=1 where id=%d", u.uid.c_str());
    sqlMutex.unlock();
}

void Others::procAskFor(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    out.headerContentType("text/plain");
    if (!u.loggedin)
    {
        out.putchar('0');
        return;
    }
    if (*s++ == '?')
    {
        if (streq(s, "Conan"))
        {
            SMySQLRes res;
            sqlMutex.lock();
            if (sql.query("select count(*) from Qualification_Conan"))
            {
                res = sql.store_res();
                sql.clear_res();
            }
            sqlMutex.unlock();
            if (!res)
                return;
            auto row = res.fetch_row();
            int cnt;
            sscanf(row[0], "%d", &cnt);
            if (cnt >= 100)
                out.putchar('2');
            else
            {
                sqlMutex.lock();
                if (sql.queryf("replace into Qualification_Conan (uid,time_prc,ip) values(%d,NOW(),%s)", u.uid.c_str(), env.RemoteAddr()))
                    out.putchar('1');
                sqlMutex.unlock();
            }
        }
    }
}

void Others::procCheckFor(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    out.headerContentType("text/plain");
    if (!u.loggedin)
    {
        out.putchar('0');
        return;
    }
    if (*s++ == '?')
    {
        if (streq(s, "Conan"))
        {
            SMySQLRes res;
            sqlMutex.lock();
            if (sql.queryf("select uid from Qualification_Conan where uid=%d", u.uid.c_str()))
            {
                res = sql.store_res();
                sql.clear_res();
            }
            sqlMutex.unlock();
            auto row = res.fetch_row();
            out.putchar(row && row[0] && row[0][0] ? '1' : '0');
        }
    }
}

void Others::procShutdown(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    out.headerContentType("text/plain");
    char* p = nullptr;
    *p = 0;
}