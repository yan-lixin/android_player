//
// Created by 李鑫 on 2020/11/9.
//

#ifndef FFMPEG_SAFE_QUEUE_H
#define FFMPEG_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>

class SafeQueue {
    typedef void (*ReleaseCallback)(T *);

    typedef void (*SyncHandle)(queue<T> &);

private:
    queue<T> q;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int work; // 标记队列是否工作
    ReleaseCallback releaseCallback;
    SyncHandle syncHandle;

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0); // 动态初始化
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    /**
     * 入队
     * @param value
     */
    void push(T value) {
        // 线程锁定
        pthread_mutex_lock(&mutex);
        if (work) {
            // 工作状态需要push
            q.push(value);
            pthread_cond_signal(&cond);
        } else {
            // 非工作状态
            if (releaseCallback) {
                releaseCallback(&value);
            }
        }
        // 解锁
        pthread_mutex_unlock(&mutex);
    }

    /**
     * 出队
     * @param value
     * @return
     */
    int pop(T &value) {
        int ret = 0;
        // 线程锁定
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            // 工作状态，说明确实需要pop，但是队列为空，需要等待
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            value = q.front();
            // 出队
            q.pop();
            ret = 1;
        }
        // 解锁
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    /**
     * 设置队列工作状态
     * @param work
     */
    void setWork(int work) {
        // 线程锁定
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        // 解锁
        pthread_mutex_unlock(&mutex);
    }

    /**
     * 判断队列是否为空
     * @return
     */
    int empty() {
        return q.empty();
    }

    /**
     * 获取队列大小
     * @return
     */
    int size() {
        return q.size();
    }

    /**
     * 清空队列
     */
    void clear() {
        // 线程锁定
        pthread_mutex_lock(&mutex);
        unsigned int size = q.size();
        for (int i = 0; i < size; ++i) {
            // 取出首元素
            T value = q.front();
            if (releaseCallback) {
                releaseCallback(&value);
            }
            q.pop();
        }
        // 解锁
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }

    void setSyncHandle(SyncHandle syncHandle) {
        this->syncHandle = syncHandle;
    }

    /**
     * 同步操作
     */
    void sync() {
        // 线程锁定
        pthread_mutex_lock(&mutex);
        syncHandle(q);
        // 解锁
        pthread_mutex_unlock(&mutex);
    }

};

#endif //FFMPEG_SAFE_QUEUE_H
