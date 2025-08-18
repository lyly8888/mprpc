#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


using namespace std;
int main(int argc, char** argv) {
    // 初始化Rpc框架：程序启动后，想要使用mprpc框架提供的功能，必须先初始化框架（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());

    fixbug::GetFriendListRequest request;
    request.set_userid(1000);
    fixbug::GetFriendListResponse response;

    // 
    MprpcController controller;
    stub.GetFriendList(&controller, &request, &response, nullptr); // 底层channel_->CallMethod 集中做所有rpc方法调用的参数序列化和网络发送
    // 一次rpc调用完成，打印结果
    if(controller.Failed())
    {
        cout << "rpc failed, error: " << controller.ErrorText() << endl;
    }
    else
    {
        if (0 == response.result().errcode()) {
            cout << "GetFriendList rpc success" << endl;
            for (int i = 0; i < response.friends_size(); ++i) {
                cout << "friend name = " << response.friends(i) << endl;
            }
            
        }
        else {
            cout << "GetFriendList rpc failed: " << response.result().errmsg() << endl;
        }
    }


    return 0;
}