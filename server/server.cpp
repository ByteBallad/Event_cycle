#include "server.h"

//Libevent的全局回调函数
void G_CALLBACK_FUN(int fd, short ev, void* arg)
{
    Call_Back* ptr = (Call_Back*)arg;
    if(ptr == NULL)
    {
        return;
    }
    if(ev & EV_READ)
    {
        ptr->Call_Back_Fun();
    }
}

bool Ser_Socket::Socket_Bind()
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(m_port);
    saddr.sin_addr.s_addr = inet_addr(ip.c_str());

    int res = bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if(res == -1)
    {
        return false;
    }

    res = listen(sockfd, LIS_MAX);
    if(res == -1)
    {
        return false;
    }
    
    return true;
}

bool MyLibevent::Lib_Init()
{
    base = event_init();
    if(NULL == base)
    {
        return false;
    }
    return true;
}

//向libevent中添加事件
bool MyLibevent::Lib_Add(int fd, Call_Back* ptr)
{
    struct event* p = event_new(base, fd, EV_READ|EV_PERSIST, G_CALLBACK_FUN, ptr);
    if(p == NULL)
    {
        return false;
    }
    if(event_add(p, NULL) == -1)
    {
        return false;
    }

    ptr->ev = p;

    return true;
}

//事件循环
void MyLibevent::Lib_Dispatch()
{
    if(NULL == base)
    {
        cout << "base NULL" << endl;
        return;
    }
    if(event_base_dispatch(base) == -1)
    {
        cout << "dispatch err" << endl;
        return;
    }
}

//Mysql_Client
bool Mysql_Client::Init_Connect()
{
    if(mysql_init(&mysql_con) == NULL)
    {
        return false;
    }
    if(mysql_real_connect(&mysql_con, db_ip.c_str(), db_username.c_str(), db_passwd.c_str(), db_name.c_str(), db_port, NULL, 0) == NULL)
    {
        return false;
    }
    return true;
}

bool Mysql_Client::Db_Zc_user(const string &tel, const string &name, const string &pw)
{
    string sql = string("insert into user_info values('")+tel+string("','")+name+string("','")+pw+string("',1)");
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }
    return true;
}

bool Mysql_Client::Db_Dl_user(const string &tel, string &name, string &pw)
{
    string sql = string("select user_name, user_passwd from user_info where id_tel=")+tel;
    if(mysql_query(&mysql_con, sql.c_str())!=0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }

    int rows = mysql_num_rows(r);
    int count = mysql_field_count(&mysql_con);
    if( rows!=1 || count!=2)
    {
        cout<<"db res: rows="<<rows<<" count="<<count<<endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(r);
    name = row[0];
    pw = row[1];

    mysql_free_result(r);
    return true;
}

bool Mysql_Client::Db_Ck_yuyue(Json::Value &ck_val)
{
    string sql = "select * from ticket_table";
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }

    int num = mysql_num_rows(r);
    cout<<"共有: "<<num<<"行"<<endl;
    
    for(int i=0; i<num; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(r);
        Json::Value tmp;
        tmp["ticket_id"] = stoi(row[0]);
        tmp["ticket_name"] = row[1];
        tmp["ticket_max"] = stoi(row[2]);
        tmp["count"] = stoi(row[3]);
        tmp["day_time"] = row[4];
        ck_val["ticket_arr"].append(tmp);
    }
    ck_val["status"] = "OK";
    ck_val["num"] = num;

    mysql_free_result(r);

    return true;
}

bool Mysql_Client::Db_Yd_ticket(const string &tel, int tk_id)
{
    string se_sql = "select tk_max, tk_count from ticket_table where tk_id="+to_string(tk_id);
    if(mysql_query(&mysql_con, se_sql.c_str()) != 0)
    {
        return false;
    }
    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }
    if(mysql_num_rows(r) != 1)
    {
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(r);
    int tk_max = stoi(row[0]);
    int tk_count = stoi(row[1]);
    if(tk_count >= tk_max)
    {
        return false;
    }

    tk_count++;
    string up_sql = string("update ticket_table set tk_count=")+to_string(tk_count)+string(" where tk_id=")+to_string(tk_id);
    if(mysql_query(&mysql_con, up_sql.c_str()) != 0)
    {
        return false;
    }

    string in_sql = string("insert into yd_set values(0,")+tel+string(",")+to_string(tk_id)+string(",")+to_string(1)+")";
    if(mysql_query(&mysql_con, in_sql.c_str()) != 0)
    {
        return false;
    }
    mysql_free_result(r);
    return true;
}

bool Mysql_Client::Db_Xsyd_ticket(Json::Value &res_val, const string &tel)
{
    string sql = string("select yd_set.yd_id, ticket_table.tk_name from yd_set, ticket_table where id_tel=")+tel+string(" and yd_set.tk_id=ticket_table.tk_id");
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }

    int num = mysql_num_rows(r);
    int count = mysql_field_count(&mysql_con);

    if(num < 0 || count != 2)
    {
        return false;
    }

    res_val["status"] = "OK";
    res_val["num"] = num;

    for(int i = 0; i < num; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(r);
        Json::Value tmp;
        tmp["yd_id"] = stoi(row[0]);
        tmp["ticket_name"] = row[1];
        res_val["yd_arr"].append(tmp);
    }
    mysql_free_result(r);
    return true;
}

bool Mysql_Client::Db_Qxyd_ticket(int yd_id)
{
    string sql_tk_id = string("select tk_id from yd_set where yd_id=")+to_string(yd_id);
    if(mysql_query(&mysql_con, sql_tk_id.c_str()) != 0)
    {
        return false;
    }
    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }
    if(mysql_num_rows(r) != 1 || mysql_field_count(&mysql_con) != 1)
    {
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(r);
    int tk_id = stoi(row[0]);
    
    string sql_de = string("delete from yd_set where yd_id=")+to_string(yd_id);
    if(mysql_query(&mysql_con, sql_de.c_str()) != 0)
    {
        return false;
    }
    mysql_free_result(r);

    string sql_tk_count = string("select tk_count from ticket_table where tk_id=")+to_string(tk_id);
    if(mysql_query(&mysql_con, sql_tk_count.c_str()) != 0)
    {
        return false;
    }
    r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }
    if(mysql_num_rows(r) != 1 || mysql_field_count(&mysql_con) != 1)
    {
        return false;
    }
    row = mysql_fetch_row(r);
    int tk_count = stoi(row[0]);
    if(tk_count <= 0)
    {
        return false;
    }
    tk_count--;

    string sql_up = string("update ticket_table set tk_count=")+to_string(tk_count)+string(" where tk_id=")+to_string(tk_id);
    if(mysql_query(&mysql_con, sql_up.c_str()) != 0)
    {
        return false;
    }
    
    mysql_free_result(r);
    return true;
}

bool Mysql_Client::Db_Dl_admin(const string &tel, string &name, string &pw)
{
    string sql = string("select user_name, user_passwd from admin_info where id_tel=")+tel;
    if(mysql_query(&mysql_con, sql.c_str())!=0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }

    int rows = mysql_num_rows(r);
    int count = mysql_field_count(&mysql_con);
    if( rows!=1 || count!=2)
    {
        cout<<"db res: rows="<<rows<<" count="<<count<<endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(r);
    name = row[0];
    pw = row[1];

    mysql_free_result(r);
    return true;
}

bool Mysql_Client::Db_Tj_user(const string &tel, const string &name, const string &passwd, int state)
{
    string sql = string("insert into user_info values('")+tel+string("','")+name+string("','")+passwd+string("',")+to_string(state)+string(")");
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }
    return true;
}

bool Mysql_Client::Db_Sc_user(const string &tel)
{
    string sql = string("delete from user_info where id_tel=")+tel;
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }
    return true;
}

bool Mysql_Client::Db_Ck_user(Json::Value &ck_val)
{
    string sql = "select * from user_info";
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }
    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }
    int num = mysql_num_rows(r);
    cout<<"共有:"<<num<<"行"<<endl;

    for(int i = 0; i < num; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(r);
        Json::Value tmp;
        tmp["id_tel"] = row[0];
        tmp["user_name"] = row[1];
        tmp["user_passwd"] = row[2];
        tmp["state"] = stoi(row[3]);
        ck_val["yh_arr"].append(tmp);
    }
    ck_val["status"] = "OK";
    ck_val["num"] = num;

    mysql_free_result(r);
    return true;
}

bool Mysql_Client::Db_Ck_ticket(Json::Value &ck_val)
{
    string sql = "select * from ticket_table";
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if(r == NULL)
    {
        return false;
    }

    int num = mysql_num_rows(r);
    cout<<"共有: "<<num<<"行"<<endl;
    
    for(int i=0; i<num; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(r);
        Json::Value tmp;
        tmp["ticket_id"] = stoi(row[0]);
        tmp["ticket_name"] = row[1];
        tmp["ticket_max"] = stoi(row[2]);
        tmp["count"] = stoi(row[3]);
        tmp["day_time"] = row[4];
        ck_val["ticket_arr"].append(tmp);
    }
    ck_val["status"] = "OK";
    ck_val["num"] = num;

    mysql_free_result(r);

    return true;
}

bool Mysql_Client::Db_Fb_yuyue(const string &tk_name, int tk_max)
{
    string sql = string("insert into ticket_table values(0,'")+tk_name+string("',")+to_string(tk_max)+string(",0,'2023-01-01')");
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }
    return true;
}

bool Mysql_Client::Db_Sc_yuyue(int tk_id)
{
    string sql = string("delete from ticket_table where tk_id=")+to_string(tk_id);
    if(mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        return false;
    }
    return true;
}

void Mysql_Client::Close_Mysql()
{
    mysql_close(&mysql_con);
}

//监听套接字回调函数
void Accept_Call_Back::Call_Back_Fun()
{
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int c = accept(sockfd, (struct sockaddr*)&caddr, &len);
    if(c < 0)
    {
        return;
    }
    cout<<"accept c: "<<c<<endl;

    Recv_Call_Back* p = new Recv_Call_Back(c);
    if(p == NULL)
    {
        close(c);
        cout<<"new recv call back err"<<endl;
    }
    if(!plib->Lib_Add(c, p))
    {
        close(c);
        delete p;
        cout<<"lib add c err"<<endl;
        return;
    }

}

void Recv_Call_Back::Send_Err()
{
    Json::Value res_val;
    res_val["status"] = "ERR";
    send(c, res_val.toStyledString().c_str(), strlen(res_val.toStyledString().c_str())+1, 0);
}

void Recv_Call_Back::Send_Ok()
{
    Json::Value res_val;
    res_val["status"] = "OK";
    send(c, res_val.toStyledString().c_str(), strlen(res_val.toStyledString().c_str())+1, 0);
}

void Recv_Call_Back::Zc_user()
{
    string tel = val["user_tel"].asString();
    string name = val["user_name"].asString();
    string passwd = val["user_passwd"].asString();
    if(tel.empty() || name.empty() || passwd.empty())
    {
        Send_Err();
        return;
    }

    //像数据库中写入数据
    Mysql_Client cli;
    cli.Init_Connect();
    if(!cli.Db_Zc_user(tel, name, passwd))
    {
        Send_Err();
        return;
    }
    Send_Ok();
    cli.Close_Mysql();
}

void Recv_Call_Back::Dl_user()
{
    string tel = val["user_tel"].asString();
    string pw = val["user_passwd"].asString();

    if(tel.empty() || pw.empty())
    {
        Send_Err();
        return;
    }

    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }

    string db_query_passwd;
    string user_name;
    if(!cli.Db_Dl_user(tel, user_name, db_query_passwd))
    {
        Send_Err();
    }
    if(db_query_passwd.compare(pw) != 0)
    {
        Send_Err();
    }
    else
    {
        Json::Value tmp;
        tmp["status"] = "OK";
        tmp["user_name"] = user_name;

        send(c, tmp.toStyledString().c_str(), strlen(tmp.toStyledString().c_str())+1, 0);
    }

    cli.Close_Mysql();
}

void Recv_Call_Back::Ck_yuyue()
{
    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }

    Json::Value ck_val;
    if(!cli.Db_Ck_yuyue(ck_val))
    {
        Send_Err();
        return;
    }

    send(c, ck_val.toStyledString().c_str(), strlen(ck_val.toStyledString().c_str())+1, 0);

    cli.Close_Mysql();
}

void Recv_Call_Back::Yd_ticket()
{
    string tel = val["user_tel"].asString();
    int tk_id = val["ticket_id"].asInt();

    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }
    if(!cli.Db_Yd_ticket(tel, tk_id))
    {
        Send_Err();
        return;
    }

    Send_Ok();
    cli.Close_Mysql();
}

void Recv_Call_Back::Xsyd_ticket()
{
    string user_tel = val["user_tel"].asString();
    if(user_tel.empty())
    {
        Send_Err();
        return;
    }

    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }

    Json::Value res_val;
    if(!cli.Db_Xsyd_ticket(res_val, user_tel))
    {
        Send_Err();
        return;
    }

    cli.Close_Mysql();
    
    send(c, res_val.toStyledString().c_str(), strlen(res_val.toStyledString().c_str())+1, 0);

}

void Recv_Call_Back::Qxyd_ticket()
{
    int yd_id = val["yd_id"].asInt();

    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }
    if(!cli.Db_Qxyd_ticket(yd_id))
    {
        Send_Err();
        return;
    }
    Send_Ok();
    cli.Close_Mysql();
}

void Recv_Call_Back::Dl_admin()
{
    string tel = val["admin_tel"].asString();
    string pw = val["admin_passwd"].asString();

    if(tel.empty() || pw.empty())
    {
        Send_Err();
        return;
    }

    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }

    string db_query_passwd;
    string admin_name;
    if(!cli.Db_Dl_admin(tel, admin_name, db_query_passwd))
    {
        Send_Err();
    }
    if(db_query_passwd.compare(pw) != 0)
    {
        Send_Err();
    }
    else
    {
        Json::Value tmp;
        tmp["status"] = "OK";
        tmp["user_name"] = admin_name;

        send(c, tmp.toStyledString().c_str(), strlen(tmp.toStyledString().c_str())+1, 0);
    }

    cli.Close_Mysql();
}

void Recv_Call_Back::Tj_user()
{
    string tel = val["user_tel"].asString();
    string name = val["user_name"].asString();
    string passwd = val["user_passwd"].asString();
    int state = val["user_state"].asInt();
    if(tel.empty() || name.empty() || passwd.empty())
    {
        Send_Err();
        return;
    }

    Mysql_Client cli;
    cli.Init_Connect();
    if(!cli.Db_Tj_user(tel, name, passwd, state))
    {
        Send_Err();
        return;
    }
    Send_Ok();
    cli.Close_Mysql();
}

void Recv_Call_Back::Sc_user()
{
    string tel = val["user_tel"].asString();
    if(tel.empty())
    {
        Send_Err();
        return;
    }

    Mysql_Client cli;
    cli.Init_Connect();
    if(!cli.Db_Sc_user(tel))
    {
        Send_Err();
        return;
    }
    Send_Ok();
    cli.Close_Mysql();
}

void Recv_Call_Back::Ck_user()
{
    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }
    Json::Value ck_val;
    if(!cli.Db_Ck_user(ck_val))
    {
        Send_Err();
        return;
    }

    send(c, ck_val.toStyledString().c_str(), strlen(ck_val.toStyledString().c_str())+1, 0);

    cli.Close_Mysql();
}

void Recv_Call_Back::Ck_ticket()
{
    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }

    Json::Value ck_val;
    if(!cli.Db_Ck_ticket(ck_val))
    {
        Send_Err();
        return;
    }

    send(c, ck_val.toStyledString().c_str(), strlen(ck_val.toStyledString().c_str())+1, 0);

    cli.Close_Mysql();
}

void Recv_Call_Back::Fb_yuyue()
{
    string tk_name = val["tk_name"].asString();
    int tk_max = val["tk_max"].asInt();
    if(tk_name.empty())
    {
        Send_Err();
        return;
    }

    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }
    if(!cli.Db_Fb_yuyue(tk_name, tk_max))
    {
        Send_Err();
        return;
    }
    Send_Ok();
    cli.Close_Mysql();
}

void Recv_Call_Back::Sc_yuyue()
{
    int tk_id = val["ticket_id"].asInt();

    Mysql_Client cli;
    if(!cli.Init_Connect())
    {
        Send_Err();
        return;
    }
    if(!cli.Db_Sc_yuyue(tk_id))
    {
        Send_Err();
        return;
    }
    Send_Ok();
    cli.Close_Mysql();
}

//连接套接字回调函数
void Recv_Call_Back::Call_Back_Fun()
{
    char buff[128] = {0};
    int n = recv(c, buff, 127, 0);
    if(n<=0)
    {
        //可以在MyLibevent类中实现Del方法，来封装event_free(ev)
        //plib->Del(ev);
        event_free(ev);
        close(c);
        cout<<"client close"<<endl;
        delete this;
        return;
    }
    cout<<buff<<endl;

    Read.parse(buff, val);
    string cmd_type = val["type"].asString();

    //map  "ZC" --> ZC
    map<string, enum CHO_TYPE>::iterator pos = map_table.find(cmd_type);
    if(pos == map_table.end())
    {
        Send_Err();
    }
    enum CHO_TYPE cho = pos->second;

    switch (cho)
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
    default:
        break;
    }


    /*
    Json::Value val;
    Json::Reader Read;
    Read.parse(buff, val);
    cout<<"类型: "<<val["type"].asString()<<endl;
    cout<<"tel: "<<val["user_tel"].asString()<<endl;
    cout<<"name: "<<val["user_name"].asString()<<endl;
    cout<<"passwd: "<<val["user_passwd"].asString()<<endl;

    val.clear();
    val["status"] = "OK";
    send(c, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);//测试
    */
}

int main()
{
    Ser_Socket ser;
    if(!ser.Socket_Init())
    {
        cout << "socket err" << endl;
        exit(1);
    }
    if(!ser.Socket_Bind())
    {
        cout << "bind err" << endl;
        exit(1);
    }

    MyLibevent* plib = new MyLibevent;
    if(plib == NULL)
    {
        cout<<"libevent create failed"<<endl;
        exit(1);
    }
    if(!plib->Lib_Init())
    {
        cout<<"libevent init err"<<endl;
        exit(1);
    }

    Accept_Call_Back* psock_cb = new Accept_Call_Back(plib, ser.Get_Sockfd());
    if(psock_cb == NULL)
    {
        cout<<"accept obj err"<<endl;
        exit(1);
    }
    if(!plib->Lib_Add(ser.Get_Sockfd(), psock_cb))
    {
        cout<<"lib add sockfd err"<<endl;
        exit(1);
    }

    plib->Lib_Dispatch();

    delete plib;
    
    exit(0);
}
