/*------------------------------------------------------
 * File Name: aesdsocket.c
 * Author Name: Abhishek Suryawanshi
 * Compiler:gcc
 * linker: g++
 * Debugger: gdb
 * References: 
 * 1. Linux System Programming chapter 5 (Process management) chapter 9(signals)
 * 2. 
 -------------------------------------------------------*/

#include <stdarg.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>

//macros
#define LOG_FILE_PATH "/var/tmp/aesdsocketdata"
#define PORT "9000"

//global variable
int cfd = -1; 
int sfd = -1;
struct addrinfo  *updated_addr;




void graceful_exit()
{

    if(cfd > -1)
    {
        shutdown(cfd, SHUT_RDWR);
        close(cfd);
    }

    if(sfd > -1)
    {
        shutdown(sfd, SHUT_RDWR); 
        close(sfd);
    }

    if(updated_addr != NULL)
    {
        freeaddrinfo(updated_addr);
    }

    remove(LOG_FILE_PATH);
    closelog();

}






static void signal_handler (int signo)
{
  

  syslog(LOG_INFO, "Signal Caught %d\n\r", signo);

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


    bool createDeamon = false;
   
    if (argc == 2) 
    {
        if (!strcmp(argv[1], "-d")) 
        {
            createDeamon = true;
        } 

        else 
        {
           syslog(LOG_ERR, "incorrect argument:%s", argv[1]);
            return (-1);
        }
    }
   

    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int res = getaddrinfo(NULL, (PORT), &hints, &updated_addr);
    
    if (res != 0) 
    {
        syslog(LOG_ERR, "Error:getaddrinfo() %s\n", gai_strerror(res));
        graceful_exit();
        return -1;
    }


    sfd = socket(updated_addr->ai_family, SOCK_STREAM, 0);
    if (sfd < 0) 
    {
        syslog(LOG_ERR, "ERROR : Socket creation failed:%d\n", errno);
        graceful_exit();
        return -1;
    }

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
    {
        syslog(LOG_ERR, "ERROR: setsockopt failed with errono:%d\n", errno);
        graceful_exit();
        return -1;
    }


    if (bind(sfd, updated_addr->ai_addr, updated_addr->ai_addrlen) < 0) 
    {
       syslog(LOG_ERR, "ERROR : failed to bind:%d\n", errno);;
        graceful_exit();
        return -1;
    }


   
    if (listen(sfd, 20)) 
    {
       syslog(LOG_ERR, "ERROR:failed to listen %d\n", errno);
       graceful_exit();
        return -1;
    }

  

    if (createDeamon == true) 
    {
         int ret_val = daemon(0,0);

         if(ret_val<0)
         {
            syslog(LOG_ERR,"ERROR: Failed to create daemon");
            graceful_exit();
            return -1;

         }    
    }


    while(true) 
    {

        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);

        cfd = accept(sfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if(cfd < 0)
        {
            syslog(LOG_ERR, "ERROR: failed to accept: %s", strerror(errno));
            graceful_exit();
            return -1;
        }

        char client_ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(clientAddr.sin_addr), client_ip, INET_ADDRSTRLEN);

        syslog(LOG_INFO, "connection successfully accepted%s \n\r",client_ip);
        
        
        char recv_buf[2000];

        while(true)
        {
            int readByteCounts = read(cfd, recv_buf, (2000));
            if (readByteCounts < 0) 
            {
                syslog(LOG_ERR, "Error: socket read failure: %d\n", errno);
                continue; 
            }

            if (readByteCounts == 0)
            {
                continue; 
            }

        
            int fd = open(LOG_FILE_PATH,O_RDWR | O_CREAT | O_APPEND, 0644);

            if (fd < 0)
            {
                syslog(LOG_ERR, "failed to openfile:%d\n\r", errno);
            }

            lseek(fd, 0, SEEK_END);

            int writeByteCount = write(fd, &recv_buf[0], readByteCounts);

            if(writeByteCount < 0)
            {
               syslog(LOG_ERR, "Error: failed to write:%d\n\r", errno);
                close(fd);
                continue; 
            }

            close(fd);
           


            if (strchr(&recv_buf[0], '\n')) 
            {  

               break; 

            } 
           

        }
       
        int offset_pos = 0;

        while(1) 
        {

            int fd = open(LOG_FILE_PATH, O_RDWR | O_CREAT | O_APPEND, 0644);
            if(fd < 0)
            {
                 syslog(LOG_ERR, "Error: failed to open a file:%d\n", errno);
                 continue; 
            }

            lseek(fd, offset_pos, SEEK_SET);

            int readByteCounts = read(fd, recv_buf, (2000));
            
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

            int writeByteCount = write(cfd, &recv_buf[0], readByteCounts );
        

            if(writeByteCount < 0)
            {
               
               syslog(LOG_ERR, "failed to write on client fd:%d\n", errno);
                continue;
            }


            offset_pos += writeByteCount;

        }

        close(cfd);
        cfd = -1;
        syslog(LOG_INFO, "closing client socket\n\r");
    }

    graceful_exit();
    return 0;

}