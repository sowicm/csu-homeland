
#include "user.h"
#include "time.h"           // time localtime
#include "rand_ts.h"
#include "SCookie.h"
#include "SCGIEnv.h"
#include "SMySQL.h"
#include "sqlMutex.h"
#include "SCGIApp.h"        // errorlog
using std::string;

User::User(SCookie& _cookie, SCGIEnv& _env, bool load)
    : cookie(_cookie)
    , env(_env)
{
    loggedin = false;
    if (load)
    {

        auto it = cookie.find("csujySessionID");
        // if (it == cookie.end())
        // {
        //     generateSessionID();
        //     cookie.set("csujySessionID", sessionID);
        // }
        // else
        if (it != cookie.end())
        {
            sessionID = std::move(it->second);
            SMySQLRes res;
            sqlMutex.lock();
            if (sql.queryf("call session_start(%s, %s)", sessionID.c_str(), env.RemoteAddr()))
            {
                res = sql.store_res();
                sql.clear_res();
            }
            sqlMutex.unlock();
            if (res)
            {
                auto row = res.fetch_row();
                if (row != nullptr && row[0] && row[0][0])
                {
                    uid = row[0];
                    loggedin = true;
                }
            }
        }
        if (!loggedin)
        {
            uid = "0";
            sessionID = "";
        }
    }
    else
    {
        uid       = "0";
        sessionID = "";
    }
}

void User::destroy()
{
    if (loggedin)
    {
        sqlMutex.lock();
        sql.queryf("call session_destroy(%s)", sessionID.c_str());
        sql.clear_res();
        sqlMutex.unlock();
    }
    cookie.erase("csujySessionID");
}

void User::save(bool remember)
{
    debuglog("generateSessionID[");
    generateSessionID();
    debuglog("]");
    if (remember)
    {
        char expires[1024];
        auto t1 = time(0);
        auto t2 = localtime(&t1);
        ++t2->tm_year;
        strftime(expires, 1024, "%A, %d-%b-%y %H:%M:%S GMT", t2);
        cookie.set("csujySessionID", sessionID.c_str(), expires);
    }
    else
        cookie.set("csujySessionID", sessionID.c_str());
    debuglog("session_save[");
    sqlMutex.lock();
    sql.queryf("call session_save(%s,%s,%d)", sessionID.c_str(), env.RemoteAddr(), uid.c_str());
    sql.clear_res();
    sqlMutex.unlock();
    debuglog("]");
}

void Student::gets(const char* _did)
{
    did = _did;
    SMySQLRes res;
    sqlMutex.lock();
    sql.queryf("select id,stuid,avatar_path from users_stu where did=%s limit 1", _did);
    res = sql.store_res();
    sqlMutex.unlock();
    auto row = res.fetch_row();
    if (row)
    {
        uid = row[0];
        stuid = row[1];
        avatar_path = (row[2] == nullptr ? "" : row[2]);
    }
    else
    {
        sqlMutex.lock();
        sql.queryf("select stuid from si_stu where did=%s limit 1", _did);
        res = sql.store_res();
        sqlMutex.unlock();
        row = res.fetch_row();
        stuid = row[0];
    }
    res.free();
}

void User::gets()
{
    type = 0;
    step = 1;
    if (loggedin)
    {
        SMySQLRes res;
        sqlMutex.lock();
        sql.queryf("select u.step,u.display_name,u.agreed,u.did,u.avatar_path,sum(r.score),count(r.score),type,email_validated from users_stu u left join ratings r on r.stuid=u.stuid where u.id=%d limit 1", uid.c_str());
        res = sql.store_res();
        sqlMutex.unlock();
        auto row = res.fetch_row();
        if (row)
        {
            if (sscanf(row[0], "%d", &step) < 1)
                step = 1;
            display_name = row[1];
            agreed = (row[2] && row[2][0] == '1');
            did = row[3];
            avatar_path = (row[4] == nullptr ? "null" : row[4]);
            sscanf(row[6], "%d", &n_scoreCnt);
            s_scoreCnt = row[6];
            if (n_scoreCnt == 0)
            {
                s_score[0] = '0';
                s_score[1] = 0;
            }
            else
            {
                sscanf(row[5], "%d", &n_scoreSum);
                d_score = n_scoreSum;
                d_score /= n_scoreCnt;
                sprintf(s_score, "%.1lf", d_score);
            }
            if (sscanf(row[7], "%d", &type) < 1)
                type = 0;
            email_validated = (row[8][0] == '1');
        }
        res.free();
    }
}

/*
const char* Session::get_str(const char* col)
{
    sql.queryf("select %d from users_stu where id=%d limit 1", col, uid.c_str());
    sql.store_res();
    auto row = sql.fetch_row();
    return row[0];
}
*/

int User::get_int(const char* col)
{
    sqlMutex.lock();
    sql.queryf("select %d from users_stu where id=%d limit 1", col, uid.c_str());
    SMySQLRes res = sql.store_res();
    sqlMutex.unlock();
    auto row = res.fetch_row();
    int n;
    if (sscanf(row[0], "%d", &n) < 1)
        n = -1;
    return n;
}


void User::generateSessionID()
{
    char uints[26 + 26 + 10];
    int un = 0;
    for (char a = 'a'; a <= 'z'; ++a)
        uints[un++] = a;
    for (char a = '0'; a <= '9'; ++a)
        uints[un++] = a;
    for (char a = 'A'; a <= 'Z'; ++a)
        uints[un++] = a;

    SMySQLRes res;
    MYSQL_ROW row;
    string s;
    do
    {
        sessionID = "";
        for (int i = 0; i < 64; ++i)
            sessionID.push_back(uints[rand_ts() % un]);
        s.assign("select `uid` from `sessions` where `sid`='").append(sessionID);
        s.push_back('\'');
        sqlMutex.lock();
        sql.query(s.c_str());
        res = sql.store_res();
        sqlMutex.unlock();
        row = res.fetch_row();
    } while (row);
}

void User::generateEmailCode(char* buf)
{
    char uints[26 + 26 + 10];
    int un = 0;
    for (char a = 'a'; a <= 'z'; ++a)
        uints[un++] = a;
    for (char a = '0'; a <= '9'; ++a)
        uints[un++] = a;
    for (char a = 'A'; a <= 'Z'; ++a)
        uints[un++] = a;

    SMySQLRes res;
    MYSQL_ROW row;
    string s;
    buf[32] = 0;
    do
    {
        for (int i = 0; i < 32; ++i)
            buf[i] = uints[rand_ts() % un];
        s.assign("select `uid` from `email_verify_codes` where `code`='").append(buf);
        s.push_back('\'');
        sqlMutex.lock();
        sql.query(s.c_str());
        res = sql.store_res();
        sqlMutex.unlock();
        row = res.fetch_row();
    } while (row);
}


/*
Sessions::Sessions(MYSQL* mysql, char* sessionid)
{
    needUpdate = false;
    pms = mysql;
    char *p = sessionid;
    while (*p)
    {
        if (! (
            *p >= 'a' && *p <= 'z' ||
            *p >= 'A' && *p <= 'Z' ||
            *p >= '0' && *p <= '9'
            ))
            break;
    }
    if (*p)
        sid = "none";
    else
    {
        string sql = "select data from sessions where sid='";
        sql.append(sid).append("'");
        mysql_query(pms, sql.c_str());
        auto res = mysql_store_result(pms);
        auto row = mysql_fetch_row(res);
        if (row != nullptr)
        {
            char *p = row[0];
            char *pKeyStart, *pKeyEnd, *pValueStart, *pValueEnd;
            while (*p)
            {
                if (*p == '\n')
                {
                    ++p;
                    continue;
                }
                pKeyStart = p++;
                while (*p && *p != ':')
                    ++p;
                if (!*p)
                    break;
                pKeyEnd = p;
                pValueStart = ++p;
                while (*p && *p != '\n')
                    ++p;
                if (!*p)
                    break;
                pValueEnd = p;
                sessionNodes[move(string(pKeyStart, pKeyEnd))] = move(string(pValueStart, pValueEnd));
            }
        }
        mysql_free_result(res);
        sid = sessionid;
    }
}
Sessions::~Sessions()
{
    if (needUpdate)
    {
        string sql = "replace into sessions (sid, ip, data) values (";
        for (auto itr = sessionNodes.begin(); itr != sessionNodes.end(); ++itr)
        {
            data += itr->first + ":" + itr->second + "\n";
        }

    }
}

const string& Sessions::get(const string& key)
{
    return sessionNodes[key];
}
void Sessions::set(const string& key, const string& value)
{
    needUpdate = true;
    sessionNodes[key] = value;
}
void Sessions::set(const string& key, string&& value)
{
    needUpdate = true;
    sessionNodes[key] = value;
}
*/
