#include "tiny-lib.h"
#include "tiny-printf.h"

// syscall function
asm
(
	".globl syscall\n"
	"syscall:\n"
	"mov %edi,%eax\n"
	"mov %rsi,%rdi\n"
	"mov %rdx,%rsi\n"
	"mov %rcx,%rdx\n"
	"mov %r8,%r10\n"
	"mov %r9,%r8\n"
	"mov 0x8(%rsp),%r9\n"
	"syscall\n"
	"retq\n"
);

void __exit(int status) 
{
	syscall(SYS_exit, status);
}

void _fatal(const char * msg)
{
	printf("FATAL ERROR: %s\n", msg);
	__exit(1);
}

size_t _read(int fd, void *buf, size_t count)
{
	return syscall(SYS_read, fd, buf, count);
}

size_t _write(int fd, const void *buf, size_t count)
{
	return syscall(SYS_write, fd, buf, count);
}

int _close(int fd)
{
	return syscall(SYS_close, fd);
}

void *_shmat(int shmid, const void *shmaddr, int shmflag)
{
	return (void *)syscall(SYS_shmat, shmid, shmaddr, shmflag);
}

size_t _fork(void)
{
	return syscall(SYS_fork);
}

size_t _waitpid(size_t pid, int *wstatus, int options)
{
	return syscall(SYS_wait4, pid, wstatus, options, 0);
}


int _kill(size_t pid, int sig)
{
	return syscall(SYS_kill, pid, sig);
}

void *_mmap(void *addr, size_t length, int prot, int flags,
               int fd, unsigned int offset)
{
	return (void *)syscall(SYS_mmap, addr, length, prot, flags, fd, offset);
}

int _munmap(void *addr, size_t length)
{
	return syscall(SYS_munmap, addr, length);
}


// implementing putchar for supporting printf
// note that the float point number operations (PRINTF_DISABLE_SUPPORT_FLOAT)
// have been disabled. Please check mini-printf.h
void _putchar(char c)
{
	_write(1, &c, 1);
}

int _memcmp(char *a, char *b, long n) {
	while(n) if(*a++ != *b++) return a[-1] - b[-1];
	return 0;
}

int _bcmp(char *a, char *b, long n) {
	return memcmp(a, b, n);
}

void *_memcpy(char *a, char *b, long n) {
	void *dest = b;
	while(n) *a++ = *b++;
	return dest;
}

void *_memset(char *a, int c, long n) {
	void *dst = a;
	while(n) *a++ = c;
	return dst;
}