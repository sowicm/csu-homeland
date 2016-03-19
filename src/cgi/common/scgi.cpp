#include "scgi.h"
#include "cgic.h"
#include <iostream>
#include <regex>
#ifdef _WIN32
#include <Windows.h>
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif
using namespace std;

void inc(const char* filepath, bool real)
{
	FILE *fp = fopen(filepath, "rb");
	if (fp == nullptr)
		return;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	char* buffer = new char[size + 1];
	if (buffer == nullptr)
		return;
	fseek(fp, 0, SEEK_SET);
	buffer[fread(buffer, 1, size, fp)] = '\0';
	fclose(fp);
    if (real)
        fwrite(buffer, 1, size, cgiOut);
    else
	    echo(buffer);
	delete[] buffer;
}

void echo(const char* script)
{
	const char *pCur = script;
#if 0
	cmatch match;
	while ( regex_search(pCur, match, regex("\\{\\$[a-zA-Z0-9_]+\\}")) )
	{
		fwrite(pCur, 1, match[0].first - pCur, stdout);
		cout << svars[string(match[0].first + 2, match[0].second - 1)];
		pCur = match[0].second;
	}
	fprintf(stdout, pCur);
#else
    const char* pStart = pCur;
    const char* first;
    string* pstr;
    while (true)
    {
        while (*pCur && *pCur != '{')
            ++pCur;
        if (!*pCur)
            break;
        first = pCur++;
        if (*pCur != '$')
            continue;
        ++pCur;
        while (*pCur &&
            (
            *pCur >= 'a' && *pCur <= 'z' ||
            *pCur >= 'A' && *pCur <= 'Z' ||
            *pCur >= '0' && *pCur <= '9')
            )
            ++pCur;
        if (!*pCur)
            break;
        if (*pCur != '}')
            continue;
        fwrite(pStart, 1, first - pStart, cgiOut);
        pstr = &svars[string(first + 2, pCur)];
        fwrite(pstr->data(), 1, pstr->length(), cgiOut);
		pStart = ++pCur;
    }
	fprintf(cgiOut, pStart);
#endif
}

#ifdef _WIN32
string utf8(const char* ansi)
{
    int wccnt;
    int mbcnt;
    wchar_t *wstr;
    char *utf8str;

    wccnt = MultiByteToWideChar(CP_ACP, 0, ansi, -1, NULL, 0);
    do
    {
        wstr = new wchar_t[wccnt + 1];
    } while (!wstr);
    wccnt = MultiByteToWideChar(CP_ACP, 0, ansi, -1, wstr, wccnt);
    wstr[wccnt] = '\0';
    mbcnt = WideCharToMultiByte(CP_UTF8, 0, wstr, wccnt, NULL, 0, NULL, NULL);
    do
    {
        utf8str = new char[mbcnt + 1];
    } while (!utf8str);
    mbcnt = WideCharToMultiByte(CP_UTF8, 0, wstr, wccnt, utf8str, mbcnt, NULL, NULL);
    utf8str[mbcnt] = '\0';

    return string(utf8str);
}
#endif

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