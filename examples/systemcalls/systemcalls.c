
/*----------------------------------------------------------------------------------------------
 * File Name: systemcalls.c
 * Author Name: Abhishek Suryawanshi
 * Compiler:gcc
 * linker: g++
 * Debugger: gdb
 * References: 1. Chapter 27 Linux Programming interface
 *             2. https://www.cs.utexas.edu/~theksong/2020/243/Using-dup2-to-redirect-output/
 -----------------------------------------------------------------------------------------------*/



#include "systemcalls.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



/**
 * @param cmd the command to execute with system()
 * @return true if the commands in ... with arguments @param arguments were executed 
 *   successfully using the system() call, false if an error occurred, 
 *   either in invocation of the system() command, or if a non-zero return 
 *   value was returned by the command issued in @param.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success 
 *   or false() if it returned a failure
*/
     /*Status return from system()*/
     int status ;  
     
     status = system(cmd);
     
     printf("system() returned: status=0x%04x (%d,%d)\n",(unsigned int) status, status>>8, status&0xff);

    
     if(status==-1)
     {
     
        return false;
     }
     
     else
     {
     
         if(WIFEXITED(status)&&WEXITSTATUS(status)==127)
         {
          
          printf("(Probably) could not invoke shell\n");
          
          return false; 
         }
         
         if (WIFEXITED(status)&&WEXITSTATUS (status) != 0)
         { 
         
         return false; 
         }
         
         if((cmd==NULL)&&WEXITSTATUS (status) == 0){

            return false;
         }
          
         return true;
      }
     

  
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the 
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    int status;
    pid_t pid;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *   
*/
     pid= fork();

     if(pid==-1)
     {

        return false;
     }

     else if(pid==0)
     {

        execv(command[0], command);
        perror("execv");
        exit(-1);
        return false;
     }

     else if(pid > 0)
     {
   
        if(waitpid(pid, &status,0)== -1)
        { 
            return false;
        }

        else if (WIFEXITED(status))
        {
            if(WEXITSTATUS(status)!=0)
                return false;
        }
     }


    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.  
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int status;
    pid_t pid;
    int fd;

    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *   
*/
    if(outputfile!=NULL)
    {
        fd = open(outputfile,O_WRONLY|O_TRUNC|O_CREAT,0644);
        if(fd<0)
        {
            perror("open");
            return false;
        }

    }

    pid = fork();

    if(pid==-1)
    {

        return false;
    }

    else if(pid == 0)
    {
       
       if(dup2(fd,STDOUT_FILENO)<0)
       {
          perror("dup2");
          return false;
       }

       close(fd);

        execv(command[0], command);
        perror("execv");
        exit (-1);
        return false; 

    }


    else if(pid > 0)
     {
           close(fd);

        if(waitpid(pid, &status,0)== -1)
        { 
            return false;
        }

        else if (WIFEXITED(status))
        {
            if(WEXITSTATUS(status)!=0)
                {
                    return false;
                }
        }
     }


    va_end(args);
    
    return true;
}
