#ifndef __THREAD_H
#define __THREAD_H

#include "Headers.hpp"

class Game;


class Thread
{
public:
	Thread(uint thread_id) 
	{
		// Only places thread_id
		this->m_thread_id=thread_id;
	} 
	virtual ~Thread() {} // Does nothing 

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	// Creates the internal thread via pthread_create 
	bool start()
	{
        if(pthread_create(&m_thread,NULL,entry_func , this )==0)
            return true;
        return false;
	}

	/** Will not return until the internal thread has exited. */
	void join()
	{
	    pthread_join(this->m_thread, nullptr);
	}

	/** Returns the thread_id **/
	uint thread_id()
	{
		return m_thread_id;
	}

    void setFinished() {
        this->finished = true;
    }
    bool getFinished() {
        return this->finished;
    }

protected:
	// Implement this method in your subclass with the code you want your thread to run. 
	virtual void thread_workload() = 0;
	uint m_thread_id; // A number from 0 -> Number of threads initialized, providing a simple numbering for you to use

private:
	static void * entry_func(void * thread) { ((Thread *)thread)->thread_workload(); return NULL; }
	pthread_t m_thread;
	bool finished =false;
};

class Worker: public Thread{
public:
    Game* game;

    Worker(int thread_id, Game* game);
    int helper(uint i,uint j,int_mat* mat);
    int find_dominant(uint i,uint j);
    int FindAverageAlive(uint i,uint j);

~Worker();
protected:
    void thread_workload() override;

};



#endif
