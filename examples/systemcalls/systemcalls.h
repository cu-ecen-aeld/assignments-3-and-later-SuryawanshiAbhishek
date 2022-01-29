/*----------------------------------------------------------------------------------------------
 * File Name: systemcalls.h
 * Author Name: Abhishek Suryawanshi
 * Compiler:gcc
 * linker: g++
 * Debugger: gdb
 * References: 1. Chapter 27 Linux Programming interface
 *             2. https://www.cs.utexas.edu/~theksong/2020/243/Using-dup2-to-redirect-output/
 -----------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

bool do_system(const char *command);

bool do_exec(int count, ...);

bool do_exec_redirect(const char *outputfile, int count, ...);
