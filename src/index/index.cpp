
#include <stdlib.h>                             // srand
#include <time.h>                               // time
#ifdef _WIN32
#include <process.h>                            // getpid
#else
#include <unistd.h>                             // getpid
#endif
#include <mutex>
#include <thread>
#include "fcgiapp.h"
#include "algorithm/md5.h"
#include "dbconfig.h"
#include "SCGIApp.h"
#include "SCGIIn.h"
#include "SCGIOut.h"
#include "SCGIEnv.h"
#include "SMySQL.h"
#include "Chatroom.h"
#include "Default.h"
#include "Feedback.h"
#include "Guestbook.h"
#include "Home.h"
#include "Login.h"
#include "News.h"
#include "Others.h"
#include "Photo.h"
#include "Profile.h"
#include "Rating.h"
#include "Search.h"
#include "TestBrowser.h"
#include "UPage.h"
#include "Verify.h"
#if defined(_DEBUG) && defined(_WIN32)
#include "Windows.h"
#endif
using std::mutex;
using std::thread;
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libmysql.lib")

#define THREAD_COUNT 1 //20

static void initRandSeed()
{
    unsigned int randint1;
    unsigned int randint2 = (unsigned int)&randint2;
    srand((unsigned)time(0) + randint1 + randint2 + getpid());
}

static void doit()
{
    debuglog("doit...\n");
    FCGX_Request request;
    int          rc;
    FCGX_InitRequest(&request, 0, 0);
    //static mutex acceptMutex;

    SCGIEnv env;
    SCGIIn  in;
    SCGIOut out;

    sql.thread_init();

    while (true)
    {
        debuglog("accept...\n");
        /* Some platforms require accept() serialization, some don't.. */
        //acceptMutex.lock();
        rc = FCGX_Accept_r(&request);
        //acceptMutex.unlock();
        debuglog("accepted...\n");
        if (rc < 0)
        {
            errorlog("accept failed(%d)\n", rc);
            FCGX_Finish_r(&request);
            break;
        }
        debuglog("handle...\n");
        in.set(request.in);
        out.set(request.out);
        env.set(request.envp);
        debuglog("request initialized\n");

#if 0
        strcpy(query, "http://");
        strcat(query, env.Host());
        if (env.Referer() && strncmp(query, env._Referer, strlen(query)) != 0)
        {
            out.headerContentType("text/plain");
            out.puts(env._Referer);
            continue;
        }
#endif

        if (env.UserAgent()[0] == 0)
        {
            debuglog("user agent is missing\n");
            out.headerContentType("text/plain");
            out.puts("Who are you?");
            continue;
        }

        debuglog("cgi proc...\n");
        if (!cgi.proc(env, in, out))
        {
            errorlog("Missed RequestUri:%s\n", env.RequestUri());

            // 没有http:// 则为"302"
            out.printf("Location: http://%s\r\n\r\n", env.Host());
        }
    }
    sql.thread_end();
}

static bool selfCheck()
{
    if (!sql.thread_safe())
    {
        errorlog("SQL is not thread safe\n");
        return false;
    }
    if (md5("") != "D41D8CD98F00B204E9800998ECF8427E" ||
        md5("The quick brown fox jumps over the lazy dog") != "9E107D9D372BB6826BD81D3542A419D6")
    {
        errorlog("md5 wrong: %s, %s\n", md5("").c_str(), md5("The quick brown fox jumps over the lazy dog").c_str());
        return false;
    }
    return true;
}

static void onExit()
{
    FCGX_Request request;
    FCGX_InitRequest(&request, 0, 0);
    errorlog("FCGX_Accept error(%d)\n", FCGX_Accept_r(&request));
    FCGX_Finish_r(&request);
    errorlog("shutdown...\n");
}

int main()
{
    debuglog("start...\n");
    initRandSeed();

    cgi.set(new Default);
    cgi.set(new Home);
    cgi.set(new Guestbook);
    cgi.set(new Photo);
    cgi.set(new UPage);
    cgi.set(new Search);
    cgi.set(new Profile);
    cgi.set(new Rating);
    cgi.set(new Chatroom);
    cgi.set(new Login);
    cgi.set(new News);
    cgi.set(new Feedback);
    cgi.set(new Verify);
    cgi.set(new TestBrowser);
    cgi.set(new Others);

    if (!sql.connect(dbserver, dbuser, dbpwd, dbname, 3306))
    {
        errorlog("MySQL connect faild: %s\n", sql.error());
        return -1;
    }

    if (!selfCheck())
        return -1;

    debuglog("fcgi init...\n");
    int n = FCGX_Init();
    if (n != 0)
    {
        errorlog("fcgi init failed(%d)\n", n);
        return -1;
    }
    for (int i = 1; i < THREAD_COUNT; ++i)
        thread(doit).detach();
    doit();

    onExit();
	return 0;
}
