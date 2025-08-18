#include "mprpcconfig.h"
#include <iostream>
#include <string>
using namespace std;

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    std::cout << "Trying to open config file: " << config_file << std::endl;
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr)
    {
        cout << config_file << " is not exist" << endl;
        exit(EXIT_FAILURE);
    }
    // 1.注释 2.正确的配置项 3.开头多余的空格
    while(!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        // 去掉字符串前面多余的空格
        string read_buf(buf);
        Trim(read_buf);

        // 判断是否是注释
        if(read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }
        // 判断是否是配置项
        int idx = read_buf.find('=');
        if(idx == -1)
        {
            continue; // 说明不是配置项
        }
        string key;
        string value;
        key = read_buf.substr(0, idx);
        // 去掉key中的空格
        Trim(key);
        int endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx+1, endidx-idx-1);
        // 去掉value中的空格
        Trim(value);

        // 将key和value存入map中
        m_configMap.insert({key, value});
    }

}

// 查询配置项信息，根据key获取value
string MprpcConfig::Load(const string &key)
{
    auto it = m_configMap.find(key);
    if(it == m_configMap.end())
    {
        return "";
    }
    else
    {
        return it->second;
    }
}

// 去掉字符串两端的空格
void MprpcConfig::Trim(string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if(idx != -1)
    {
        // 说明有开头空格
        src_buf = src_buf.substr(idx, src_buf.size()-idx);
    }
    // 去掉字符串后面的空格
    idx = src_buf.find_last_not_of(' ');
    if(idx != -1)
    {
        // 说明有末尾空格
        src_buf = src_buf.substr(0, idx+1);
    }
}