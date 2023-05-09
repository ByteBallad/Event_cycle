#include "client.h"

bool Client::Socket_Init()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        return false;
    }
    return true;
}

bool Client::Connect_to_Server()
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip.c_str());

    int res = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if(res == -1)
    {
        return false;
    }
    return true;
}

void Client::print_info()
{
    if(!dl_flg)
    {
        cout<<"------------------------"<<endl;
        cout<<"用户名: "<<curr_name<<"   状态: 未登录"<<endl;
        cout<<"1 登录        2 注册"<<endl;
        cout<<"------------------------"<<endl;
        cin>>choice;
    }
    else
    {
        cout<<"------------------------"<<endl;
        cout<<"1 查看预约     2 预订"<<endl;
        cout<<"3 显示我的预约  4 取消预约"<<endl;
        cout<<"5 退出"<<endl;
        cout<<"------------------------"<<endl;
        cin>>choice;
        choice += 2;
    }
}

void Client::Zc_user()
{
    string user_tel;
    string user_name;
    string user_passwd;
    cout<<"请输入用户手机号码:"<<endl;
    cin>>user_tel;
    cout<<"请输入注册的用户名:"<<endl;
    cin>>user_name;
    cout<<"请输入用户密码:"<<endl;
    cin>>user_passwd;

    if(user_tel.empty() || user_name.empty() || user_passwd.empty())
    {
        cout<<"输入有误"<<endl;
        return;
    }

    Json::Value val;
    val["type"] = "ZC";
    val["user_tel"] = user_tel;
    val["user_name"] = user_name;
    val["user_passwd"] = user_passwd;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char status_buff[128] = {0};
    int n = recv(sockfd, status_buff, 127, 0);
    if(n<=0)
    {
        cout<<"server close"<<endl;
        return;
    }
    cout<<status_buff<<endl;

    Json::Value res_val;
    Json::Reader Read;
    Read.parse(status_buff, res_val);

    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"注册失败"<<endl;
        return;
    }

    curr_name = user_name;
    curr_tel = user_tel;
    cout<<"注册成功"<<endl;
    dl_flg = true;

}

void Client::Dl_user()
{
    string user_tel;
    string user_passwd;
    cout<<"请输入用户手机号码:"<<endl;
    cin>>user_tel;
    cout<<"请输入用户密码:"<<endl;
    cin>>user_passwd;

    if(user_tel.empty() || user_passwd.empty())
    {
        cout<<"输入无效"<<endl;
        return;
    }

    Json::Value val;
    val["type"] = "DL";
    val["user_tel"] = user_tel;
    val["user_passwd"] = user_passwd;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()+1), 0);

    char status_buff[128] = {0};
    int num = recv(sockfd, status_buff, 127, 0);
    if(num<=0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(status_buff, res_val))
    {
        cout<<"无法解析json字符串"<<endl;
        return;
    }

    //string status_str = res_val["type"].asString();
    string status_str = res_val["status"].asString();
    if(status_str.compare("OK")!=0)
    {
        cout<<"登陆失败"<<endl;
        return;
    }

    cout<<"登录成功"<<endl;
    dl_flg = true;
    curr_name = res_val["user_name"].asString();
    curr_tel = user_tel;
}

void Client::Ck_yuyue()
{
    Json::Value val;
    val["type"] = "CK";
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char res_buff[1024] = {0};
    if(recv(sockfd, res_buff, 1023, 0) <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }
    //cout<<res_buff<<endl;

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(res_buff, res_val))
    {
        cout<<"json无法解析"<<endl;
        return;
    }
    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"查看预约信息失败"<<endl;
        return;
    }

    int num = res_val["num"].asInt();
    if(num <= 0)
    {
        cout<<"暂时没有可预约的信息"<<endl;
        return;
    }

    cout<<"----------------------------------------------------"<<endl;
    cout<<"|  序号  |  地点名称  |  票数  |  已预约  |  日期  |"<<endl;

    map_tk.clear();
    //res_val["ticket_arr"].size();
    for(int i = 0; i < num; i++)
    {
        map_tk.insert(make_pair(i, res_val["ticket_arr"][i]["ticket_id"].asInt()));
        cout<<endl;
        cout<<"   ";
        cout<<setw(7)<<left<<i;
        cout<<setw(20)<<left<<res_val["ticket_arr"][i]["ticket_name"].asString();
        cout<<setw(10)<<left<<res_val["ticket_arr"][i]["ticket_max"].asInt();
        cout<<setw(10)<<left<<res_val["ticket_arr"][i]["count"].asInt();
        cout<<setw(12)<<left<<res_val["ticket_arr"][i]["day_time"].asString()<<endl;
    }
    cout<<"----------------------------------------------------"<<endl;
}

void Client::Yd_ticket()
{
    cout<<"请输入要预订的序号:"<<endl;
    int num;
    cin>>num;

    map<int, int>::iterator pos = map_tk.find(num);
    if(pos == map_tk.end())
    {
        cout<<"输入无效"<<endl;
        return;
    }
    int tk_id = pos->second;

    Json::Value val;
    val["type"] = "YD";
    val["user_tel"] = curr_tel;
    val["ticket_id"] = tk_id;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char status_buff[128] = {0};
    if(recv(sockfd, status_buff, 127, 0) <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(status_buff, res_val))
    {
        cout<<"json解析错误"<<endl;
        return;
    }
    if(res_val["status"].asString().compare("OK") != 0)
    {
        cout<<"预约失败"<<endl;
        return;
    }
    cout<<"预约成功"<<endl;
}

void Client::Xsyd_ticket()
{
    Json::Value val;
    val["type"] = "XSYD";
    val["user_tel"] = curr_tel;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char res_buff[1024] = {0};
    int n = recv(sockfd, res_buff, 1023, 0);
    if(n <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(res_buff, res_val))
    {
        cout<<"解析json失败"<<endl;
        return;
    }

    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"查询我的预约失败"<<endl;
        return;
    }

    int num = res_val["num"].asInt();
    if(num <= 0)
    {
        cout<<"当前用户没有预约信息"<<endl;
        return;
    }

    cout<<"当前用户共预约:"<<num<<"条信息"<<endl;
    cout<<"---------------------------------"<<endl;
    cout<<"序号     预约信息"<<endl;

    map_my_tk.clear();
    for(int i = 0; i < num; i++)
    {
        map_my_tk.insert(make_pair(i, res_val["yd_arr"][i]["yd_id"].asInt()));
        cout<<i<<"         "<<res_val["yd_arr"][i]["ticket_name"].asString()<<endl;
    }

    cout<<"---------------------------------"<<endl;
}

void Client::Qxyd_ticket()
{
    cout<<"请输入取消预定的序号:"<<endl;
    int num;
    cin>>num;

    map<int, int>::iterator pos = map_my_tk.find(num);
    if(pos == map_my_tk.end())
    {
        cout<<"输入无效"<<endl;
        return;
    }
    int yd_id = pos->second;

    Json::Value val;
    val["type"] = "QXYD";
    val["yd_id"] = yd_id;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char status_buff[128] = {0};
    if(recv(sockfd, status_buff, 127, 0) <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(status_buff, res_val))
    {
        cout<<"Json解析错误"<<endl;
        return;
    }
    if(res_val["status"].asString().compare("OK") != 0)
    {
        cout<<"取消预约失败"<<endl;
        return;
    }

    cout<<"取消预约成功"<<endl;
}

void Client::run()
{
    bool running = true;
    while(running)
    {
        print_info();
        switch (choice)
        {
        case DL:
            Dl_user();
            break;
        case ZC:
            Zc_user();
            break;
        case CK:
            Ck_yuyue();
            break;
        case YD:
            Yd_ticket();
            break;
        case XSYD:
            Xsyd_ticket();
            break;
        case QXYD:
            Qxyd_ticket();
            break;
        case TC:
            running = false;
            break;
        default:
            cout<<"请重新输入!"<<endl;
            break;
        }
    }
}

int main()
{
    Client cli;
    if(!cli.Socket_Init())
    {
        cout<<"socket init err"<<endl;
        exit(1);
    }

    if(!cli.Connect_to_Server())
    {
        cout<<"connect to server err"<<endl;
        exit(1);
    }

    cli.run();

    exit(0);
}