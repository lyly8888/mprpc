#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

using namespace std;

MprpcConfig MprpcApplication::m_config;
void ShowArgsHelp()
{
    std::cout << "format: command -i <config_file>" << std::endl;
}
void MprpcApplication::Init(int argc, char* argv[])
{
    if(argc < 2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    string config_file;
    while((c = getopt(argc, argv, "i:")) != -1) // "i:"  i表示支持一个 -i 的选项， ：指后面必须跟一个参数
    {
        switch(c)
        {
            case 'i':
                config_file = optarg; // optarg表示getopt解析的参数，即 -i 后面的参数
                // std::cout << "Trying to open config file: " << optarg << std::endl;
                break;
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // 开始加载配置文件 rpcserver_ip=   rpcserver_port=  zookeeper_ip=  zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // cout << "rpcserverip: " << m_config.Load("rpcserverip") << endl;
    // cout << "rpcserverport: " << m_config.Load("rpcserverport") << endl;
    // cout << "zookeeperip: " << m_config.Load("zookeeperip") << endl;
    // cout << "zookeeperport: " << m_config.Load("zookeeperport") << endl;


}
MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}
// 获取配置文件
MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}