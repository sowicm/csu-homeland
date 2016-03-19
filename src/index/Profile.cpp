
#include "Profile.h"
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SVars.h"
#include "user.h"

void Profile::attach(SCGIApp* app) const
{
    app->insert("/profile", proc);
}

void Profile::proc(const char* s, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
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

    vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_profile.css\" media=\"all\" />";
    vars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_profile.js\"></script>";

    out.headerContentType("text/html");
    out.inc("../html/header.html", &vars);
    out.inc("../html/nav.html", &vars);
    out.inc("../html/p_profile.html", &vars);
    out.inc("../html/footer.html", &vars);
}
