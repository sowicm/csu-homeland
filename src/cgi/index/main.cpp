#include "../common/cgic.h"
#include "../common/scgi.h"
#include "../common/md5.h"
#include "../common/dbconfig.h"

#ifdef _WIN32
#include <Winsock2.h>
#endif
#include <stdio.h>
#include <iostream>
#ifdef _WIN32
#include <mysql.h>
#pragma comment(lib, "libmysql.lib")
#else
#include <mysql/mysql.h>
#endif
#include <time.h>
#include <stdarg.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

using std::cout;
using std::endl;
using std::string;
using std::map;

MYSQL mysql;

#define die(x) cgiHeaderContentType("text/plain"),puts(x),exit(0);

#ifdef _WIN32
#define B(x) MessageBox(0, x, "B", 0)
#define u(x) utf8(x)
#else
#define B(x) 0
#define u(x) x
#endif

inline void mysql_init()
{    
	mysql_init(&mysql);

    /*
	FILE *fp = fopen("../data/safe/dbconfig", "rb");
    if (!fp)
        die("dbconfig");
	if (fscanf(fp, "dbserver:%s\n", &dbserver) < 1)
		die("file:dbconfig error");
	if (fscanf(fp, "dbuser:%s\n", &dbuser) < 1)
		die("file:dbconfig error");
	if (fscanf(fp, "dbpwd:%s\n", &dbpwd) < 1)
		die("file:dbconfig error");
	if (fscanf(fp, "dbname:%s\n", &dbname) < 1)
		die("file:dbconfig error");
    fclose(fp);
    */
}
inline void mysql_connect()
{
    if (!mysql_real_connect(&mysql, dbserver, dbuser, dbpwd, dbname, 3306, nullptr, CLIENT_MULTI_RESULTS))
	{
        cgiHeaderContentType("text/plain");
		cout << "failed to connect db. errno: " << mysql_errno(&mysql) << " (" << mysql_error(&mysql) << ")" << endl;
		exit(0);
	}
    mysql_set_character_set(&mysql, "utf8");
    mysql_query(&mysql, "set time_zone = '+8:00'");
}
inline void mysql_close()
{	
	mysql_close(&mysql);
}
int mysql_queryf(char* buffer, const char* format, ...)
{
    const char *pstart, *pend;
    va_list ap;
    va_start(ap, format);
    pend = pstart = format;
    char* pcur = buffer;
    do
    {
        ++pend;
        while (*pend && *pend != '%')
            ++pend;
        memcpy(pcur, pstart, pend - pstart);
        pcur += pend - pstart;
        if (!*pend)
            break;        
        const char* p = va_arg(ap, const char*);
        switch(*++pend)
        {
        case 'd':
            pcur += mysql_real_escape_string(&mysql, pcur, p, strlen(p));
            break;
        case 's':
            *pcur++ = '\'';
            pcur += mysql_real_escape_string(&mysql, pcur, p, strlen(p));
            *pcur++ = '\'';
            break;
        case 'p':
            {
                ++pend;
                auto entry = cgiFormEntryFindFirst(p);
                if (!entry)
                {
                    *pcur++ = 'n';
                    *pcur++ = 'u';
                    *pcur++ = 'l';
                    *pcur++ = 'l';
                }
                else
                {
                    switch(*pend)
                    {
                    case 'd':
                        pcur += mysql_real_escape_string(&mysql, pcur, entry->value, entry->valueLength);
                        break;
                    case 's':
                        *pcur++ = '\'';
                        pcur += mysql_real_escape_string(&mysql, pcur, entry->value, entry->valueLength);
                        *pcur++ = '\'';
                        break;
                    }
                }
                break;
            }
        };
        pstart = ++pend;
        if (!*pstart)
            break;
    } while (true);
#if 0
    *pcur = 0;
    B(buffer);
#endif
    return mysql_real_query(&mysql, buffer, pcur - buffer);
    /*
    int l = strlen(procedure);
    int n = l + 8;// 'call ()'
    for (int i = 0; i < num; ++i)
    {
        p = va_arg(ap, const char*);
        n += strlen(p);
    }
    va_start(ap, num);
    char* query = new char[n];
    if (query == nullptr)
        exit(1);
    */
}
void mysql_clear_result()
{
    MYSQL_RES* res;
    while (mysql_next_result(&mysql) > 0)
    {
        res = mysql_store_result(&mysql);
        mysql_free_result(res);
    }
}

string uid;
char   sid[65];
char   query[512 * 1024];
void gensid()
{
    char *pend;
    int randint1;
    long long randint2 = (long long)&randint2;
    srand(randint2 + (time(0) * randint1));

    char uints[26 + 26 + 10];
    int un = 0;
    for (char a = 'a'; a <= 'z'; ++a)
        uints[un++] = a;
    for (char a = '0'; a <= '9'; ++a)
        uints[un++] = a;
    for (char a = 'A'; a <= 'Z'; ++a)
        uints[un++] = a;

    MYSQL_ROW row;
    const char* front = "select `uid` from `sessions` where `sid`='";
    const int lfront = sizeof("select `uid` from `sessions` where `sid`='") - 1;
    memcpy(query, front, lfront);
    sid[64] = 0;
    do
    {
        for (int i = 0; i < 64; ++i)
        {
            sid[i] = uints[rand() % un];
        }
        pend = query + lfront;
        memcpy(pend, sid, 64);
        pend += 64;
        *pend++ = '\'';
        mysql_real_query(&mysql, query, pend - query);
        auto res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        mysql_free_result(res);
    } while (row);
}
void session_start()
{
    if (cgiFormNotFound == cgiCookieString("csujySessionId", sid, 65))
    {
        gensid();
        fprintf(cgiOut, "Set-Cookie: csujySessionId=%s;\r\n", sid);
    }
    else
    {
        if (mysql_queryf(query, "call session_start(%s, %s)", sid, cgiRemoteAddr) == 0)
        {
            auto res = mysql_store_result(&mysql);
            if (res != nullptr)
            {
                auto row = mysql_fetch_row(res);
                if (row != nullptr)
                {
                    uid = row[0];
                }
            }
            mysql_free_result(res);
            mysql_clear_result();
        }
    }
}
void session_save(bool remember = false)
{
    gensid();
    char expires[1024];
    auto t1 = time(0);
    auto t2 = localtime(&t1);
    ++t2->tm_year;
    strftime(expires, 1024, "%A, %d-%b-%y %H:%M:%S GMT", t2);
    if (remember)
        fprintf(cgiOut, "Set-Cookie: csujySessionId=%s;Expires=%s;\r\n", sid, expires);
    else
        fprintf(cgiOut, "Set-Cookie: csujySessionId=%s;\r\n", sid);
    mysql_queryf(query, "call session_save(%s,%s,%d)", sid, cgiRemoteAddr, uid.c_str());
}
void session_destroy()
{
    if (!sid[0])
        session_start();
    if (!uid.empty())
        mysql_queryf(query, "call session_destroy(%s,%d)", sid, uid.c_str());
}
#define S_STEP 3
inline int GetStep()
{
    int step = 1;
    mysql_queryf(query, "select step from users_stu where id=%d", uid.c_str());
    auto res = mysql_store_result(&mysql);
    auto row = mysql_fetch_row(res);
    if (row)
    {
        if (sscanf(row[0], "%d", &step) < 1)
            step = 1;
    }
    mysql_free_result(res);
    return step;
}
void mail(const char* from, const char* to, const char* subject, const char* html)
{
#ifndef _WIN32
    FILE *fp = popen("/usr/sbin/sendmail -t -i ", "w");
    fprintf(fp,
        "To: %s\n"
        "Subject: %s\n"
        "From: %s\n"
        "MIME-Version: 1.0\n"
        "Content-type: text/html; charset=UTF-8\n"
		"\n%s\n"
        ,
        to, subject, from, html);
    pclose(fp);
#endif
}
void genEmailCode(char* buf)
{
    char *pend;
    int randint1;
    long long randint2 = (long long)&randint2;
    srand(randint2 + (time(0) * randint1));
    char uints[26 + 26 + 10];
    int un = 0;
    for (char a = 'a'; a <= 'z'; ++a)
        uints[un++] = a;
    for (char a = '0'; a <= '9'; ++a)
        uints[un++] = a;
    for (char a = 'A'; a <= 'Z'; ++a)
        uints[un++] = a;
    MYSQL_ROW row;
    const char* front = "select `uid` from `email_verify_codes` where `code`='";
    const int lfront = sizeof("select `uid` from `email_verify_codes` where `code`='") - 1;
    memcpy(query, front, lfront);
    buf[32] = 0;
    do
    {
        for (int i = 0; i < 32; ++i)
        {
            buf[i] = uints[rand() % un];
        }
        pend = query + lfront;
        memcpy(pend, buf, 32);
        pend += 32;
        *pend++ = '\'';
        mysql_real_query(&mysql, query, pend - query);
        auto res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        mysql_free_result(res);
    } while (row);
}
void _log(const char* str, int s = -1)
{
    if (s == -1)
        s = strlen(str);
    FILE *fp = fopen("log.txt", "ab");
    fwrite(str, 1, s, fp);
    fclose(fp);
}

int cgiMain()
{
    strcpy(query, "http://");
    strcat(query, getenv("HTTP_HOST"));
    
    if (cgiReferrer[0] && strncmp(query, cgiReferrer, strlen(query)) != 0)
    {
        cgiHeaderContentType("text/plain");
        B(cgiReferrer);
        return 0;
    }

    // need agent
    if (cgiUserAgent[0] == 0)
    {
        cgiHeaderContentType("text/plain");
        puts("Who are you?");
        return 0;
    }

    sid[0] = 0;
    if (strcmp(cgiRequestUri, "/") == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        mysql_close();
        if (!uid.empty())
        {
            fprintf(cgiOut, "Location: home\r\n\r\n");
            return 0;
        }
        cgiHeaderContentType("text/html");
        
        svars["title"] = u("中南大学家园网");
        svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_index.css\" media=\"all\" />";
        svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_index.js\"></script>";

        inc("../html/header.html");
        inc("../html/p_index.html");
        inc("../html/footer.html");

        cgiCookieString("csujyNoMoreTest", query, 8);
        if (strcmp(query, "true") != 0)
        {
            fputs(
                "<script type=\"text/javascript\">"
                    "$(function(){"
                        "createWnd(800,550,'测试浏览器的多啦A梦 - Designed by shop_dd', '<iframe scrolling=\"no\" src=\"/testbrowser\" width=\"800px\" height=\"520px\"></iframe><button id=\"notagain\">我已经用了新浏览器，不要再出现了</button>');"
                        "$('.wnd').hide();"
                        "$('.wnd').delay(4500).fadeIn(500);"
                        "$('#notagain').click(function(){", cgiOut);
            if (strncmp(cgiUserAgent, "Mozilla/5", sizeof("Mozilla/5") - 1) == 0)
            {
                fputs("$.get('/nomoretest').success(function(){$('.wnd').hide();});", cgiOut);
            }
            else
            {
                fputs("alert(\"休想骗我！\");", cgiOut);
            }
            fputs(
                        "});"
                    "});"
                "</script>", cgiOut);
            
        }
    }
    else if (strcmp(cgiRequestUri, "/home") == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        if (uid.empty())
        {
            mysql_close();
            fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
            return 0;
        }

        int step = GetStep();

        mysql_queryf(query,
            "select display_name from users_stu where users_stu.id=%d",
            uid.c_str());
        auto res = mysql_store_result(&mysql);
        auto row = mysql_fetch_row(res);
        
        svars["name"] = row[0];
        svars["title"].assign(u("中南大学家园网 - ")).append(row[0]);

        mysql_free_result(res);

        cgiHeaderContentType("text/html");
        switch (step)
        {
        case 0:
        case 3:
            bool reagree;
            mysql_queryf(query, "select agreed from users_stu where id=%d limit 1", uid.c_str());
            res = mysql_store_result(&mysql);
            row = mysql_fetch_row(res);
            reagree = !(row[0] && row[0][0] != '0');
            mysql_free_result(res);

            if (reagree)
            {
                svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" />";
                svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_reagree.js\"></script>";
                inc("../html/header.html");
                inc("../html/nav.html");
                inc("../html/p_reagree.html");
                mysql_queryf(query, "select did from users_stu where id=%d", uid.c_str());
                res = mysql_store_result(&mysql);
                row = mysql_fetch_row(res);
                svars["did"] = row[0];
                mysql_free_result(res);
                mysql_queryf(query, "select avatar_path from users_stu where id=%d", uid.c_str());
                res = mysql_store_result(&mysql);
                row = mysql_fetch_row(res);
                if (!row[0])
                {
                    inc("../html/w_uploadphoto.html");
                }
                mysql_free_result(res);
                inc("../html/footer.html");
                break;
            }

            svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" /><link rel=\"stylesheet\" type=\"text/css\" href=\"../css/says.css\" media=\"all\" />";
            svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_home.js\"></script><script type=\"text/javascript\" src=\"../js/says.js\"></script>";
            
            mysql_queryf(query, "select did from users_stu where id=%d", uid.c_str());
            res = mysql_store_result(&mysql);
            row = mysql_fetch_row(res);
            svars["did"] = row[0];
            mysql_free_result(res);

            mysql_queryf(query, "select sum(r.score),count(r.score) from users_stu u inner join ratings r on r.stuid=u.stuid where u.id=%d", uid.c_str());
            res = mysql_store_result(&mysql);
            row = mysql_fetch_row(res);
            int cnt;
            sscanf(row[1], "%d", &cnt);
            svars["scorep"] = row[1];
            if (cnt == 0)
                svars["score"] = "0";
            else
            {
                int sum;
                sscanf(row[0], "%d", &sum);
                double score = sum;
                score /= cnt;
                sprintf(query, "%.1lf", score);
                svars["score"] = query;
            }
            mysql_free_result(res);

            inc("../html/header.html");
            inc("../html/nav.html");
            inc("../html/p_home.html");

            mysql_queryf(query, "select avatar_path from users_stu where id=%d", uid.c_str());
            res = mysql_store_result(&mysql);
            row = mysql_fetch_row(res);
            if (!row[0])
            {
                inc("../html/w_uploadphoto.html");
            }
            mysql_free_result(res);

            inc("../html/footer.html");
            break;
        case 1:
            svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" />";
            svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_home_step1.js\"></script>";
            inc("../html/header.html");
            inc("../html/nav.html");
            inc("../html/p_home_step1.html");
            inc("../html/footer.html");
            break;
        case 2:
            svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_home.css\" media=\"all\" />";
            svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_home_step2.js\"></script>";
            inc("../html/header.html");
            inc("../html/nav.html");
            inc("../html/p_home_step2.html");
            inc("../html/footer.html");
            break;
        }
        mysql_close();
    }
    else if (strcmp(cgiRequestUri, "/home?say") == 0)
    {
        int l;
        cgiFormStringSpaceNeeded("txt", &l);
        if (l > 1)
        {
            mysql_init();
            mysql_connect();
            session_start();
            if (!uid.empty() && GetStep() == S_STEP)
            {
                mysql_queryf(query, "call user_say(%d,%ps,%s,%s)", uid.c_str(), "txt", cgiRemoteAddr, cgiUserAgent);
            }
            mysql_close();
        }
        fprintf(cgiOut, "\r\n");
    }
    else if (strcmp(cgiRequestUri, "/home?reply") == 0)
    {
        int l1, l2;
        cgiFormStringSpaceNeeded("tid", &l1);
        cgiFormStringSpaceNeeded("txt", &l2);
        if (l1 > 1 && l2 > 1)
        {
            mysql_init();
            mysql_connect();
            session_start();
            if (!uid.empty() && GetStep() == S_STEP)
            {
                mysql_queryf(query, "call user_reply(%pd,%d,%ps,%s,%s)", "tid", uid.c_str(), "txt", cgiRemoteAddr, cgiUserAgent);
            }
            mysql_close();
        }
        fprintf(cgiOut, "\r\n");
    }
    else if (strncmp(cgiRequestUri, "/guestbook?p=", 13) == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        cgiHeaderContentType("text/html");
        if (!uid.empty() && GetStep() == S_STEP)
        {
            int p;
            sscanf(cgiRequestUri + 13, "%d", &p);
            mysql_query(&mysql, "select count(*) from says");
            auto res = mysql_store_result(&mysql);
            auto row = mysql_fetch_row(res);
            int numSays;
            sscanf(row[0], "%d", &numSays);
            mysql_free_result(res);
            fputs("<div class=\"says\"><ol>", cgiOut);
            if (numSays != 0)
            {
                char start[64];
                sprintf(start, "%d", (p - 1) * 20);
                int numPages = ((numSays - 1) / 20) + 1;
                if (p > 0 && p <= numPages)
                {
                    mysql_queryf(query, "select * from v_says order by says_id DESC limit %d,20", start);
                    res = mysql_store_result(&mysql);
                    while (row = mysql_fetch_row(res))
                    {
                        svars["tid"] = row[0];
                        svars["did"] = row[1];
                        svars["name"] = row[2];
                        svars["content"] = row[3];
                        svars["time"] = row[4];
                        svars["numre"] = row[5];
                        inc("../html/li_say.html");
                    }
                    mysql_free_result(res);

                    if (numPages > 1)
                    {
#if 0
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
#endif
                        fputs("<div class=\"pagebar\">", cgiOut);
                        if (p > 1)
                            fputs("<span class=\"prevpage\">上一页</span>", cgiOut);
                        int i;
                        for (i = 1; i < p; ++i)
                            fprintf(cgiOut, "<span class=\"pagenumber\">%d</span>", i);
                        fprintf(cgiOut, "<span class=\"curpage\">%d</span>", i);
                        for (++i; i <= numPages; ++i)
                            fprintf(cgiOut, "<span class=\"pagenumber\">%d</span>", i);
                        if (p < numPages)
                            fputs("<span class=\"nextpage\">下一页</span>", cgiOut);
                        fputs("<span>到</span><input /><span>页</span><button>确定</button></div>", cgiOut);
                    }
                }
            }
            fputs("</ol></div><script type=\"text/javascript\">onSaysLoad();</script>", cgiOut);
        }
        mysql_close();
    }
    else if (strncmp(cgiRequestUri, "/guestbook?t=", 13) == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        cgiHeaderContentType("text/html");
        if (!uid.empty() && GetStep() == S_STEP)
        {
            try
            {
                int t, p;
                char *tid = cgiRequestUri + 13;
                sscanf(tid, "%d", &t);
                char *pcur = tid;
                while (*pcur && *pcur != '&')
                    ++pcur;
                if (*pcur != '&')
                    throw 0;
                *pcur = '\0';
                if (*++pcur != 'p')
                    throw 0;
                if (*++pcur != '=')
                    throw 0;
                ++pcur;
                sscanf(pcur, "%d", &p);
                mysql_queryf(query, "select replies_count from says where says_id=%d", tid);
                auto res = mysql_store_result(&mysql);
                auto row = mysql_fetch_row(res);
                int numReplies;
                sscanf(row[0], "%d", &numReplies);
                mysql_free_result(res);
                fputs("<ol>", cgiOut);
                if (numReplies != 0)
                {
                    char start[64];
                    sprintf(start, "%d", (p - 1) * 20);
                    int numPages = ((numReplies - 1) / 20) + 1;
                    if (p > 0 && p <= numPages)
                    {
                        mysql_queryf(query, "select * from v_replies where says_id=%d limit %d,20", tid, start);
                        res = mysql_store_result(&mysql);
                        while (row = mysql_fetch_row(res))
                        {
                            svars["did"] = row[1];
                            svars["name"] = row[2];
                            svars["content"] = row[3];
                            svars["time"] = row[4];
                            inc("../html/li_say_reply.html");
                        }
                        mysql_free_result(res);
                    }
                }
                fputs("</ol><script type=\"text/javascript\">onRepliesLoad();</script>", cgiOut);
            }
            catch (...)
            {
            }
        }
        mysql_close();
    }
    else if (strncmp(cgiRequestUri, "/photo/", 7) == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        if (!uid.empty() && GetStep() == S_STEP)
        {
            mysql_queryf(query, "select avatar_path, stuid from users_stu where did=%s limit 1", cgiRequestUri + 7);
            auto res = mysql_store_result(&mysql);
            auto row = mysql_fetch_row(res);
            if (row)
            {
                if (row[0] && row[0][0])
                {                
                    string path =
#ifdef _WIN32
                        "E:\\__Sowicm\\_Projects\\_All\\csujy\\data\\photos\\";
#else
                        "../../csujy_data/photos/";
#endif
                    path.append(uid).append("/").append(row[0]);

                    FILE *fp = fopen(path.c_str(), "rb");
                    unsigned char header[8];
                    fread(header, 1, 8, fp);
                    fclose(fp);

                    if (!memcmp(header, sig_gif, 3))
                        cgiHeaderContentType("image/gif");
                    else if (!memcmp(header, sig_jpg, 3))
                        cgiHeaderContentType("image/jpeg");
                    else if (!memcmp(header, sig_png, 3))
                    {
                        if (!memcmp(header, sig_png, 8))
                            cgiHeaderContentType("image/png");
                        else
                            goto maybeattack;
                    }
                    else if (!memcmp(header, sig_bmp, 2))
                        cgiHeaderContentType("image/bmp");
                    else
                        goto maybeattack;

                    inc(path.c_str(), true);
                    return 0;
maybeattack:
                    cgiHeaderContentType("image/jpeg");
                    inc("../data/pig.jpg", true);
                }
                else if (row[1])
                {
                    char fpath[261] = 
#ifdef _WIN32
                        "E:\\__Sowicm\\_Projects\\_All\\csujy\\data\\si_photos\\";
#else
                        "../../csujy_data/si_photos/";
#endif
                    strcat(fpath, row[1]);
                    strcat(fpath, ".jpg");
                    cgiHeaderContentType("image/jpeg");
                    inc(fpath, true);
                }
            }
            else
            {                
                cgiHeaderContentType("image/jpeg");
                inc("../data/protected.jpg", true);
            }
            mysql_free_result(res);
        }
        else
            cgiHeaderContentType("image/jpeg");
        mysql_close();
    }
    else if (strncmp(cgiRequestUri, "/u/", 3) == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        if (uid.empty())
        {
            mysql_close();
            fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
            return 0;
        }
        cgiHeaderContentType("text/html");
        if (GetStep() == S_STEP)
        {
            char *did = cgiRequestUri + 3;
            mysql_queryf(query, "call visit(%s,%d,%s,%s)", did, uid.c_str(), cgiRemoteAddr, cgiUserAgent);
            auto res = mysql_store_result(&mysql);
            auto row = mysql_fetch_row(res);
            if (row)
            {
                svars["title"].assign(u("中南大学家园网 - ")).append(row[0]);
                svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_u.css\" media=\"all\" />";
                svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_u.js\"></script>";

                svars["did"] = did;
                svars["name"] = row[0];
                svars["sex"] =  (row[1][0] == '1' ? u("男") : u("女"));

                auto tt = time(0);
                auto lt = localtime(&tt);
                int cyear = lt->tm_year + 1900;
                int byear = (row[2][0] - '0') * 1000 + (row[2][1] - '0') * 100 + (row[2][2] - '0') * 10 + (row[2][3] - '0');
                sprintf(query, "%d", cyear - byear);
                svars["age"] = query;
                svars["native"] = row[3];
                svars["nation"] = row[4];
                svars["address"] = row[5];
                svars["zone"] = row[6];
                char *pcur = row[7];
                char*& academy = row[7];
                while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
                    ++pcur;
                try
                {
                    if (!*pcur)
                        throw 0;
                    char *grade = pcur;
                    while (*pcur && (*pcur >= '0' && *pcur <= '9'))
                        ++pcur;
                    if (!*pcur)
                        throw 0;
                    *pcur = 0;
                    svars["grade"] = grade;
                    grade[0] = 0;
                    svars["academy"] = academy;
                    ++pcur;
                    if (!*pcur)
                        throw 0;
                    ++pcur;
                    if (!*pcur)
                        throw 0;
                    ++pcur;
                    if (!*pcur)
                        throw 0;
                    char *pro = pcur;
                    while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
                        ++pcur;
                    if (!*pcur)
                        throw 0;
                    *pcur = 0;
                    svars["pro"] = pro;
                    ++pcur;
                    if (!*pcur)
                        throw 0;
                    ++pcur;
                    if (!*pcur)
                        throw 0;
                    svars["class"] = pcur;

                    int cnt;
                    sscanf(row[9], "%d", &cnt);
                    svars["scorep"] = row[9];
                    if (cnt == 0)
                        svars["score"] = "0";
                    else
                    {
                        int sum;
                        sscanf(row[8], "%d", &sum);
                        double score = sum;
                        score /= cnt;
                        sprintf(query, "%.1lf", score);
                        svars["score"] = query;
                    }
                }
                catch (...)
                {
                }

                inc("../html/header.html");
                inc("../html/nav.html");
                inc("../html/p_u.html");
                inc("../html/footer.html");
            }
            mysql_free_result(res);
        }
        mysql_close();
    }
    else if (strncmp(cgiRequestUri, "/search?", 8) == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        if (uid.empty())
        {
            mysql_close();
            fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
            return 0;
        }
        cgiHeaderContentType("text/html");
        /* DEBUG 
        puts(cgiRequestMethod); // GET
        puts(cgiRequestUri);    // /search?n=%E5%BC%A0
        char s_n[1024];     
        cgiFormString("n", s_n, 1024); 
        puts(s_n);              // 张
        */
        if (GetStep() == S_STEP)
        {
            svars["title"].assign(u("中南大学家园网 - 搜索结果"));
            svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_search.css\" media=\"all\" />";
            svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_search.js\"></script>";

            inc("../html/header.html");
            inc("../html/nav.html");
            inc("../html/p_search_h.html");

            bool allow = true;
            mysql_queryf(query, "select email_validated from users_stu where id=%d", uid.c_str());
            auto res = mysql_store_result(&mysql);
            auto row = mysql_fetch_row(res);
            bool validated = (row[0][0] == '1');
            mysql_free_result(res);
            if (!validated)
            {
                mysql_queryf(query, "select uid from log_search_stu where uid=%d", uid.c_str());
                res = mysql_store_result(&mysql);
                row = mysql_fetch_row(res);
                if (row)
                {
                    allow = false;
                }
                mysql_free_result(res);
            }

            if (allow)
            {
                int len;
                cgiFormStringSpaceNeeded("n", &len);
                if (len > 1)
                {
                    if (mysql_queryf(query, "call search(%ps,%d,%s,%s)", "n", uid.c_str(), cgiRemoteAddr, cgiUserAgent) == 0)
                    {
                        auto res = mysql_store_result(&mysql);
                        MYSQL_ROW row;
                        char *pcur;
                        while (row = mysql_fetch_row(res))
                        {
                            svars["did"] = row[0];
                            svars["name"] = row[1];

                            pcur = row[2];
                            char*& academy = row[2];
                            while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
                                ++pcur;
                            try
                            {
                                if (!*pcur)
                                    throw 0;
                                char *grade = pcur;
                                while (*pcur && (*pcur >= '0' && *pcur <= '9'))
                                    ++pcur;
                                if (!*pcur)
                                    throw 0;
                                *pcur = 0;
                                svars["grade"] = grade;
                                grade[0] = 0;
                                svars["academy"] = academy;
                                ++pcur;
                                if (!*pcur)
                                    throw 0;
                                ++pcur;
                                if (!*pcur)
                                    throw 0;
                                ++pcur;
                                if (!*pcur)
                                    throw 0;
                                char *pro = pcur;
                                while (*pcur && !(*pcur >= '0' && *pcur <= '9'))
                                    ++pcur;
                                if (!*pcur)
                                    throw 0;
                                *pcur = 0;
                                svars["pro"] = pro;
                            }
                            catch (...)
                            {
                            }
                            inc("../html/li_search_result.html");
                        }
                        mysql_free_result(res);
                    }
                }
            }
            else
            {
                fputs("未通过邮箱验证的用户，限使用1次搜索！", cgiOut);
            }
            inc("../html/p_search_f.html");
            inc("../html/footer.html");
        }
        mysql_close();

    }
    else if (strcmp(cgiRequestUri, "/search") == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        if (uid.empty())
        {
            mysql_close();
            fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
            return 0;
        }

        if (GetStep() != S_STEP)
        {
            fprintf(cgiOut, "Location: home\r\n\r\n");
            return 0;
        }
        mysql_close();

        cgiHeaderContentType("text/html");
        svars["title"].assign(u("中南大学家园网 - 搜索"));
        svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_search.css\" media=\"all\" />";
        svars["js"]  = "<script type=\"text/javascript\" src=\"../js/p_search.js\"></script>";

        inc("../html/header.html");
        inc("../html/nav.html");
        inc("../html/p_search_h.html");
        inc("../html/p_search_f.html");
        inc("../html/footer.html");
    }
    else if (strcmp(cgiRequestUri, "/rating") == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();

        cgiHeaderContentType("text/plain");
        char did[17];
        char score[3];
        cgiFormString("did", did, 17);
        cgiFormString("score", score, 3);
        
        int check;
        sscanf(score, "%d", &check);
        if (check < 0 || check > 10)
        {
            mysql_close();
            return 0;
        }

        string stuid;
        mysql_queryf(query, "select stuid from si_stu where did=%s", did);
        auto res = mysql_store_result(&mysql);
        auto row = mysql_fetch_row(res);
        if (!row)
        {
            mysql_free_result(res);
            mysql_close();
            return 0;
        }
        stuid = row[0];
        mysql_free_result(res);
        
        mysql_queryf(query, "select id from ratings where uid=%d and stuid=%s and TO_DAYS(NOW())=TO_DAYS(time_prc) limit 1", uid.c_str(), stuid.c_str());
        res = mysql_store_result(&mysql);
        row = mysql_fetch_row(res);
        if (row)
        {
            putchar('2');
            mysql_free_result(res);
            mysql_close();
            return 0;
        }
        mysql_free_result(res);

        mysql_queryf(query, "call rating(%d,%s,%d,%s,%s)", uid.c_str(), stuid.c_str(), score, cgiRemoteAddr, cgiUserAgent);
        putchar('1');
        mysql_close();
    }
    else if (strcmp(cgiRequestUri, "/login") == 0)
    {
        char post_id[33];
        char post_pwd[17];
        char post_r[2];
        cgiFormString("id", post_id, 33);
        cgiFormString("pwd", post_pwd, 17);
        cgiFormString("r", post_r, 2);

        // 4 fun
        if (strstr(post_pwd, " or ") || strstr(post_id, " or "))
        {
            cgiHeaderContentType("text/plain");
            putchar('2');
            return 0;
        }

        mysql_init();
        mysql_connect();

        //mysql_queryf(query, "select 

        bool b = false;
        if (mysql_queryf(query, "call user_login(%ps,%s,%s,%s,%s)", "id", post_pwd, md5(post_pwd).c_str(), cgiRemoteAddr, cgiUserAgent) == 0)
        {
            auto res = mysql_store_result(&mysql);
            if (res != nullptr)
            {
                auto row = mysql_fetch_row(res);
                if (row != nullptr)
                {
                    b = true;
                    uid = row[0];
                    mysql_free_result(res);
                    mysql_clear_result();
                    session_save(post_r[0] == '1');
                }
                else
                {
                    mysql_free_result(res);
                    mysql_clear_result();
                }
            }
        }
        cgiHeaderContentType("text/plain");
        putchar(b ? '1' : '0');
        mysql_close();
    }
    else if (strcmp(cgiRequestUri, "/logout") == 0)
    {
        mysql_init();
        mysql_connect();
        session_destroy();
        mysql_close();
        fprintf(cgiOut, "Location: ../\r\n\r\n");
    }
    else if (strncmp(cgiRequestUri, "/news/", 6) == 0)
    {
        cgiHeaderContentType("text/html");
        string fpath = "../data/news/";
        fpath.append(cgiRequestUri + 6).append(".html");
        inc(fpath.c_str(), true);
    }
    else if (strcmp(cgiRequestUri, "/enlist_submit") == 0)
    {
        fprintf(cgiOut, "\r\n");
        mysql_init();
        mysql_connect();
        mysql_queryf(query, "call enlist_submit(%ps,%ps,%ps,%ps,%s,%s)", "email", "stuid", "phone", "txt", cgiRemoteAddr, cgiUserAgent);
        mysql_close();
    }
    else if (strcmp(cgiRequestUri, "/suggestion_submit") == 0)
    {
        fprintf(cgiOut, "\r\n");
        mysql_init();
        mysql_connect();
        mysql_queryf(query, "call suggestion_submit(%ps,%ps,%s,%s)", "email", "txt", cgiRemoteAddr, cgiUserAgent);
        mysql_close();
    }
    else if (strcmp(cgiRequestUri, "/sendverifymail") == 0)
    {
        int l;
        cgiFormStringSpaceNeeded("email", &l);
        if (l > 5)
        {
            mysql_init();
            mysql_connect();
            session_start();
            if (!uid.empty())
            {
                char emailcode[33];
                genEmailCode(emailcode);
                if (mysql_queryf(query, "call changeEmail(%d,%ps,%s,%s,%s)", uid.c_str(), "email", emailcode, cgiRemoteAddr, cgiUserAgent) == 0)
                {
                    mysql_queryf(query,
                        "select did,display_name from users_stu where id=%d",
                        uid.c_str());
                    auto res = mysql_store_result(&mysql);
                    auto row = mysql_fetch_row(res);
                    char to[321];
                    char did[17];
                    did[16] = 0;
                    string message = "<html><meta http-equiv=\"content-type\"content=\"text/html; charset=UTF-8\"/><body><table><tr><td></td><td style=\"font-size:14px;\">";
                    memcpy(did, row[0], 16);
                    message += row[1];
                    mysql_free_result(res);

                    cgiFormString("email", to, 321);
                    message += "，你好！</td></tr><tr><td></td></tr><tr><td></td></tr><tr><td></td><td style=\"font-size:14px;\">点击以下链接完成邮箱验证并激活在CSU家园网的帐号：</td></tr><tr><td></td></tr><tr><td></td><td><a href=\"http://csujy.com/verify?";
                    message += did;
                    message += '&';
                    message += emailcode;
                    message += "\">http://csujy.com/verify?";
                    message += did;
                    message += '&';
                    message += emailcode;
                    message += "</a></td></tr><tr><td></td></tr><tr><td></td><td style=\"font-size:14px;\">如无法点击，请将链接拷贝到浏览器地址栏中直接访问。</td></tr><tr><td></td></tr><tr><td></td><td>---------------------------------------------------------</td></tr><tr><td></td></tr><tr><td></td><td style=\"border-bottom:1px solid #e0e0e0; font-size:12px; line-height:2px;\"height=\"2\">&nbsp;</td></tr><tr><td></td><td style=\"color:#999999;font-size:12px;\">本邮件包含登录信息,<span style=\"font-weight:bold;\">请勿转发他人</span>。系统自动发出,<span style=\"font-weight:bold;\">请勿直接回复</span>。<br/>如有疑问或建议,可发送邮件至<a href=\"mailto:csujy@csujy.com\">csujy@csujy.com</a>。<br/></td></tr></table></body></html>";
                    mail("CSU家园网 <noreply@csujy.com>", to, "感谢使用CSU家园网，请完成邮箱验证", message.c_str());
                }
            }
            mysql_close();
        }
        fprintf(cgiOut, "\r\n");
    }
    else if (strcmp(cgiRequestUri, "/testbrowser") == 0)
    {
        cgiHeaderContentType("text/html");
        inc("../html/testbrowser.html", true);
    }
    else if (strncmp(cgiRequestUri, "/verify?", 8) == 0)
    {
        mysql_init();
        mysql_connect();
        try
        {
            char *pcur = cgiRequestUri + 8;
            while (*pcur && *pcur != '&')
                ++pcur;
            if (*pcur != '&')
                throw 1;
            *pcur++ = '\0';
            mysql_queryf(query, "call verifyEmail(%s,%s)", cgiRequestUri + 8, pcur);
            auto res = mysql_store_result(&mysql);
            if (res)
            {
                auto row = mysql_fetch_row(res);
                if (row == nullptr || row[0] == nullptr)
                {
                    mysql_free_result(res);
                    mysql_clear_result();
                    throw 2;
                }
                uid = row[0];
            }
            mysql_free_result(res);
            mysql_clear_result();
            session_save();
            mysql_close();
            fprintf(cgiOut, "Location: home\r\n\r\n");
        }
        catch (...)
        {
            session_destroy();
            mysql_close();
            fprintf(cgiOut, "Location: .\r\n\r\n");
        }
    }
    else if (strcmp(cgiRequestUri, "/skipverify") == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        if (!uid.empty() && GetStep() == 1)
        {
            mysql_queryf(query, "update users_stu set step=2 where id=%d and email is not null and email <> ''", uid.c_str());
        }
        mysql_close();
        fprintf(cgiOut, "Location: home\r\n\r\n");
    }
    else if (strcmp(cgiRequestUri, "/home?createpass") == 0)
    {
        char post_pwd[17];
        cgiFormString("pwd", post_pwd, 17);
        mysql_init();
        mysql_connect();
        session_start();
        if (!uid.empty() && GetStep() == 2)
        {
            mysql_queryf(query, "call createpass(%d,%s,%s,%s,%s)", uid.c_str(), post_pwd, md5(post_pwd).c_str(), cgiRemoteAddr, cgiUserAgent);
        }
        mysql_close();
        fprintf(cgiOut, "\r\n\r\n");
    }
    else if (strcmp(cgiRequestUri, "/resendall") == 0)
    {
        mysql_init();
        mysql_connect();
        if (mysql_queryf(query, "select u.did,u.display_name,v.code,u.email from email_verify_codes v inner join users_stu u on u.id=v.uid limit 100") == 0)
        {
            auto res = mysql_store_result(&mysql);
            MYSQL_ROW row;
            char did[17];
            did[16] = 0;
            string message;
            while (row = mysql_fetch_row(res))
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
                mail("CSU家园网 <noreply@csujy.com>", row[3], "感谢使用CSU家园网，请完成邮箱验证", message.c_str());
            }
            mysql_free_result(res);
        }
        mysql_close();
        fprintf(cgiOut, "\r\n");
    }
    else if (strcmp(cgiRequestUri, "/agreement") == 0)
    {
        cgiHeaderContentType("text/html");

        svars["title"].assign(u("CSU家园网 - 服务条款"));
        svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_agreement.css\" media=\"all\" />";

        inc("../html/header.html");
        inc("../html/p_agreement.html");
        inc("../html/footer.html");
    }
    else if (strcmp(cgiRequestUri, "/uploadphoto") == 0)
    {
        mysql_init();
        mysql_connect();
        session_start();
        if (uid.empty())
        {
            mysql_close();
            fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
            return 0;
        }

        if (GetStep() != S_STEP)
        {
            fprintf(cgiOut, "Location: home\r\n\r\n");
            return 0;
        }
        
        svars["title"].assign(u("CSU家园网 - 上传照片"));
        svars["css"] = "<link rel=\"stylesheet\" type=\"text/css\" href=\"../css/p_uploadfile.css\" media=\"all\" />";

        int size;
        cgiFilePtr file;
        int got;
        cgiFormFileSize("photo", &size);
        if (size > 2 * 1024 * 1024)
            svars["info"] = u("请勿上传大于2M的文件！");
        else
        {
            if (cgiFormFileOpen("photo", &file) != cgiFormSuccess)
                svars["info"] = u("上传文件失败！");
            else
            {
                time_t t;
                t = time(0);
                tm* t2 = localtime(&t);
                string path =
#ifdef _WIN32
                    "E:\\__Sowicm\\_Projects\\_All\\csujy\\data\\photos\\";
#else
                    "../../csujy_data/photos/";
#endif
                char d[9];
                sprintf(d, "%04d%02d%02d", t2->tm_year + 1900, t2->tm_mon + 1, t2->tm_mday);
#ifndef _WIN32
                mkdir(path.c_str(), S_IRWXU);
#endif
                path.append(uid);
#ifndef _WIN32
                mkdir(path.c_str(), S_IRWXU);
#endif                
                path.append("/").append(d);
#ifndef _WIN32
                mkdir(path.c_str(), S_IRWXU);
#endif
                string path2;
                char str[13];
                str[12] = 0;

                int randint1;
                long long randint2 = (long long)&randint2;
                srand(randint2 + (time(0) * randint1));

                char uints[26 + 10];
                int un = 0;
                for (char a = 'a'; a <= 'z'; ++a)
                    uints[un++] = a;
                for (char a = '0'; a <= '9'; ++a)
                    uints[un++] = a;
                while (true)
                {
                    str[0] = uints[rand() % 26];
                    for (int i = 1; i < 12; ++i)
                    {
                        str[i] = uints[rand() % un];
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

                FILE *fp = fopen(path2.c_str(), "wb");
                if (fp)
                {
                    while (cgiFormFileRead(file, query, sizeof(query), &got) == cgiFormSuccess)
                    {
                        fwrite(query, 1, got, fp);
                    }
                    fclose(fp);
                    path.assign(d).append("/").append(str).append(".jpg");
                    mysql_queryf(query, "call stu_modify_info(%d,'avatar_path',%s,%s,%s)",
                        uid.c_str(), path.c_str(), cgiRemoteAddr, cgiUserAgent);
                    svars["info"] = u("上传头像成功!");
                }
                else
                {
                    svars["info"] = u("上传头像失败!");
                }
                cgiFormFileClose(file);
            }
        }
        mysql_close();


        cgiHeaderContentType("text/html");
        inc("../html/header.html");
        inc("../html/nav.html");
        inc("../html/p_uploadphoto.html");
        inc("../html/footer.html");
    }
    else if (strcmp(cgiRequestUri, "/choosephoto") == 0)
    {
        fputs("\r\n", cgiOut);
        mysql_init();
        mysql_connect();
        session_start();
        if (uid.empty())
        {
            mysql_close();
            fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
            return 0;
        }

        if (GetStep() != S_STEP)
        {
            fprintf(cgiOut, "Location: home\r\n\r\n");
            return 0;
        }
        
        mysql_queryf(query, "call stu_modify_info(%d,'avatar_path','',%s,%s)",
            uid.c_str(), cgiRemoteAddr, cgiUserAgent);
        mysql_close();
    }
    else if (strcmp(cgiRequestUri, "/reagree") == 0)
    {
        fputs("\r\n", cgiOut);
        mysql_init();
        mysql_connect();
        session_start();
        if (uid.empty())
        {
            mysql_close();
            fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
            return 0;
        }

        if (GetStep() != S_STEP)
        {
            fprintf(cgiOut, "Location: home\r\n\r\n");
            return 0;
        }
        
        mysql_queryf(query, "update users_stu set agreed=1 where id=%d", uid.c_str());
        mysql_close();
    }
    else if (strcmp(cgiRequestUri, "/nomoretest") == 0)
    {
        char expires[1024];
        auto t1 = time(0);
        auto t2 = localtime(&t1);
        ++t2->tm_year;
        strftime(expires, 1024, "%A, %d-%b-%y %H:%M:%S GMT", t2);
        fprintf(cgiOut, "Set-Cookie: csujyNoMoreTest=true;Expires=%s;\r\n\r\n", expires);
    }
    else
    {
#if _WIN32
        MessageBox(0, cgiRequestUri, "Missed RequestUrl", 0);
#endif
        // 没有http:// 则为"302"
        fprintf(cgiOut, "Location: http://%s\r\n\r\n", getenv("HTTP_HOST"));
    }
	return 0;
}