
#ifndef USER_H
#define USER_H
#include <string>

class SCGIEnv;
class SCookie;

#define S_STEP 3

class Student
{
public:
    void            gets(const char* _did);

public:
    std::string     uid;
    std::string     did;
    std::string     stuid;
    int             step;
    int             type;
    std::string     display_name;
    std::string     avatar_path;
    char            s_score[8];
    std::string     s_scoreCnt;
    int             n_scoreSum;
    int             n_scoreCnt;
    double          d_score;
    bool            agreed;
};

class User : public Student
{
public:
                                User(SCookie& cookie, SCGIEnv& env, bool load = true);

    void                        destroy();
    void                        save(bool remember);

public:
    int                         get_int(const char* col);
    void                        gets();

    void                        generateSessionID();
    void                        generateEmailCode(char* buf);

public:
    bool                        email_validated;

    bool                        loggedin;
    std::string                 sessionID;

protected:
    SCookie&                    cookie;
    SCGIEnv&                    env;
};

#endif // USER_H
