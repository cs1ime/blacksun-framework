#pragma once
#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#define xs
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <syscall.h>
#include "forceinline.h"


namespace util{
	__forceinline uint64_t RAND() {
		int fd = open(xs("/dev/urandom"), O_RDONLY);
		if (fd == -1) {
			return 0;
		}
		uint64_t result = 0;
		read(fd, &result, 8);
		close(fd);
		return result;
	}
    __forceinline uint64_t RAND(uint64_t min,uint64_t max) {
        uint64_t interval=max-min;
        return (RAND()%interval)+min;
    }
	__forceinline int msleep(long msec)
    {
        struct timespec ts;
        int res;

        if (msec < 0)
        {
            errno = EINVAL;
            return -1;
        }

        ts.tv_sec = msec / 1000;
        ts.tv_nsec = (msec % 1000) * 1000000;

        do {
            res = nanosleep(&ts, &ts);
        } while (res && errno == EINTR);

        return res;
    }
    __forceinline uint64_t GetTickCount() {
        long long tmp;
        struct timeval tv;

        syscall(SYS_gettimeofday, &tv, NULL);
        //gettimeofday(&tv, NULL);
        tmp = tv.tv_sec;
        tmp = tmp * 1000;
        tmp = tmp + (tv.tv_usec / 1000);

        return (uint64_t)tmp;
    }
}




#endif // !_UTIL_H_


