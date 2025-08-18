#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>
#include "mprpcapplication.h"
#include <arpa/inet.h>
#include <unistd.h>
#include "mprpccontroller.h"
#include "zookeeperutil.h"


// using namespace std;
// using namespace google::protobuf;

/*
                                
header_size(4个字节）+ header_str + args_str
header_str: service_name + method_name + args_size（防止粘包）
*/
// 所有通过stub代理对象调用的rpc方法都会调用这个方法，做统一的数据序列化和网络发送
void MprpcChannel::CallMethod(const MethodDescriptor* method,
                        RpcController* controller, const Message* request,
                        Message* response, Closure* done)
{
    const ServiceDescriptor* sd = method->service();
    string service_name = sd->name();
    string method_name = method->name();

    // 获取参数的序列化字符串长度 args_size
    string args_str;
    uint32_t args_size = 0;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        // cout << "serialize request failed" << endl;
        controller->SetFailed("serialize request failed");
        return;
    }
    // 构造rpc 的请求 header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        // cout << "serialize rpc header failed" << endl;
        controller->SetFailed("serialize rpc header failed");
        return;
    }
    // 组织rpc请求的完整字符串
    string send_rpc_str;
    send_rpc_str.insert(0, string((char*)&header_size, 4)); // 插入header_size
    send_rpc_str += rpc_header_str; // 插入header_str
    send_rpc_str += args_str; // 插入args_str

    cout << "=====================================================================" << endl;
    cout << "header_size: " << header_size << " rpc_header_str: " << rpc_header_str << endl;
    cout << "service_name: " << service_name << " method_name: " << method_name << " args_str: " << args_str << endl;
    cout << "=====================================================================" << endl;

    // 使用Tcp 编程，发送rpc请求,网络发送
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == -1)
    {
        // cout << "socket create failed, errno: "<< errno << endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "socket create failed, errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }
    // // 通过文件读取配置文件，获取服务器ip和port
    // string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 


    // 连接服务器
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "connect failed, errno: %d", errno);
        controller->SetFailed(errtxt);
        close(client_fd);
        return;
        
    }

    // 发送数据
    if(send(client_fd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1) 
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "send data failed, errno: %d", errno);
        controller->SetFailed(errtxt);
        close(client_fd);
        return;
    }
    // 接收数据
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if((recv_size = recv(client_fd, recv_buf, 1024, 0)) == -1)
    {

        char errtxt[512] = {0};
        sprintf(errtxt, "recv data failed, errno: %d", errno);
        controller->SetFailed(errtxt);
        close(client_fd);
        return;
    }
    // 反序列化rpc响应
    // string response_str(recv_buf, 0, recv_size);
    // if(!response->ParseFromString(response_str))
    // {
    //     cout << "response parse error, response_str: " << response_str << endl;
    //     close(client_fd);
    //     return;
    // }
    if(!response->ParseFromArray(recv_buf, recv_size))
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "response parse error, errno: %s", recv_buf);
        controller->SetFailed(errtxt);
        close(client_fd);
        return;
    }
    close(client_fd);
}