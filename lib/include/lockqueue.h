#ifndef LOCKQUEUE_H
#define LOCKQUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
using namespace std;

// 异步写日志的日志队列
template <typename T>
class LockQueue
{
public:

    // 多个工作线程写日志
    void Push(const T& data)
    {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push(data);
        // 日志队列有数据，通知消费者
        m_condvariable.notify_one();
    }
    // 一个写日志线程读数据写到txt文件中
    T Pop()
    {
        unique_lock<mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            // 日志队列空，等待
            m_condvariable.wait(lock);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;


};









#endif // LOCKQUEUE_H