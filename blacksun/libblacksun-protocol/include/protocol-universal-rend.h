//
// Created by RSP on 2023/3/25.
//

#ifndef LIBPROTOCOL_PROTOCOL_UNIVERSAL_REND_H
#define LIBPROTOCOL_PROTOCOL_UNIVERSAL_REND_H

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#define DRAW_BUFFER_NEED_SIZE (0x100000)
#define ELEMENT_STRING_MAX_LENGTH (260)
#define ELEMENT_BLOCK_MAX_SIZE (300)

struct TaskShareData {
    uint32_t PlaneWidth;
    uint32_t PlaneHeight;
};
struct TaskBuffer {
    uint32_t Magic;
    TaskShareData Share;
    bool have_render_data;
    bool have_sec_render_data;
    pthread_mutex_t lock;
    uint32_t ele_count;
    uint32_t SizeOfBuffer;
    uint8_t Data;
};
struct ReleasedTaskBuffer {
    TaskBuffer Task;
    uint8_t DrawBuffer[DRAW_BUFFER_NEED_SIZE];
    uint32_t Second_ele_count;
    uint8_t SecondDrawBuffer[DRAW_BUFFER_NEED_SIZE];
};
struct TaskSenderCache {
    uint32_t ele_count;
    uint32_t SizeOfBuffer;
    uint8_t Data;
};
struct ReleasedTaskSenderCache {
    TaskSenderCache Cache;
    uint8_t DrawBuffer[DRAW_BUFFER_NEED_SIZE];
};

#define TaskElementHeaderSize ((long)&((TaskElement *)0)->Data)

struct TaskElement {
    uint32_t SizeOfBlock;
    uint32_t Type;
    uint8_t Data;
};

enum ElementType {
    ELEMENT_LINE = 1,
    ELEMENT_CIRCLE,
    ELEMENT_STRING,
    ELEMENT_STROKESTRING,
    ELEMENT_SOILDRECT,
    ELEMENT_SOILDTRIANGLE,
    ELEMENT_SOILDCIRCLE,

};
struct ElementLine {
    float x1;
    float y1;
    float x2;
    float y2;
    uint32_t col;
};
struct ElementCircle {
    float centerx;
    float centery;
    float rad;
    long pointCount;
    uint32_t col;
};
struct ElementString {
    float x;
    float y;
    uint32_t size;
    uint32_t col;
    uint8_t str;
};
struct ElementStrokeString {
    float x;
    float y;
    uint32_t size;
    uint32_t col;
    uint32_t StrokeCol;
    uint8_t str;
};
struct ElementSoildRect {
    float x;
    float y;
    float w;
    float h;
    uint32_t col;
};
struct ElementSoildTriangle {
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    uint32_t col;
};
struct ElementSoildCircle {
    float centerx;
    float centery;
    float rad;
    long pointCount;
    uint32_t col;
};

typedef void(*TaskDispatchRoutine)(TaskElement*);

#endif //LIBPROTOCOL_PROTOCOL_UNIVERSAL_REND_H
