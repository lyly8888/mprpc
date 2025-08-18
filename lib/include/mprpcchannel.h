#ifndef MPRPCCHANNEL_H
#define MPRPCCHANNEL_H
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>


using namespace google::protobuf;
using namespace std;

class MprpcChannel : public google::protobuf::RpcChannel 
{
public:
    // 所有通过stub代理对象调用的rpc方法都会调用这个方法，做统一的数据序列化和网络发送
    void CallMethod(const MethodDescriptor* method,
                          RpcController* controller, const Message* request,
                          Message* response, Closure* done);

    
};


#endif // MPRPCCHANNEL_H