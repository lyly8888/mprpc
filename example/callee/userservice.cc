#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"


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
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端（rpc服务提供者）
{
public:
    bool Login(string name, string pwd)
    {
        cout << "do local service: login" << endl;
        cout << "name: " << name << " pwd: " << pwd << endl;
        return true;
    }
    bool Register(uint32_t id, string name, string pwd)
    {
        cout << "do local service: register" << endl;
        cout << "id: " << id << " name: " << name << " pwd: " << pwd << endl;
        return true;
    }
    /*
    // 重写基类UserServiceRpc中的纯虚函数, 下面这些方法都是框架直接调用的
    1. caller  ===> Login(LoginRequest) => muduo => callee
    2. callee  ===> Login(LoginRequest)=>交到下面重写的这个Login方法上了
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数LoginRequest, 应用程序获取相应数据做本地业务
        string name = request->name();
        string pwd = request->pwd();

        // 调用本地方法完成业务
        bool login_result = Login(name, pwd); 

        // 把响应写入response: 错误码、错误信息、返回值
        ResultCode * code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("success");
        response->set_success(login_result);

        // 执行回调操作，数据序列化及网络发送（由框架完成）
        done->Run();
        
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        string name = request->name();
        string pwd = request->pwd();

        bool register_result = Register(id, name, pwd);

        ResultCode * code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("success");
        response->set_success(register_result);

        done->Run();
    }

    
};

int main(int argc, char **argv)
{
    // UserService userService;
    // userService.Login("zhangsan", "123456");


    // 调用框架初始化操作 provider -i config.conf
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    // provider.NotifyService(new ProductService());

    // 启动rpc服务发布节点，Run后，进入阻塞状态，等待远程调用请求
    provider.Run();

    return 0;
}