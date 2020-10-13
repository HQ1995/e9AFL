#ifndef __TINY_LIB_H
#define __TINY_LIB_H

#include <stdint.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <syscall.h>


// syscalls
#define exit     __exit
#define _exit    __exit
void    __exit(int status) __attribute__ ((noreturn));
void _fatal(const char * msg) __attribute__ ((noreturn));

#define read    _read
size_t  _read(int fd, void *buf, size_t count);

#define write   _write
size_t  _write(int fd, const void *buf, size_t count);

#define close   _close
int     _close(int fd);

#define shmat   _shmat
void    *_shmat(int shmid, const void *shmaddr, int shmflag);

#define fork    _fork
size_t  _fork(void);

size_t  _waitpid(size_t pid, int *wstatus, int options);
int     _kill(size_t pid, int sig);
void    *_mmap(void *addr, size_t length, int prot, int flags,
               int fd, unsigned int offset);
int _munmap(void *addr, size_t length);


// libc replacements
void    _putchar(char c);

#define memcmp  _memcmp
int     _memcmp(char *a, char *b, long n);

#define bcmp    _bcmp
int     _bcmp(char *a, char *b, long n);

#define memcpy  _memcpy
void    *_memcpy(char *a, char *b, long n);

#define memset  _memset
void    *_memset(char *a, int c, long n);

#endif
