#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

caddr_t _sbrk(int incr);
int _write(int file, char *ptr, int len);
int _read(int file, char *ptr, int len);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
