//
// Created by 123456 on 2021/8/23.
// 头文件介绍 : 线程同步机制包装类，使用对象管理资源，要使用RAII机制(资源获取之时就是将管理资源的对象初始化的时候)
//

#ifndef LINUX_LOCKER_H
#define LINUX_LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/* 封装信号量的类 */
class sem
{
public:
    /* 创建并初始化信号量 */
    sem()
    {
        if(sem_init(&m_sem, 0, 0) != 0)
        {
            /* 构造函数吗，没有返回值，可以通过抛出异常来报告错误 */
            throw std::exception();
        }
    }

    /* 销毁信号量 */
    ~sem()
    {
        sem_destroy(&m_sem);
    }

    /* 等待信号量 相当于PV操作里的P操作 */
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }

    /* 增加信号量 相当于PV操作里的V操作 */
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }
private:
    sem_t m_sem;
};

/* 封装互斥锁的类 */
class Locker
{
public:
    /* 创建并初始化互斥锁 */
    Locker()
    {
        if(pthread_mutex_init(&m_mutex, nullptr) != 0)
        {
            throw std::exception();
        }
    }

    /* 销毁互斥锁 */
    ~Locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    /* 获取互斥锁 */
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    /* 释放互斥锁 */
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
private:
    pthread_mutex_t m_mutex;
};

/* 封装条件变量的类 */
class cond
{
public:
    /* 创建并初始化条件变量 */
    cond()
    {
        if (pthread_mutex_init(&m_mutex, nullptr) != 0)
        {
            throw std::exception();
        }
        if(pthread_cond_init(&m_con, nullptr) != 0)
        {
            /* 构造函数中一旦出现问题， 就应该立即释放已经成功分配了的资源 */
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }

    /* 销毁条件变量 */
    ~cond()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_con);
    }

    /* 等待条件变量 */
    bool wait()
    {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_con, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    /* 唤醒等待条件变量的线程 */
    bool signal()
    {
        return pthread_cond_signal(&m_con);
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_con;
};



#endif //LINUX_LOCKER_H
