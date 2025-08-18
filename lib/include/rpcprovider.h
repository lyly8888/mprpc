#ifndef RPCPROVIDER_H
#define RPCPROVIDER_H
#include <memory>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

using namespace muduo;
using namespace muduo::net;
using namespace std;

#include "google/protobuf/service.h"
// 框架提供的专门服务发布rpc服务的网络对象
class RpcProvider
{
public:
    // 框架提供给外部使用的， 可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service* service);
    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    // 组合了muduo的EventLoop对象，用于事件循环
    EventLoop m_eventloop;
    // 服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service* m_service; // 保存rpc服务对象
        unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 保存rpc服务对象中所有方法
    };
    unordered_map<std::string, ServiceInfo> m_serviceMap; // 保存所有rpc服务对象

    //新的socket连接回调
    void OnConnection(const TcpConnectionPtr& conn);
    // 已建立连接的socket读事件回调
    void OnMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    // Closure的回调操作，用于序列化rpc响应和网络发送
    void SendRpcResponse(const TcpConnectionPtr& conn, google::protobuf::Message* response);
};



#endif // RPCPROVIDER_H