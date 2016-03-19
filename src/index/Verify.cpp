
#include "Verify.h"
#include <string>
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIIn.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SPost.h"
#include "mail.h"
#include "user.h"
using std::string;

void Verify::attach(SCGIApp* app) const
{
    app->insert("/verify", proc);
    app->insert("/sendverifymail", procSend);
    app->insert("/skipverify", procSkip);
}

void Verify::proc(const char* s, SCGIEnv& env, SCGIIn&, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    SMySQLRes res;
    if (*s != '?' || strlen(s) < 3)
        goto error;
    char* p = (char*) ++s;
    while (*p && *p != '&')
        ++p;
    if (*p != '&')
        goto error;
    *p++ = '\0';
    if (strlen(p) < 2)
        goto error;

    sqlMutex.lock();
    if (sql.queryf("call verifyEmail(%s,%s)", s, p))
    {
        res = sql.store_res();
        sql.clear_res();
    }
    sqlMutex.unlock();
    if (res)
    {
        auto row = res.fetch_row();
        if (row == nullptr || row[0] == nullptr)
            goto error;
        //u.uid = row[0];
        //u.save();
        //fprintf(cgiOut, "Location: home\r\n\r\n");
        //return;
    }
error:
    if (u.loggedin)
        u.destroy();
    out.puts("Location: .\r\n\r\n");
}

void Verify::procSend(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SPost post(in, env);
    const char* email = post["email"];
    if (email != SPost::null)
    {
        SCookie cookie(env, out);
        User    u(cookie, env);
        if (u.loggedin)
        {
            char emailcode[33];
            u.generateEmailCode(emailcode);
            SMySQLRes res;
            sqlMutex.lock();
            if (sql.queryf("call changeEmail(%d,%s,%s,%s,%s)", u.uid.c_str(), email, emailcode, env.RemoteAddr(), env.UserAgent()))
            {
                sql.clear_res();
                sql.queryf("select did,display_name from users_stu where id=%d", u.uid.c_str());
                res = sql.store_res();
            }
            sqlMutex.unlock();
            auto row = res.fetch_row();
            if (row)
            {
                //char did[17];
                //did[16] = 0;
                string message = "<html><meta http-equiv=\"content-type\"content=\"text/html; charset=UTF-8\"/><body><table><tr><td></td><td style=\"font-size:14px;\">";
                //memcpy(did, row[0], 16);
                message += row[1];
                message += "，你好！</td></tr><tr><td></td></tr><tr><td></td></tr><tr><td></td><td style=\"font-size:14px;\">点击以下链接完成邮箱验证并激活在CSU家园网的帐号：</td></tr><tr><td></td></tr><tr><td></td><td><a href=\"http://csujy.com/verify?";
                message += row[0];
                message += '&';
                message += emailcode;
                message += "\">http://csujy.com/verify?";
                message += row[0];
                message += '&';
                message += emailcode;
                message += "</a></td></tr><tr><td></td></tr><tr><td></td><td style=\"font-size:14px;\">如无法点击，请将链接拷贝到浏览器地址栏中直接访问。</td></tr><tr><td></td></tr><tr><td></td><td>---------------------------------------------------------</td></tr><tr><td></td></tr><tr><td></td><td style=\"border-bottom:1px solid #e0e0e0; font-size:12px; line-height:2px;\"height=\"2\">&nbsp;</td></tr><tr><td></td><td style=\"color:#999999;font-size:12px;\">本邮件包含登录信息,<span style=\"font-weight:bold;\">请勿转发他人</span>。系统自动发出,<span style=\"font-weight:bold;\">请勿直接回复</span>。<br/>如有疑问或建议,可发送邮件至<a href=\"mailto:csujy@csujy.com\">csujy@csujy.com</a>。<br/></td></tr></table></body></html>";
                mail("CSU Home Net <noreply@csujy.com>", email, "Welcome to CSU Home net, please verify your email", message.c_str());
            }
        }
    }
    out.puts("\r\n");
}

void Verify::procSkip(const char*, SCGIEnv& env, SCGIIn&, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    if (u.loggedin && u.get_int("step") == 1)
    {
        sqlMutex.lock();
        sql.queryf("update users_stu set step=2 where id=%d and email is not null and email <> ''", u.uid.c_str());
        sqlMutex.unlock();
    }
    out.puts("Location: home\r\n\r\n");
}
