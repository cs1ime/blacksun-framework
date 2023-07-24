#pragma once

#ifndef _PRINT_CONTROL_H_
#define _PRINT_CONTROL_H_

//#define DISABLE_PRINT

#define pt printf

#define p1x(v1)printf(xs(""#v1"=%08llX\r\n"),v1)
#define p1d(v1)printf(xs(""#v1"=%08lld\r\n"),v1)


#ifdef DISABLE_PRINT
    #define printf
    #define DbgPrint
    #define _ptb
    #define pt
    #define p1x
    #define p1d
    #define puts
#endif

#endif
