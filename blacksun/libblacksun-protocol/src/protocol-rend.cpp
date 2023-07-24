//
// Created by RSP on 2023/3/25.
//

#include "AsyncRenderTask.h"
#include <protocol-rend.h>

rendrecver::rendrecver(ReleasedTaskBuffer *buffer) {
    m_buffer=buffer;
}
void rendrecver::InitNewTaskBuffer(ReleasedTaskBuffer *buffer) {
    memset(buffer,0,sizeof(ReleasedTaskBuffer));
    TaskRecv::InitNewTaskBuffer(&buffer->Task);
}
void rendrecver::UpdateScreenSize(uint32_t Width, uint32_t Height) {
    TaskShareData shdat;
    shdat.PlaneHeight=Height;
    shdat.PlaneWidth=Width;
    TaskRecv::UpdateShareData(&m_buffer->Task,shdat);
}
bool rendrecver::ProcessTask(TaskDispatchRoutine Dispatch){
    return TaskRecv::ProcessTask(&m_buffer->Task,Dispatch);
}

rendsender::rendsender(ReleasedTaskSenderCache *cache, TaskBuffer *task) {
    m_cache=cache;
    m_task=task;
}
rendsender::rendsender(ReleasedTaskSenderCache *cache) {
    m_cache=cache;
}
void rendsender::Begin() {
    TaskSender::Begin(&m_cache->Cache);
}
bool rendsender::PresentToTask(TaskBuffer *Task) {
    return TaskSender::PresentToTask(Task,&m_cache->Cache);
}
void rendsender::AddLine(float x1, float y1, float x2, float y2, uint32_t color) {
    TaskSender::AddLine(&m_cache->Cache,x1,y1,x2,y2,color);
}
void rendsender::AddBox(float x, float y, float w, float h, uint32_t col){
    TaskSender::AddBox(&m_cache->Cache,x,y,w,h,col);
}
void rendsender::AddCornBox(float x, float y, float w, float h, uint32_t col){
    TaskSender::AddCornBox(&m_cache->Cache,x,y,w,h,col);
}
void rendsender::AddCircle(float centerx, float centery, float rad, long pointCount, uint32_t col){
    TaskSender::AddCircle(&m_cache->Cache,centerx,centery,rad,pointCount,col);
}
void rendsender::AddSoildCircle(float centerx, float centery, float rad, long pointCount, uint32_t col){
    TaskSender::AddSoildCircle(&m_cache->Cache,centerx,centery,rad,pointCount,col);
}
void rendsender::FillRect(float x, float y, float w, float h, uint32_t col){
    TaskSender::FillRect(&m_cache->Cache,x,y,w,h,col);
}
void rendsender::AddSoildTriangle(float x1, float y1, float x2, float y2, float x3, float y3, uint32_t col){
    TaskSender::AddSoildTriangle(&m_cache->Cache,x1,y1,x2,y2,x3,y3,col);
}
void rendsender::AddString(float x, float y, const char* Str, uint32_t Size, uint32_t col){
    TaskSender::AddString(&m_cache->Cache,x,y,Str,Size,col);
}
void rendsender::AddStringStroke(float x, float y, const char* Str, uint32_t Size, uint32_t col, uint32_t StrokeColor){
    TaskSender::AddStringStroke(&m_cache->Cache,x,y,Str,Size,col,StrokeColor);
}

