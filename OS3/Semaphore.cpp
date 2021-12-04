//
// Created by student on 12/31/20.
//

#include "Semaphore.hpp"

Semaphore::Semaphore() {
    pthread_mutex_init(&mutex,nullptr);
    pthread_cond_init(&cond,nullptr);
    threads_num = 0;

}

Semaphore::Semaphore(unsigned int val) {
    pthread_mutex_init(&mutex,nullptr);
    pthread_cond_init(&cond,nullptr);
    threads_num = val;


}

void Semaphore::up() {
    pthread_mutex_lock(&mutex);
    threads_num++;
    if(threads_num > 0 )
        pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void Semaphore::down() {
    pthread_mutex_lock(&mutex);
    while ( threads_num <= 0 ){
        pthread_cond_wait(&cond,&mutex);
    }
    threads_num--;
    pthread_mutex_unlock(&mutex);
}

Semaphore::~Semaphore() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}


