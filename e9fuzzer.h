#ifndef __E9FUZZER_H
#define __E9FUZZER_H

#include <stdint.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <syscall.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>


#define FORKSRV_FD          198

#define MAP_SIZE_POW2		16
#define MAP_SIZE 			(1 << MAP_SIZE_POW2)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  s32;

void e9_maybe_log(intptr_t addr, intptr_t next, intptr_t base);

void init(int argc, char **argv, char **envp);

#endif
