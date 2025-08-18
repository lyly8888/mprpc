// #pragma once

#ifndef MPRPCAPPLICATION_H
#define MPRPCAPPLICATION_H

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
// mprpc 框架的基础类，负责框架的一些初始化操作
class MprpcApplication
{
public:
    static void Init(int argc, char** argv);
    static MprpcApplication& GetInstance(); // 单例模式
    static MprpcConfig& GetConfig(); // 获取配置文件
private:

    static MprpcConfig m_config; // 配置文件
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;

};



#endif // MPRPCAPPLICATION_H