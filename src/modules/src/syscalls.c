#include "syscalls.h"

extern char _end;  // Defined by linker script
static char *heap_end;

char *_sbrk(int incr) {
    if (heap_end == 0) {
        heap_end = &_end;
    }
    char *prev_heap_end = heap_end;
    heap_end += incr;
    // You may want to check for stack/heap collision here
    return (char *)prev_heap_end;
}

int _write(int file, char *ptr, int len) {
    // Implement writing to UART or just discard
    // For example, loop over ptr and send bytes over UART
    return len;  // pretend we wrote all bytes
}

int _read(int file, char *ptr, int len) {
    // Implement reading from UART or return 0 for EOF
    return 0;
}

int _close(int file) { return -1; }
int _fstat(int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
int _isatty(int file) { return 1; }
int _lseek(int file, int ptr, int dir) { return 0; }
