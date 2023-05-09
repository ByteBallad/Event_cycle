#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>

using namespace std;

const int LIS_MAX = 5;

enum CHO_TYPE {DL=1, ZC, CK, YD, XSYD, QXYD, TC};

class MyLibevent;

//Ser_socket
class Ser_Socket
{
public:
    //构造函数中不要写入容易出错的代码，不易处理该错误
    //无参构造函数
    Ser_Socket()
    {
        sockfd = -1;
        m_port = 6000;
        ip = "127.0.0.1";
    }
    //带参构造函数
    Ser_Socket(const string ips, const short port)
    {
        sockfd = -1;
        ip = ips;
        m_port = port;
        
    }
    //初始化套接字
    bool Socket_Init()
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
            return false;
        }
        return true;
    }

    bool Socket_Bind();

    int Get_Sockfd()
    {
        return sockfd;
    }
private:
    int sockfd;
    short m_port;
    string ip;
};

class Mysql_Client
{
public:
    Mysql_Client()
    {
        db_ip = "127.0.0.1";
        db_port = 3306;
        db_username = "root";
        db_name = "Res_Info";
        db_passwd = "135799";

    }

    Mysql_Client(string ips, short port, string username, string name, string passwd)
    {
        db_ip = ips;
        db_port = port;
        db_username = username;
        db_name = name;
        db_passwd = passwd;
    }

    bool Init_Connect();

    void Close_Mysql();

    bool Db_Zc_user(const string &tel, const string &name, const string &pw);

    bool Db_Dl_user(const string &tel, string &name, string &pw);

    bool Db_Ck_yuyue(Json::Value &ck_val);

    bool Db_Yd_ticket(const string &tel, int tk_id);

    bool Db_Xsyd_ticket(Json::Value &res_val, const string &tel);

    bool Db_Qxyd_ticket(int yd_id);


private:
    string db_ip;
    short db_port;
    string db_username;
    string db_name;
    string db_passwd;

    MYSQL mysql_con;
};

//回调函数
class Call_Back
{
public:
    virtual void Call_Back_Fun() = 0;

    struct event* ev;
};

//监听套接字
class Accept_Call_Back: public Call_Back
{
public:
    Accept_Call_Back(MyLibevent* plib, int sockfd)
    {
        this->sockfd = sockfd;
        this->plib = plib;
    }

    void Call_Back_Fun();

private:
    int sockfd;
    MyLibevent* plib;
};

//连接套接字
class Recv_Call_Back: public Call_Back
{
public:
    Recv_Call_Back(int c)
    {
        this->c = c;
        map_table.insert(make_pair("ZC", ZC));
        map_table.insert(make_pair("DL", DL));
        map_table.insert(make_pair("CK", CK));
        map_table.insert(make_pair("YD", YD));
        map_table.insert(make_pair("XSYD", XSYD));
        map_table.insert(make_pair("QXYD", QXYD));
    }
    void Call_Back_Fun();

private:
    void Send_Err();

    void Send_Ok();

    void Zc_user();

    void Dl_user();

    void Ck_yuyue();

    void Yd_ticket();

    void Xsyd_ticket();

    void Qxyd_ticket();

private:
    int c;
    map<string, enum CHO_TYPE> map_table;
    Json::Value val;
    Json::Reader Read;
};

//MyLibevent－－－libevent库
class MyLibevent
{
public:
    MyLibevent()
    {
        base = NULL;
    }

    ~MyLibevent()
    {
        event_base_free(base);
    }

    bool Lib_Init();

    bool Lib_Add(int fd, Call_Back* ptr);

    void Lib_Dispatch();
private:
    struct event_base* base;
};