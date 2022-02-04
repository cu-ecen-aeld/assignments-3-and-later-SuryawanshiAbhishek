#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    struct thread_data* thread_func_arg = (struct thread_data*)thread_param;
    bool thread_status =true;
    
    int result = usleep((thread_func_arg->wait_to_obtain_ms)*1000);
    if(result!=0)
    {
       ERROR_LOG("\n\rfailed to wait\n\r");
       thread_status=false; 
    }
    
    result=pthread_mutex_lock(thread_func_arg->lock);
    if(result!=0)
    {
       ERROR_LOG("\n\rRequired Mutex lock is failed\n\r");
       thread_status=false;
    }
    
     result = usleep((thread_func_arg->wait_to_release_ms)*1000);
     if(result!=0)
    {
       ERROR_LOG("\n\rfailed to wait\n\r");
       thread_status=false;
    }
    
    result = pthread_mutex_unlock(thread_func_arg->lock);
     if(result!=0)
    {
       ERROR_LOG("\n\rfailed to wait\n\r");
       thread_status=false;
    }
    
    
    thread_func_arg->thread_complete_success=thread_status;       
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     * 
     * See implementation details in threading.h file comment block
     */
     
     
     
     struct thread_data* parameters = malloc(sizeof(struct thread_data));
     
     parameters->thread = thread;
     parameters->lock = mutex;
     parameters->wait_to_obtain_ms = wait_to_obtain_ms;
     parameters->wait_to_release_ms = wait_to_release_ms;
     parameters->thread_complete_success = false;

    int result = pthread_create(thread, NULL, &threadfunc, (void *)parameters);
    
    if (result != 0)
    {
    
        ERROR_LOG("Thread creation failed");
    	return false;
    }
    else
    {
        DEBUG_LOG("Thread successfully created");
    	return true;
    }     
     
}

