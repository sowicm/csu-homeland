
#include "Default.h"
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SVars.h"
#include "SCookie.h"
#include "user.h"

namespace{
void proc(const char*, SCGIEnv& env, SCGIIn&, SCGIOut& out)
{
    SVars   vars;
    SCookie cookie(env, out);
    User    u(cookie, env);
    if (u.loggedin)
    {
        out.puts("Location: home\r\n\r\n");
        return;
    }

    out.headerContentType("text/html");

    vars["title"] = u8("中南大学家园网");
    vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_index.css\" media=\"all\" />";
    vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_index.js\"></script>";

    out.inc("../html/header.html", &vars);
    out.inc("../html/p_index.html", &vars);
    out.inc("../html/footer.html", &vars);

    /*
        puts(u(
            "<script type=\"text/javascript\">"
            "$(function(){"
            "createWnd(800,550,'测试浏览器的多啦A梦 - Designed by shop_dd', '<iframe scrolling=\"no\" src=\"/testbrowser\" width=\"800px\" height=\"520px\"></iframe><button id=\"notagain\">我已经用了新浏览器，不要再出现了</button>');"
            "$('.wnd').hide();"
            "$('.wnd').delay(4500).fadeIn(500);"
            "$('#notagain').click(function(){"));
        if (strncmp(cgiUserAgent, "Mozilla/5", sizeof("Mozilla/5") - 1) == 0)
        {
            fputs("$.get('/nomoretest').success(function(){$('.wnd').hide();});", cgiOut);
        }
        else
        {
            fputs(u("alert(\"休想骗我！\");"), cgiOut);
        }
        fputs(
            "});"
            "});"
            "</script>", cgiOut);
    */
}
}

void Default(SCGIApp* app)
{
    app->insert("/", proc);
}

