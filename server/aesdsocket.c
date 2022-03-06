
/*------------------------------------------------------
 * File Name: aesdsocket.c
 * Author Name: Abhishek Suryawanshi
 * Compiler:gcc
 * linker: g++
 * Debugger: gdb
 * References: 
 * 1. Linux System Programming chapter 5 (Process management) chapter 9(signals)
 -------------------------------------------------------*/



#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/queue.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <poll.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#define LOG_FILE_PATH "/var/tmp/aesdsocketdata"
#define PORT "9000"


static int sfd = -1;
static struct addrinfo  *updated_addr;
static pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
bool signal_recv = false;



typedef struct 
{
    pthread_t thread; 
    int cfd;
    struct sockaddr_in addr; 
    pthread_mutex_t* mutex;
    bool complete_status_flag;

} thread_data; 



struct slist_data_s
{
    thread_data   params;
    SLIST_ENTRY(slist_data_s) entries;
};

typedef struct slist_data_s slist_data_t;


void graceful_exit()
{

    if(sfd > -1)
    {
        shutdown(sfd, SHUT_RDWR); 

        close(sfd);
    }

    pthread_mutex_destroy(&mutex_lock);

    closelog();

}


static void signal_handler(int signo)
{
    syslog(LOG_INFO, "Signal Caught %d\n\r", signo);
    signal_recv = true;
    
  if (signo == SIGINT)
  {
    syslog(LOG_DEBUG,"Caught SIGINT!\n");
    graceful_exit();
    exit (EXIT_FAILURE);
  }
  
  else if (signo == SIGTERM)
  {
     syslog(LOG_DEBUG,"Caught SIGTERM!\n");
     graceful_exit();
     exit (EXIT_FAILURE);
  }

  else 
  {
    /* this should never happen */
    syslog(LOG_DEBUG,"Unexpected signal!\n");
    graceful_exit();
    exit (EXIT_FAILURE);
  }


}



void *timer_thread(void *args)
{
  
  int tempcount = 10; //Timer Interval
  struct tm *localTime;
  struct timespec realTime = {0, 0};
  size_t buflen;
  time_t rtime;
 
  

  while (!signal_recv)
  {
    
    if (clock_gettime(CLOCK_MONOTONIC, &realTime))
    {
      syslog(LOG_ERR, "Error: failed to get monotonic time, [%s]\n", strerror(errno));
      continue;
    }

    realTime.tv_sec += 1; 
    realTime.tv_nsec += 1000000;
    if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &realTime, NULL) != 0)
    {
     
      if (errno == EINTR)
      {
        break; 
      }

    }   


    if ((--tempcount) <= 0)
    {
        char timestamp_buf[100] = {0};
        time(&rtime);         
        localTime = localtime(&rtime); 
        buflen = strftime(timestamp_buf, 100, "timestamp:%a, %d %b %Y %T %z\n", localTime);

   
        int fd = open(LOG_FILE_PATH,O_RDWR | O_APPEND, 0644);
        
        if (fd < 0)
        {
         syslog(LOG_ERR, "Error: failed to open a file:%d\n", errno);
        }


         int retVal = pthread_mutex_lock(&mutex_lock);
        
        if(retVal)
        {
            syslog(LOG_ERR, "ERROR:Error in locking the mutex\n\r");
            close(fd);
        }
        
        lseek(fd, 0, SEEK_END); 

        int writeByteCount = write(fd, timestamp_buf, buflen);
        
        syslog( LOG_INFO, "Timestamp %s written to file\n", timestamp_buf);
   

        if (writeByteCount < 0)
        {
            syslog(LOG_ERR, "Write of timestamp failed errno %d",errno);
    
        }
        
        retVal = pthread_mutex_unlock(&mutex_lock);
        
        if(retVal)
        {
            syslog(LOG_ERR, "ERROR:Error in unlocking the mutex\n\r");
            close(fd);
        }
        
        close(fd);
        tempcount = 10;
    }

      
  }

  pthread_exit(NULL);
}




void* packetRWthread(void* thr_params)

{

    char buf[2000];

    thread_data* params = (thread_data*)thr_params;


    int readByteCounts=0;
    while(true)
    {

        readByteCounts = read(params->cfd, buf, (2000));
        
        if (readByteCounts < 0) 
        {
            syslog(LOG_ERR, "Error: reading from socket errno=%d\n", errno);
            params->complete_status_flag = true;
            pthread_exit(NULL);
        }


        if (readByteCounts == 0)
        {
            continue;
        }           

        

        if (strchr(buf, '\n')) 
        {  
           break; 

        } 

    }

    int fd = open(LOG_FILE_PATH,O_RDWR | O_APPEND, 0644);
    
    if (fd < 0)
    {
          syslog(LOG_ERR, "Error: failed to open a file:%d\n", errno);
    }

    lseek(fd, 0, SEEK_END); 
    
    int retVal = pthread_mutex_lock(params->mutex);
    
    if(retVal)
    {
        syslog(LOG_ERR, "ERROR:Error in locking the mutex\n\r");
        params->complete_status_flag = true;
        pthread_exit(NULL);
    }

    int writeByteCount = write(fd, buf, readByteCounts);
    
    if(writeByteCount < 0)
    {
        syslog(LOG_ERR, "ERROR: Writing to file error no: %d\n\r", errno);
        params->complete_status_flag = true;
        close(fd);
        pthread_exit(NULL);
    }

    lseek(fd, 0, SEEK_SET); 


    retVal = pthread_mutex_unlock(params->mutex);
    
    if(retVal)
    {
        syslog(LOG_ERR, "ERROR:Error in unlocking the mutex\n\r");
        params->complete_status_flag = true;
        pthread_exit(NULL);
    }

    close(fd);
  


    memset(buf,0, 2000);
    int read_offset_pos = 0;

    while(true) 
    {

        int fd = open(LOG_FILE_PATH, O_RDWR | O_APPEND, 0644);
        
        if(fd < 0)
        {
            syslog(LOG_ERR, "Error: failed to open a file:%d\n", errno);
            continue; 
        }

        lseek(fd, read_offset_pos, SEEK_SET);


        int retVal = pthread_mutex_lock(params->mutex);
        
        if(retVal)
        {
            syslog(LOG_ERR, "ERROR:Error in locking the mutex\n\r");
            params->complete_status_flag = true;
            pthread_exit(NULL);
        }
        
        int readByteCounts = read(fd, buf, 2000);

        retVal = pthread_mutex_unlock(params->mutex);   
        
        if(retVal)
        {
            syslog(LOG_ERR, "ERROR:Error in locking the mutex\n\r");
            params->complete_status_flag = true;
            pthread_exit(NULL);
        }
        
        close(fd);
        
        if(readByteCounts < 0)
        {
            syslog(LOG_ERR, "Error: failed to read from file:%d\n", errno);
            continue;
        }

        if(readByteCounts == 0)
        {
            break;
        }
        
        int writeByteCount = write(params->cfd, buf, readByteCounts );
    

        if(writeByteCount < 0)
        {
            syslog(LOG_ERR, "failed to write on client fd:%d\n", errno);
            continue;
        }

        read_offset_pos += writeByteCount;

    }

    params->complete_status_flag = true;
    pthread_exit(NULL);
}





int main(int argc, char **argv) 
{

     openlog(NULL, 0, LOG_USER);

// Register signal_handler as our signal handler for SIGINT
if (signal (SIGINT, signal_handler) == SIG_ERR) 
{
   syslog(LOG_ERR, "Cannot handle SIGINT!\n");
   graceful_exit();
   exit (EXIT_FAILURE);
}

//Register signal_handler as our signal handler for SIGTERM.

if (signal (SIGTERM, signal_handler) == SIG_ERR) 
{
 syslog(LOG_ERR, "Cannot handle SIGTERM!\n");
  graceful_exit();
 exit (EXIT_FAILURE);

}

    
    bool createDaemon = false;
   
   //check if -d  argument is present or not if present then create the deamon
    if (argc == 2) 
    {
        if (!strcmp(argv[1], "-d")) 
        {
            createDaemon = true;
        } 

        else 
        {
           syslog(LOG_ERR, "incorrect argument:%s", argv[1]);
            return (-1);
        }
    }
    
    int write_fd = creat(LOG_FILE_PATH, 0644);

    if(write_fd < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to create file %d", errno);
        graceful_exit();
        exit(1);
    }
    close(write_fd);
    
    //Initialize the linked list
    slist_data_t *datap = NULL;
    SLIST_HEAD(slisthead, slist_data_s) head;
    SLIST_INIT(&head);

    struct addrinfo addr_hints;
    memset(&addr_hints, 0, sizeof(addr_hints));
    addr_hints.ai_family = AF_INET;
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_flags = AI_PASSIVE;


    int res = getaddrinfo(NULL, (PORT), &addr_hints, &updated_addr);
     
    if (res != 0) 
    {
        syslog(LOG_ERR, "Error:getaddrinfo() %s\n", gai_strerror(res));
        graceful_exit();
        return -1;
    }

    //create socket for communication
    sfd = socket(updated_addr->ai_family, SOCK_STREAM, 0);

    if (sfd < 0) 
    {
        syslog(LOG_ERR, "ERROR : Socket creation failed:%d\n", errno);
        graceful_exit();
        return -1;
    }
     
    //set scoket option
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
    {
        syslog(LOG_ERR, "ERROR: setsockopt failed with errono:%d\n", errno);
        graceful_exit();
        return -1;
    }

     //bind the socket
    if (bind(sfd, updated_addr->ai_addr, updated_addr->ai_addrlen) < 0)
    {
        syslog(LOG_ERR, "ERROR : failed to bind:%d\n", errno);;
        graceful_exit();
        return -1;
    }

    freeaddrinfo(updated_addr);
     
     
    if (listen(sfd, 10)) 
    {
       syslog(LOG_ERR, "ERROR:failed to listen %d\n", errno);
       graceful_exit();
        return -1;
    }


    printf("waiting for client connections\n\r");


    if (createDaemon == true) 
    {
        
         int retVal_val = daemon(0,0);

         if(retVal_val<0)
         {
            syslog(LOG_ERR,"ERROR: Failed to create daemon");
            graceful_exit();
            return -1;

         } 
    }


    pthread_t time_id; 
    pthread_create(&time_id, NULL, timer_thread, NULL);
 

    while(!(signal_recv)) 
    {

        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);


        int cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if(signal_recv)
        {
            break;
        }

        if(cfd < 0)
        {
            syslog(LOG_ERR, "ERROR: failed to accept the connection: %s", strerror(errno));
            graceful_exit();
            return -1;
        }

        char client_info[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), client_info, INET_ADDRSTRLEN);
        syslog(LOG_INFO, "Connection Accepted%s \n\r",client_info);
  
       
        datap = (slist_data_t *) malloc(sizeof(slist_data_t));
        
        SLIST_INSERT_HEAD(&head,datap,entries);

        datap->params.cfd = cfd;
        datap->params.addr = clientAddr;
        datap->params.mutex = &mutex_lock;
        datap->params.complete_status_flag = false;

        pthread_create(&(datap->params.thread), NULL, packetRWthread, (void*)&datap->params);

        SLIST_FOREACH(datap,&head,entries)
        {
            
            if(datap->params.complete_status_flag == true)
            {
                pthread_join(datap->params.thread,NULL);
                
                shutdown(datap->params.cfd, SHUT_RDWR);
                
                close(datap->params.cfd);
                
                syslog(LOG_INFO, "Join thread:%d\n\r",(int)datap->params.thread);
         

            }
        }



    }

    pthread_join(time_id, NULL);

    while (!SLIST_EMPTY(&head)) 
    {
        datap = SLIST_FIRST(&head);
        SLIST_REMOVE_HEAD(&head, entries);
        free(datap);
        datap = NULL;
    }
    
  
 
    if (access(LOG_FILE_PATH, F_OK) == 0) 
    {
       remove(LOG_FILE_PATH);
    }
    graceful_exit();

    exit(0);

}



