#include <stdio.h>

#ifdef DEBUG
#define debug_perror(string) perror(string);
#define debug_printf(...) printf(__VA_ARGS__);
#define debug_fprintf(stream, ...) fprintf(stream,  __VA_ARGS__);
#else
#define debug_perror(string)
#define debug_printf(...)
#define debug_fprintf(stream, ...)
#endif
