/* Includes */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

/* Variables */
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));
extern int _end;

char *__env[1] = {0};
char **environ = __env;

/* Functions */
void initialise_monitor_handles() {}

int _getpid(void) { return 1; }

int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}

void _exit(int status) {
  _kill(status, -1);
  while (1) {
  } /* Make sure we hang here */
}

__attribute__((weak)) int _read(int file, char *ptr, int len) {

  /** Default Implementation.
  ** May run into a scanf reads that wait for the whole buffer to fill.
  ** If we use this, we need to do `setvbuf(stdin, NULL, _IONBF, 0)`, for
  ** scanf to work properly.
  **/
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++) {
    *ptr++ = __io_getchar();
  }
  return len;

  /**
  ** Alternate Implementaion.
  ** Removes the need to fill the whole buffer. If we do this, we do not
  ** need to do the `setvbuf` call like above.
  **/

  /*
  (void)file;
  (void)len;

  *ptr = __io_getchar();

  return 1;
  */
}

__attribute__((weak)) int _write(int file, char *ptr, int len) {
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++) {
    __io_putchar(*ptr++);
  }
  return len;
}

int _close(int file) {
  (void)file;
  return -1;
}

int _fstat(int file, struct stat *st) {
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty(int file) {
  (void)file;
  return 1;
}

int _lseek(int file, int ptr, int dir) {
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}

int _open(char *path, int flags, ...) {
  (void)path;
  (void)flags;
  /* Pretend like we always fail */
  return -1;
}

int _wait(int *status) {
  (void)status;
  errno = ECHILD;
  return -1;
}

int _unlink(char *name) {
  (void)name;
  errno = ENOENT;
  return -1;
}

int _times(struct tms *buf) {
  (void)buf;
  return -1;
}

int _stat(char *file, struct stat *st) {
  (void)file;
  st->st_mode = S_IFCHR;
  return 0;
}

int _link(char *old, char *new) {
  (void)old;
  (void)new;
  errno = EMLINK;
  return -1;
}

int _fork(void) {
  errno = EAGAIN;
  return -1;
}

int _execve(char *name, char **argv, char **env) {
  (void)name;
  (void)argv;
  (void)env;
  errno = ENOMEM;
  return -1;
}

/* This is some simplified logic for heap allocation */
/* If previously unassigned, heap will point to the end of our .stack section in
 * RAM */
/* And it will increase/decrease based on when and how this syscall is made */
/* For more conventional ways of handling the stack, we use the traditional way
 */
/* Which is to put the stack at the end of SRAM */
/* For our minimal examples, we can forego this */
void *_sbrk(int incr) {
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&_end;
  }
  prev_heap = heap;

  heap += incr;

  return prev_heap;
}
