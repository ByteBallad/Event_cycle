#include "admin.h"

bool Admin::Socket_Init()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        return false;
    }
    return true;
}

bool Admin::Connect_to_Server()
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

void Admin::Dl_admin()
{
    string admin_tel;
    string admin_passwd;
    cout<<"请输入管理员手机号码:"<<endl;
    cin>>admin_tel;
    cout<<"请输入管理员密码:"<<endl;
    cin>>admin_passwd;
    if(admin_tel.empty() || admin_passwd.empty())
    {
        cout<<"输入无效"<<endl;
        return;
    }

    Json::Value val;
    val["type"] = "DL_ADMIN";
    val["admin_tel"] = admin_tel;
    val["admin_passwd"] = admin_passwd;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char status_buff[128] = {0};
    int num = recv(sockfd, status_buff, 127, 0);
    if(num <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(status_buff, res_val))
    {
        cout<<"无法解析Json字符串"<<endl;
        return;
    }

    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"登陆失败"<<endl;
        return;
    }

    cout<<"登录成功"<<endl;
    dl_flg = true;
    curr_name = res_val["admin_name"].asString();
    curr_tel = admin_tel;
}

void Admin::Tj_user()
{
    string user_tel;
    string user_name;
    string user_passwd;
    int user_state = 1;
    cout<<"请输入用户手机号:"<<endl;
    cin>>user_tel;
    cout<<"请输入用户名:"<<endl;
    cin>>user_name;
    cout<<"请输入用户密码:"<<endl;
    cin>>user_passwd;
    if(user_tel.empty() || user_name.empty() || user_passwd.empty())
    {
        cout<<"输入无效"<<endl;
        return;
    }

    Json::Value val;
    val["type"] = "TJ";
    val["user_tel"] = user_tel;
    val["user_name"] = user_name;
    val["user_passwd"] = user_passwd;
    val["user_state"] = user_state;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char status_buff[128] = {0};
    int num = recv(sockfd, status_buff, 127, 0);
    if(num <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(status_buff, res_val))
    {
        cout<<"无法解析Json字符串"<<endl;
        return;
    }

    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"添加用户失败"<<endl;
        return;
    }
    cout<<"添加用户成功"<<endl;
}

void Admin::Sc_user()
{
    string user_tel;
    cout<<"请输入要删除用户的手机号:"<<endl;
    cin>>user_tel;
    if(user_tel.empty())
    {
        cout<<"输入无效"<<endl;
        return;
    }

    Json::Value val;
    val["type"] = "SC";
    val["user_tel"] = user_tel;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char status_buff[128] = {0};
    int num = recv(sockfd, status_buff, 127, 0);
    if(num <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(status_buff, res_val))
    {
        cout<<"无法解析Json字符串"<<endl;
        return;
    }

    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"删除用户失败"<<endl;
        return;
    }
    cout<<"删除用户成功"<<endl;
}

void Admin::Ck_user()
{
    Json::Value val;
    val["type"] = "CK_YH";

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char res_buff[1024] = {0};
    if(recv(sockfd, res_buff, 1023, 0) <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(res_buff, res_val))
    {
        cout<<"无法解析Json字符串"<<endl;
        return;
    }

    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"查看用户信息失败"<<endl;
        return;
    }

    int num = res_val["num"].asInt();
    if(num <= 0)
    {
        cout<<"暂没有注册用户信息"<<endl;
        return;
    }
    cout<<"----------------------------------------------------"<<endl;
    cout<<"|  序号  |  注册手机号  |  注册用户名  |  密码  |  状态  |"<<endl;
    map_yh.clear();
    for(int i = 0; i < num; i++)
    {
        map_yh.insert(make_pair(i, res_val["yh_arr"][i]["id_tel"].asString()));
        cout<<endl;
        cout<<"   ";
        cout<<setw(7)<<left<<i;
        cout<<setw(20)<<left<<res_val["yh_arr"][i]["id_tel"].asString();
        cout<<setw(10)<<left<<res_val["yh_arr"][i]["user_name"].asString();
        cout<<setw(10)<<left<<res_val["yh_arr"][i]["user_passwd"].asString();
        cout<<setw(12)<<left<<res_val["yh_arr"][i]["state"].asInt()<<endl;
    }
    cout<<"----------------------------------------------------"<<endl;
}

void Admin::Ck_ticket()
{
    Json::Value val;
    val["type"] = "CK_YY";
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

void Admin::Fb_yuyue()
{
    string tk_name;
    int tk_max;
    cout<<"请输入发布票的名称:"<<endl;
    cin>>tk_name;
    cout<<"请输入该票的最大预约数:"<<endl;
    cin>>tk_max;
    if(tk_name.empty())
    {
        cout<<"输入无效"<<endl;
        return;
    }

    Json::Value val;
    val["type"] = "FB_YY";
    val["tk_name"] = tk_name;
    val["tk_max"] = tk_max;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str())+1, 0);

    char status_buff[128] = {0};
    int num = recv(sockfd, status_buff, 127, 0);
    if(num <= 0)
    {
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(status_buff, res_val))
    {
        cout<<"无法解析Json字符串"<<endl;
        return;
    }
    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"发布预约失败"<<endl;
        return;
    }
    cout<<"发布预约成功"<<endl;
}

void Admin::Sc_yuyue()
{
    cout<<"请输入要删除可预约票的序号:"<<endl;
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
    val["type"] = "SC_YY";
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
        cout<<"无法解析Json字符串"<<endl;
        return;
    }

    string status_str = res_val["status"].asString();
    if(status_str.compare("OK") != 0)
    {
        cout<<"删除可预约票失败"<<endl;
        return;
    }
    cout<<"删除可预约票成功"<<endl;
}

void Admin::print_info()
{
    if(!dl_flg)
    {
        cout<<"------------------------"<<endl;
        cout<<"管理员: "<<curr_name<<"   状态: 未登录"<<endl;
        cout<<"1 登录"<<endl;
        cout<<"------------------------"<<endl;
        cin>>choice;
        //choice += 7;
    }
    else
    {
        cout<<"------------------------"<<endl;
        cout<<"1 添加用户     2 删除用户"<<endl;
        cout<<"3 查看用户     4 查看预约"<<endl;
        cout<<"5 发布预约     6 删除预约"<<endl;
        cout<<"7 退出"<<endl;
        cout<<"------------------------"<<endl;
        cin>>choice;
        choice += 1;
    }
}

void Admin::run()
{
    bool running = true;
    while(running)
    {
        print_info();
        switch (choice)
        {
        case DL_ADMIN:
            Dl_admin();
            break;
        case TJ:
            Tj_user();
            break;
        case SC:
            Sc_user();
            break;
        case CK_YH:
            Ck_user();
            break;
        case CK_YY:
            Ck_ticket();
            break;
        case FB_YY:
            Fb_yuyue();
            break;
        case SC_YY:
            Sc_yuyue();
            break;
        case TC_ADMIN:
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
    Admin cli;
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