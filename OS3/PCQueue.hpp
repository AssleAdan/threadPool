#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"


// Single Producer - Multiple Consumer queue
template <typename T>class PCQueue
{
public:
    PCQueue(){
        this->sem=Semaphore(0);
        this->Queue=std::queue<T>();
        this->waiting_producer=false;
        pthread_mutex_init(&this->mutex, nullptr);
    }

    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop() {
        //consider priority for producer
        while (this->waiting_producer) {
            sched_yield();
        }
        sem.down();
        pthread_mutex_lock(&mutex);
        T item = this->Queue.front();
        this->Queue.pop();
        pthread_mutex_unlock(&mutex);
        return item;
    }

    // Allows for producer to enter with *minimal delay* and push items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void push(const T& item) {
        waiting_producer = true;
        pthread_mutex_lock(&this->mutex);
        this->Queue.push(item);
        this->waiting_producer = false;
        pthread_mutex_unlock(&this->mutex);
        this->sem.up();
    }

private:
    Semaphore sem;
    queue<T> Queue;
    pthread_mutex_t mutex;
    bool waiting_producer;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif