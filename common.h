#ifndef __COMMON_H__
#define __COMMON_H__

#define LOG(message) fputs((message), stderr)
#define LOGF(format, ...) fprintf(stderr, (format), __VA_ARGS__)

#endif // __COMMON_H__
