#include "rpcprovider.h"
#include <string>
#include "mprpcapplication.h"
#include <functional>
#include <google/protobuf/descriptor.h>
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;


/*
service_name:UserService
method_name:Login


*/
void RpcProvider::NotifyService(google::protobuf::Service* service)
{
    // 获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务对象的名字
    string service_name = pserviceDesc->name();
    // cout << "service_name: " << service_name << endl;
    LOG_INFO("service_name: %s", service_name.c_str());
    // 获取服务对象的方法数量
    int method_num = pserviceDesc->method_count();
    ServiceInfo service_info; // 存储服务对象的信息
    service_info.m_service = service; // 存储服务对象F
    // 遍历服务对象的方法
    for (int i = 0; i < method_num; ++i)
    {
        // 获取服务对象指定下标的方法描述信息
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        // 获取服务对象的方法名
        string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // cout << "method_name: " << method_name << endl;
        LOG_INFO("method_name: %s", method_name.c_str());
    }
    
    m_serviceMap.insert({service_name, service_info});
}
// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    InetAddress address(ip, port);

    // 创建TcpServer对象，开始监听和提供服务
    TcpServer server(&m_eventloop, address, "RpcProvider");
    // 注册连接回调 和 消息读写回调函，分离了网络和业务
    server.setConnectionCallback(bind(&RpcProvider::OnConnection, this, _1));
    server.setMessageCallback(bind(&RpcProvider::OnMessage, this, _1, _2, _3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start(); // 连接zkserver
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    cout << "RpcProvider start service at ip: " << ip << " port: " << port << endl;

    // 启动服务
    server.start();
    m_eventloop.loop(); // 阻塞在这里，循环处理I/O事件
}

//新的socket连接回调
void RpcProvider::OnConnection(const TcpConnectionPtr& conn)
{
    if (!conn->connected())
    {
        // 连接断开
        conn->shutdown(); // 调用shutdown关闭连接，muduo库会自动销毁连接对象
    }

}

/*
在框架内部，RpcProvider和RpcConsumer之间协商好，rpc请求的格式
service_name method_name args 定义proto的message类型，进行数据的序列化和反序列化
                                header_str: service_name + method_name + args_size（防止粘包）
header_size(4个字节）+ header_str + args_str
*/
// 已建立连接的socket读事件回调, 处理rpc请求,
void RpcProvider::OnMessage(const TcpConnectionPtr& conn, 
                            Buffer* buf, 
                            Timestamp time)
{
    string recv_buf = buf->retrieveAllAsString(); // 网络上接收的rpc请求码流，方法名+参数

    // 从字符流中读取前四个字节,二进制存储
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流, 反序列化数据，得到rpc请求的详细信息
    string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpc_header;
    // 反序列化rpc请求的数据头
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpc_header.ParseFromString(rpc_header_str))
    {
        // 反序列化成功
        service_name = rpc_header.service_name();
        method_name = rpc_header.method_name();
        args_size = rpc_header.args_size();
    }
    else
    {
        // 反序列化失败
        cout << "rpc_header_str: " << rpc_header_str << " parse error" << endl;
        return;
    }
    // 获取rpc请求的参数
    string args_str = recv_buf.substr(4 + header_size, args_size);
    cout << "=====================================================================" << endl;
    cout << "header_size: " << header_size << "rpc_header_str: " << rpc_header_str << endl;
    cout << "service_name: " << service_name << " method_name: " << method_name << " args_str: " << args_str << endl;
    cout << "=====================================================================" << endl;

    // 获取service对象 和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        cout << "service_name: " << service_name << " is not found" << endl;
        return;
    }
    
    auto method_it = it->second.m_methodMap.find(method_name);
    if (method_it == it->second.m_methodMap.end())
    {
        cout << service_name << " : " << method_name << " is not found" << endl;
        return;
    }
    google::protobuf::Service* service = it->second.m_service; // 获取service对象  UserService
    const google::protobuf::MethodDescriptor* method = method_it->second; // 获取method对象 Login

    // 生成rpc 方法调用的请求request和响应response的参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))
    {
        cout << "request args_str: " << args_str << " parse error" << endl;
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();
    // 给下面的method->CallMethod()调用绑定一个Closure的回调函数
    google::protobuf::Closure* done = 
        google::protobuf::NewCallback<RpcProvider, const TcpConnectionPtr& , google::protobuf::Message* >(this, &RpcProvider::SendRpcResponse, conn, response);
    // 在框架上根据service对象和method对象，调用rpc方法，得到结果
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作，用于序列化rpc响应和网络发送
void RpcProvider::SendRpcResponse(const TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    string response_str ;
    if(response->SerializeToString(&response_str)) // 序列化rpc响应
    {
        // 序列化成功, 发送rpc响应
        conn->send(response_str);
        
    }
    else
    {
        // 序列化失败
        cout << "response_str: " << response_str << " serialize error" << endl;
    }
    conn->shutdown(); //模拟短链接，响应后关闭连接
}