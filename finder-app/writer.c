/*----------------------------------------------
 * File Name: writer.c
 * Author Name: Abhishek Suryawanshi
 * Compiler:gcc
 * linker: g++
 * Debugger: gdb
 * References: Chapter 2 Linux System Programming by Robert Love
 ------------------------------------------------*/

/*includes*/
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<syslog.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

/*Macros*/
#define TOTAL_ARGC 3


/* function: main 
 * -----------------------------------------------------------
 * Open file and update it with user input string 
 *
 * Parameters:
 *   int   arg   : Total number of argument
 *   char* agrv[]: Argument value ( filepath & writesr) 
 *
 * Returns:
 *   return 0 if open & write is successful or 1 for failure 
 ------------------------------------------------------------*/

int main(int argc, char* argv[])
{

	int fd;
	ssize_t nr;
   /*Initialize the syslog daemon*/
	openlog(NULL, 0, LOG_USER);

	int status = 0;
  
    /*check if toltal number arguments is 3  i.e ./write.c, filepath & writestr*/
	if(argc!=TOTAL_ARGC)
	{
		syslog(LOG_ERR, "\n\r Enter correct number of argument: 1. filepath/filename 2. String to be written\n\r");
		status = 1;
	}
  
   /*if number of arguments are correct then open the file*/
	if(status==0) 
	{ 
       /*store argv[1] i.e second argument in filepath variable*/
		char* filepath = argv[1];
       /*store argv[2] i.e third argument in  writestr variable*/
		char* writestr = argv[2];
        
	   /*open the file with Read-write access and with permission 0664*/	
		fd=open(filepath,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
       
	   
		if(fd==-1)
		{
			syslog(LOG_ERR,"\n\r Error:cannot open or create file:%s with error number:%d\n\r",filepath,errno);
			status = 1;
		}
        /*if file is successfully open then write the user input string*/ 
		if(status==0)
		{ 
			syslog(LOG_DEBUG,"\n\r File:%s successfully is open/created\n\r", filepath);

			nr=write(fd,writestr,strlen(writestr));

			if(nr==-1)
			{
				syslog(LOG_ERR,"\n\r file:%s cannot be updated with given string:%s with error number:%d\n\r",filepath,writestr,errno);
				status = 1;
			}

           else if(nr!=strlen(writestr))
           {
              
              syslog(LOG_ERR,"\n\r file:%s  partially updated & process exit with error number:%d\n\r",filepath,errno);

              status = 1;
           }

           else if(nr==strlen(writestr))
			{
               syslog(LOG_DEBUG,"\n\r file:%s is successfully updated with given string:%s",filepath,writestr);
			}

           /*close the file*/       
			if(!close(fd))
			{
				syslog(LOG_DEBUG,"\n\r file:%s file is successfully closed\n\r",filepath);
			}
		}
	}

	closelog();
	return status;
}