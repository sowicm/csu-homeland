
#include "Photo.h"
#include "stdio.h"
#ifdef _WIN32
#include "direct.h"             // mkdir
#endif
#include <regex>
#include <string>
#include "rand_ts.h"
#include "SCGIApp.h"
#include "SCGIEnv.h"
#include "SCGIOut.h"
#include "SCookie.h"
#include "SVars.h"
#include "SPost.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "user.h"
using std::string;

void Photo::attach(SCGIApp* app) const
{
    app->match("/photo/([a-zA-Z0-9]+)", procShow);
    app->insert("/uploadphoto", procUpload);
    app->insert("/choosephoto", procChoose);
}

void Photo::procShow(cmatch& mr, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    u.gets();
    Student agent;
    if (!u.loggedin || u.step != S_STEP)
    {
        out.headerContentType("image/jpeg");
        return;
    }

    agent.gets(mr[1].first);
    if (!agent.uid.empty())
    {
        if (!agent.avatar_path.empty())
        {
            string path = "../../csujy_data/photos/";
            path.append(agent.uid).append("/").append(agent.avatar_path);

            FILE *fp = fopen(path.c_str(), "rb");
            if (!fp)
            {
                out.headerContentType("image/jpeg");
                return;
            }
            unsigned char header[8];
            fread(header, 1, 8, fp);
            fclose(fp);

            const unsigned char sig_gif[3] = {'G', 'I', 'F'};
            const unsigned char sig_bmp[2] = {'B', 'M'};
            const unsigned char sig_jpg[3] = {0xff, 0xd8, 0xff};
            const unsigned char sig_png[8] = {0x89, 0x50, 0x4e, 0x47,
                0x0d, 0x0a, 0x1a, 0x0a};

            if (!memcmp(header, sig_gif, 3))
                out.headerContentType("image/gif");
            else if (!memcmp(header, sig_jpg, 3))
                out.headerContentType("image/jpeg");
            else if (!memcmp(header, sig_png, 3))
            {
                if (!memcmp(header, sig_png, 8))
                    out.headerContentType("image/png");
                else
                    goto maybeattack;
            }
            else if (!memcmp(header, sig_bmp, 2))
                out.headerContentType("image/bmp");
            else
                goto maybeattack;

            out.inc(path.c_str());
            return;
maybeattack:
            out.headerContentType("image/jpeg");
            out.inc("../data/pig.jpg");
        }
        else if (!agent.stuid.empty())
        {
            string fpath = "../../csujy_data/si_photos/";
            fpath.append(agent.stuid).append(".jpg");
            out.headerContentType("image/jpeg");
            out.inc(fpath.c_str());
        }
    }
    else
    {
        if (u.type >= 100)
        {
            string fpath = "../../csujy_data/si_photos/";
            fpath.append(agent.stuid).append(".jpg");
            out.headerContentType("image/jpeg");
            out.inc(fpath.c_str());
        }
        else
        {
            out.headerContentType("image/jpeg");
            out.inc("../data/protected.jpg");
        }
    }
}

void Photo::procUpload(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
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
    vars["title"].assign(u8("CSU家园网 - 上传照片"));
    vars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_uploadfile.css\" media=\"all\" />";

    SPost post(in, env);
    int got;
    auto photo = post.file["photo"];
    if (photo == nullptr)
        vars["info"] = u8("上传文件失败！");
    else if (photo->filesize > 2 * 1024 * 1024)
        vars["info"] = u8("请勿上传大于2M的文件！");
    else
    {
        debuglog("open tfile : %s ...", photo->tfilename.c_str());
        FILE *fr = fopen(photo->tfilename.c_str(), "rb");
        if (!fr) debuglog("failed\n"),
            vars["info"] = u8("上传文件失败！");
        else
        {
            debuglog("success\n");
            time_t t;
            t = time(0);
            tm* t2 = localtime(&t);
            string path = "../../csujy_data/photos/";
            char d[9];
            sprintf(d, "%04d%02d%02d", t2->tm_year + 1900, t2->tm_mon + 1, t2->tm_mday);
#ifdef _WIN32
            // not necessary
            // if (_mkdir(path.c_str()) != 0)
            //     debuglog("mkdir %s failed\n", path.c_str());
#else
            mkdir(path.c_str(), S_IRWXU);
#endif
            path.append(u.uid);
#ifdef _WIN32
            // if (_mkdir(path.c_str()) != 0)
            //     debuglog("mkdir %s failed\n", path.c_str());
#else
            mkdir(path.c_str(), S_IRWXU);
#endif
            path.append("/").append(d);
#ifdef _WIN32
            // if (_mkdir(path.c_str()) != 0)
            //     debuglog("mkdir %s failed\n", path.c_str());
#else
            mkdir(path.c_str(), S_IRWXU);
#endif
            string path2;
            char str[13];
            str[12] = 0;

            char uints[26 + 10];
            int un = 0;
            for (char a = 'a'; a <= 'z'; ++a)
                uints[un++] = a;
            for (char a = '0'; a <= '9'; ++a)
                uints[un++] = a;
            while (true)
            {
                str[0] = uints[rand_ts() % 26];
                for (int i = 1; i < 12; ++i)
                {
                    str[i] = uints[rand_ts() % un];
                }
                path2 = path;
                path2.append("/").append(str).append(".jpg");
#ifdef _WIN32
                break;
#else
                if (access(path2.c_str(), F_OK) != 0)
                    break;
#endif
            }

            debuglog("save to %s\n", path2.c_str());
            FILE *fw = fopen(path2.c_str(), "wb");
            if (fw)
            {
                char buffer[512 * 1024];
                while ((got = fread(buffer, 1, sizeof(buffer), fr)) > 0)
                {
                    fwrite(buffer, 1, got, fw);
                }
                fclose(fw);
                path.assign(d).append("/").append(str).append(".jpg");
                sqlMutex.lock();
                sql.queryf("call stu_modify_info(%d,'avatar_path',%s,%s,%s)",
                    u.uid.c_str(), path.c_str(), env.RemoteAddr(), env.UserAgent());
                sql.clear_res();
                sqlMutex.unlock();
                vars["info"] = u8("上传头像成功!");
            }
            else
            {
                vars["info"] = u8("上传头像失败!");
            }
            fclose(fr);
        }
    }

    out.headerContentType("text/html");
    out.inc("../html/header.html", &vars);
    out.inc("../html/nav.html", &vars);
    out.inc("../html/p_uploadphoto.html", &vars);
    out.inc("../html/footer.html", &vars);
}

void Photo::procChoose(const char*, SCGIEnv& env, SCGIIn& in, SCGIOut& out)
{
    SCookie cookie(env, out);
    User    u(cookie, env);
    out.puts("\r\n");
    if (!u.loggedin)
    {
        out.printf("Location: http://%s\r\n\r\n", env.Host());
        return;
    }
    if (u.get_int("step") != S_STEP)
    {
        out.printf("Location: home\r\n\r\n");
        return;
    }

    sqlMutex.lock();
    sql.queryf("call stu_modify_info(%d,'avatar_path','',%s,%s)",
        u.uid.c_str(), env.RemoteAddr(), env.UserAgent());
    sql.clear_res();
    sqlMutex.unlock();
}
