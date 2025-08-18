#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"

using namespace std;
using namespace fixbug;


/*
// UserService原来是一个本地服务，提供了两个进程内的本地方法：Login和GetFriendLists
class UserService {
public:
    bool Login(string name, string pwd)
    {
        cout << "do local service: login" << endl;
        cout << "name: " << name << " pwd: " << pwd << endl;
    } 
};

*/
class FriendService : public fixbug::FriendServiceRpc // 使用在rpc服务发布端（rpc服务提供者）
{
public:

    vector<string> GetFriendList(uint32_t userid)
    {
        cout << "do local service: GetFriendList, userid = " << userid << endl;
        vector<string> vec;
        vec.push_back("zhangsan");
        vec.push_back("lisi");
        vec.push_back("wangwu");
        return vec;
    }
    /*
    // 重写基类UserServiceRpc中的纯虚函数, 下面这些方法都是框架直接调用的
    1. caller  ===> Login(LoginRequest) => muduo => callee
    2. callee  ===> Login(LoginRequest)=>交到下面重写的这个Login方法上了
    */
    void GetFriendList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendListRequest* request,
                       ::fixbug::GetFriendListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid(); // 获取远程调用请求参数

        vector<string> vec = GetFriendList(userid); // 调用本地服务获取结果

        response->mutable_result()->set_errcode(0); // 设置结果
        response->mutable_result()->set_errmsg("");
        for (vector<string>::iterator it = vec.begin(); it != vec.end(); ++it)
        {
            response->add_friends(*it);

        }

        done->Run(); // 执行回调操作
        
    }

    
};

int main(int argc, char **argv)
{
    // UserService userService;
    // userService.Login("zhangsan", "123456");
    LOG_INFO("first log message: start friendserver...");
    LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    // 调用框架初始化操作 provider -i config.conf
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());
    // provider.NotifyService(new ProductService());

    // 启动rpc服务发布节点，Run后，进入阻塞状态，等待远程调用请求
    provider.Run();

    return 0;
}