#ifndef LOGGER_H
#define LOGGER_H

#include "lockqueue.h" // 日志队列
#include <string>
#include <iostream>

using namespace std;



enum Loglevel
{
    INFO, // 普通信息
    ERROR, // 错误信息
};
// Mprpc框架提供的日志系统
class Logger
{
public:
    static Logger& GetInstance(); // 获取单例对象
    void SetLogLevel(Loglevel level); // 设置日志级别
    void Log(string msg); // 写日志
private:
    int m_loglevel; // 记录日志的级别
    LockQueue<string> m_lckQue; // 日志队列

    Logger();
    Logger(const Logger &) = delete; // 禁止拷贝构造
    Logger(const Logger &&) = delete; // 禁止移动构造
};


// 定义宏
#define LOG_INFO(logmsgformat, ...) \
    do { \
        Logger& logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, sizeof(c), logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do { \
        Logger& logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, sizeof(c), logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \


#endif // LOGGER_H