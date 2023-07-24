//
// Created by RSP on 2023/3/25.
//

#ifndef LIBPROTOCOL_PROTOCOL_RENDERTASK_H
#define LIBPROTOCOL_PROTOCOL_RENDERTASK_H

#include "protocol-universal-rend.h"


class rendrecver {
private:
    ReleasedTaskBuffer *m_buffer=0;
public:
    rendrecver(ReleasedTaskBuffer *buffer);

    static void InitNewTaskBuffer(ReleasedTaskBuffer *buffer);
    void UpdateScreenSize(uint32_t Width,uint32_t Height);
    bool ProcessTask(TaskDispatchRoutine Dispatch);
};

class rendsender {
private:
    ReleasedTaskSenderCache *m_cache=0;
    TaskBuffer* m_task=0;
public:
    rendsender(ReleasedTaskSenderCache *cache);
    rendsender(ReleasedTaskSenderCache *cache,TaskBuffer*task);

    void Begin();
    bool PresentToTask(TaskBuffer* Task);
    bool PresentToTask(){
        if(m_task!=0){
            return PresentToTask(m_task);
        }
        return false;
    }
    void AddLine(float x1, float y1, float x2, float y2, uint32_t color);
    void AddBox(float x, float y, float w, float h, uint32_t col);
    void AddCornBox(float x, float y, float w, float h, uint32_t col);
    void AddCircle(float centerx, float centery, float rad, long pointCount, uint32_t col);
    void AddSoildCircle(float centerx, float centery, float rad, long pointCount, uint32_t col);
    void FillRect(float x, float y, float w, float h, uint32_t col);
    void AddSoildTriangle(float x1, float y1, float x2, float y2, float x3, float y3, uint32_t col);
    void AddString(float x, float y, const char* Str, uint32_t Size, uint32_t col);
    void AddStringStroke(float x, float y, const char* Str, uint32_t Size, uint32_t col, uint32_t StrokeColor);

};

#endif //LIBPROTOCOL_PROTOCOL_RENDERTASK_H
