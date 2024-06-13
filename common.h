#ifndef __COMMON_H__
#define __COMMON_H__

#define PRINT(message) fputs((message), stdout)
#define PRINTF(format, ...) fprintf(stdout, (format), __VA_ARGS__)
#define PRINTC(ch) fputc((ch), stdout)

#define LOG(message) fputs((message), stderr)
#define LOGF(format, ...) fprintf(stderr, (format), __VA_ARGS__)
#define LOGC(ch) fputc((ch), stderr)

#endif // __COMMON_H__
