#pragma once

#include <pthread.h>
#include <mutex>
#include <queue>
#include <memory>
#include <condition_variable>

/**
 * @brief Thread pool for data processing
 *
 * @tparam T The data type
 * @tparam Size The number of threads to spawn
 */
template <typename T, size_t Size = 1, size_t QueueMaxSize = 25>
class DataProcessingThreadPool
{
    typedef void(*ExecutorFunction)(T &, void*);

public:
    /**
     * @brief Construct a new Data Processing Thread Pool object
     *
     * @param executor The executor function that will process the data
     */
    DataProcessingThreadPool(ExecutorFunction executor, void* argument)
        : executor(executor), executorArg(argument)
    {
        for (size_t i = 0; i < Size; i++)
        {
            pthread_create(&pool[i], NULL, &DataProcessingThreadPool<T, Size>::innerExecutor, this);
        }
    }

    ~DataProcessingThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            // Empty queue
            while (dataQueue.size() > 0)
            {
                dataQueue.pop();
            }
            // Notify the threads to stop
            running = false;
            lock.unlock();
            // Awake all threads in the pool
            cv.notify_all();
        }

        // Wait for all the threads to end their life
        for (size_t i = 0; i < Size; i++)
        {
            pthread_join(pool[i], NULL);
        }
    }

    /**
     * @brief This function will copy the data into the queue for being processed by the pool
     *
     * @param data The data to add
     */
    void process(const T &data)
    {
        std::unique_lock<std::mutex> lock(mtx);
        dataQueue.emplace(std::make_unique<T>(data));
        checkQueueLimits();
        lock.unlock();
        cv.notify_one();
    }

    /**
     * @brief This function will move the data into the queue for being processed by the pool
     *
     * @param data The data to add
     */
    void process(T &&data)
    {
        std::unique_lock<std::mutex> lock(mtx);
        dataQueue.emplace(std::make_unique<T>(std::move(data)));
        checkQueueLimits();
        lock.unlock();
        cv.notify_one();
    }

    /**
     * @brief This function will add directly the data to the queue for being processed by the pool
     *
     * @param data The data to process
     */
    void process(T *data)
    {
        std::unique_lock<std::mutex> lock(mtx);
        dataQueue.emplace(std::unique_ptr<T>(data));
        checkQueueLimits();
        lock.unlock();
        cv.notify_one();
    }

    /**
     * Clears the pending data inside the queue
     */
    void clear()
    {
        std::unique_lock<std::mutex> lock(mtx);
        while(!dataQueue.empty()) {
            dataQueue.pop();
        }
    }

    static void *innerExecutor(void *arguments)
    {
        DataProcessingThreadPool<T, Size> *_this = reinterpret_cast<DataProcessingThreadPool<T, Size> *>(arguments);
        while (_this->running)
        {
            std::unique_lock<std::mutex> lock(_this->mtx);
            _this->cv.wait(lock, [&]()
                           { return _this->dataQueue.size() > 0; });
            std::unique_ptr<T> entry = std::unique_ptr<T>(std::move(_this->dataQueue.front()));
            _this->dataQueue.pop();
            lock.unlock();

            if (entry != nullptr)
            {
                _this->executor(*entry, _this->executorArg);
            }
        }

        return NULL;
    }

private:
    ExecutorFunction executor;
    void* executorArg;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::unique_ptr<T>> dataQueue;
    pthread_t pool[Size];
    bool running = true;
    
    void checkQueueLimits()
    {
        return;
        while(dataQueue.size() > QueueMaxSize) {
            dataQueue.pop();
        }
    }
};
