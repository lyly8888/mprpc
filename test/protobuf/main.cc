#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;
using namespace std;

// protoc test.proto --cpp_out=./
// g++ main.cc test.pb.cc -lprotobuf

int main()
{
    // // 对象套对象的使用
    // LoginResponse rsp;
    // ResultCode * rc = rsp.mutable_result(); // 可改变的成员对象，返回指针，用于修改
    // rc->set_errcode(0);
    // rc->set_errmsg("success");


    // 对象中套列表对象的使用
    GetFriendListsResponse rsp;
    ResultCode * rc = rsp.mutable_result();
    rc->set_errcode(0);
    // rc->set_errmsg("success");
    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);
    cout << rsp.friend_list_size() << endl;

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(30);
    user2->set_sex(User::WOMAN);
    cout << rsp.friend_list_size() << endl;

    int size = rsp.friend_list_size();
    for(int i = 0; i < size; i++)
    {
        // const User &user = rsp.friend_list(i);
        // cout << user.name() << " " << user.age() << " " << user.sex() << endl;
        User *user = rsp.mutable_friend_list(i);
        cout << user->name() << " " << user->age() << " " << user->sex() << endl;
    }

    return 0;

}
int main1()
{
    // 封装了login请求对象的数据
    LoginRequest req; // 创建对象
    req.set_name("zhang san");
    req.set_pwd("123456");
    
    // 对象数据序列化 =》 字符串
    string send_str;
    if(req.SerializeToString(&send_str))
    {
        cout << "SerializeToString success: "<< send_str.c_str() << endl; //zhang san123456
    }

    // 从send_str反序列化一个LoginRequest对象
    LoginRequest req2;
    if(req2.ParseFromString(send_str))
    {
        cout << "ParseFromString success: " << req2.name() << " " << req2.pwd() << endl; // zhang san 123456
    }

    return 0;
}