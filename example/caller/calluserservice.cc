#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

using namespace std;
int main(int argc, char** argv) {
    // 初始化Rpc框架：程序启动后，想要使用mprpc框架提供的功能，必须先初始化框架（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    fixbug::LoginResponse response;

    stub.Login(nullptr, &request, &response, nullptr); // 底层channel_->CallMethod 集中做所有rpc方法调用的参数序列化和网络发送
    // 一次rpc调用完成，打印结果
    if (0 == response.result().errcode()) {
        cout << "login rpc success: " << response.success() << endl;
        
    }
    else {
        cout << "login rpc failed: " << response.result().errmsg() << endl;
    }

    // 演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("li si");
    req.set_pwd("666666");
    fixbug::RegisterResponse res;
    stub.Register(nullptr, &req, &res, nullptr);
    if (0 == res.result().errcode()) {
        
        cout << "register rpc success: " << res.success() << endl;
    }
    else {
        cout << "register rpc failed: " << res.result().errmsg() << endl;
    }

    return 0;
}