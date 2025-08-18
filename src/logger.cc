#include "logger.h"
#include <time.h>


// 获取单例对象
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}
Logger::Logger()
{
    // 启动专门的写日志线程
    thread writeLogTask([&]() {
        for(;;)
        {
            // 获取当前的日期，然后取日志信息，写入相应的日志文件中 a+
            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);
            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday);

            FILE* fp = fopen(file_name, "a+");
            if(fp == nullptr)
            {
                cout << "logger file: "<< file_name << " open failed!" << endl;
                exit(EXIT_FAILURE);
            }
            // 从缓冲区中取出日志信息，写入文件中
            string msg = m_lckQue.Pop();

            char time_buf[128] = {0};
            sprintf(time_buf, "%d-%d-%d %d:%d:%d => [%s]", 
                                                    nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday, nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec,
                                                    (m_loglevel == INFO?"info":"error"));
            msg.insert(0, time_buf);
            msg += "\n";
            // 写入文件中
            fputs(msg.c_str(), fp);
            fclose(fp);

        }    
    });
    // 设置线程为分离状态
    writeLogTask.detach();
}
// 设置日志级别
void Logger::SetLogLevel(Loglevel level)
{
    m_loglevel = level;
}
// 写日志,将日志写入lockqueue缓冲区
void Logger::Log(string msg)
{
    m_lckQue.Push(msg);
} 


    