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

enum CHO_TYPE {DL_ADMIN=8, TJ, SC, CK_YH, CK_YY, FB_YY, SC_YY, TC_ADMIN};

class Admin
{
public:
    Admin()
    {
        ip = "127.0.0.1";
        port = 6000;
        sockfd = -1;
        dl_flg = false;
        curr_name = "游客";
        curr_tel = "";
        choice = -1;
    }

    Admin(string ips, short port)
    {
        ip = ips;
        this->port = port;
        sockfd = -1;
        dl_flg = false;
        curr_name = "游客";
        curr_tel = "";
        choice = -1;
    }

    ~Admin()
    {
        close(sockfd);
    }

    bool Socket_Init();

    bool Connect_to_Server();

    void run();
private:
    void print_info();

    void Dl_admin();

    void Tj_user();

    void Sc_user();

    void Ck_user();

    void Ck_ticket();

    void Fb_yuyue();

private:
    int sockfd;
    string ip;
    short port;

    bool dl_flg;
    string curr_name;
    string curr_tel;

    int choice;
    map<int, string> map_yh;
    map<int, int> map_tk;
};