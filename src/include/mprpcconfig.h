#ifndef MPRPCCOFIG_H
#define MPRPCCOFIG_H

#include <unordered_map>
#include <string>
using namespace std;

//rpcserverip rpcserverport zookeeperip zookeeperport
//框架读取配置文件类
class MprpcConfig
{
public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char* config_file);

    // 查询配置项信息，根据key获取value
    string Load(const string &key);

private:
    unordered_map<string, string> m_configMap;
    // 去掉字符串两端的空格
    void Trim( string &s);
   
};



#endif // MPRPCCOFIG_H