#ifndef CONSOLA_MDJ_H
#define CONSOLA_MDJ_H

#include <stdio.h>
#include <openssl/md5.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <commons/string.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <readline/history.h>


//definicion de funciones
void consolaMDJ();
void ls(char *);
void cd(char * );
void md5(char * );
void cat(char *);
void print_md5_sum(unsigned char*);
unsigned long get_size_by_fd(int );

#endif