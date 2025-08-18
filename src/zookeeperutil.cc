#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <iostream>


// 全局的watcher 观察者   zkserver给zkclient的通知
void global_watcher(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
    if( type == ZOO_SESSION_EVENT) // 表示回调的消息类型是和会话相关的
    {
        if(state == ZOO_CONNECTED_STATE) // 表示zkclient和zkserver连接成功
        {
            sem_t* sem = (sem_t*)zoo_get_context(zh); // 从指定的句柄zh中获取信号量
            sem_post(sem); // 信号量+1
        }

    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{

}  // 使用成员初始化列表将m_zhandle指针初始化为nullptr
ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}
// zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    /*
    zookeeper_mt：多线程版本
    zookeeper的API客户端程序提供了三个线程：API调用线程、网络I/O线程（pthread_create poll）、watcher回调线程
    */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (m_zhandle == nullptr) // 只表示资源句柄创建
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem); // 给指定的句柄m_zhandle设置信号量

    sem_wait(&sem);// 阻塞  ，等待信号量+1
    std::cout << "zookeeper_connect success!" << std::endl;

}
// 在zkserver上根据指定的path创建znode节点
void ZkClient::Create(const char* path, const char* data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ZNONODE == flag) // 表示path路径下的znode节点不存在，不存在就创建
	{
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen); // state表示节点的类型，永久节点还是临时节点
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}

}
// 根据参数指定的znode节点path获取该节点的数据
std::string ZkClient::GetData(const char* path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}

}
