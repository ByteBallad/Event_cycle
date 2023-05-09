#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <jsoncpp/json/json.h>
#include <iomanip>

using namespace std;

enum CHO_TYPE {DL=1, ZC, CK, YD, XSYD, QXYD, TC};

class Client
{
public:
    Client()
    {
        ip = "127.0.0.1";
        port = 6000;
        sockfd = -1;
        dl_flg = false;
        curr_name = "游客";
        curr_tel = "";
        choice = -1;
    }

    Client(string ips, short port)
    {
        ip = ips;
        this->port = port;
        sockfd = -1;
        dl_flg = false;
        curr_name = "游客";
        curr_tel = "";
        choice = -1;
    }

    ~Client()
    {
        close(sockfd);
    }

    bool Socket_Init();

    bool Connect_to_Server();

    void run();
private:
    void print_info();

    void Zc_user();

    void Dl_user();

    void Ck_yuyue();

    void Yd_ticket();

    void Xsyd_ticket();

    void Qxyd_ticket();
private:
    int sockfd;
    string ip;
    short port;

    bool dl_flg;
    string curr_name;
    string curr_tel;

    int choice;
    map<int, int> map_tk;
    map<int, int> map_my_tk;
};