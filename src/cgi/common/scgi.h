#include "vars.h"

void inc(const char* filepath, bool real = false);
void echo(const char* script);
#ifdef _WIN32
std::string utf8(const char* ansi);
#endif

/*
error
class MYSQL;
class Sessions
{
public:
    Sessions(MYSQL* mysql, char* sessionid);
    ~Sessions();

    const string& get(const string& key);
    void          set(const string& key, const string& value);
    void          set(const string& key, string&& value);

private:
    bool                needUpdate;
    MYSQL*              pms;
    const char*         sid;
    map<string, string> sessionNodes;
};
*/