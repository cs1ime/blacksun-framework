//
// 由 RSP 于 2023 年 3 月 25 日创建。
//

#include "AsyncRenderTask.h"
#include <protocol-rend.h>

// 接收器类的构造函数
rendrecver::rendrecver(ReleasedTaskBuffer *buffer) {
    m_buffer = buffer;
}

// 初始化新的任务缓冲区
void rendrecver::InitNewTaskBuffer(ReleasedTaskBuffer *buffer) {
    memset(buffer, 0, sizeof(ReleasedTaskBuffer));
    TaskRecv::InitNewTaskBuffer(&buffer->Task);
}

// 更新任务缓冲区中的屏幕尺寸信息
void rendrecver::UpdateScreenSize(uint32_t Width, uint32_t Height) {
    TaskShareData shdat;
    shdat.PlaneHeight = Height;
    shdat.PlaneWidth = Width;
    TaskRecv::UpdateShareData(&m_buffer->Task, shdat);
}

// 使用提供的调度函数处理任务
bool rendrecver::ProcessTask(TaskDispatchRoutine Dispatch) {
    return TaskRecv::ProcessTask(&m_buffer->Task, Dispatch);
}

// 发送器类的带参数构造函数
rendsender::rendsender(ReleasedTaskSenderCache *cache, TaskBuffer *task) {
    m_cache = cache;
    m_task = task;
}

// 发送器类的构造函数
rendsender::rendsender(ReleasedTaskSenderCache *cache) {
    m_cache = cache;
}

// 开始任务发送
void rendsender::Begin() {
    TaskSender::Begin(&m_cache->Cache);
}

// 将任务呈现到任务缓冲区
bool rendsender::PresentToTask(TaskBuffer *Task) {
    return TaskSender::PresentToTask(Task, &m_cache->Cache);
}


// -------------------------------------渲染类型




// 添加线段到任务缓冲区
void rendsender::AddLine(float x1, float y1, float x2, float y2, uint32_t color) {
    TaskSender::AddLine(&m_cache->Cache, x1, y1, x2, y2, color);
}

// 添加矩形到任务缓冲区
void rendsender::AddBox(float x, float y, float w, float h, uint32_t col) {
    TaskSender::AddBox(&m_cache->Cache, x, y, w, h, col);
}

// 添加带圆角的矩形到任务缓冲区
void rendsender::AddCornBox(float x, float y, float w, float h, uint32_t col) {
    TaskSender::AddCornBox(&m_cache->Cache, x, y, w, h, col);
}

// 添加圆到任务缓冲区
void rendsender::AddCircle(float centerx, float centery, float rad, long pointCount, uint32_t col) {
    TaskSender::AddCircle(&m_cache->Cache, centerx, centery, rad, pointCount, col);
}

// 添加实心圆到任务缓冲区
void rendsender::AddSoildCircle(float centerx, float centery, float rad, long pointCount, uint32_t col) {
    TaskSender::AddSoildCircle(&m_cache->Cache, centerx, centery, rad, pointCount, col);
}

// 填充一个矩形区域
void rendsender::FillRect(float x, float y, float w, float h, uint32_t col){
    TaskSender::FillRect(&m_cache->Cache, x, y, w, h, col);
}

// 添加一个实心三角形
void rendsender::AddSoildTriangle(float x1, float y1, float x2, float y2,float x3, float y3, uint32_t col){
    TaskSender::AddSoildTriangle(&m_cache->Cache, x1, y1, x2, y2, x3, y3, col);
}

// 添加一个字符串（无描边）
void rendsender::AddString(float x, float y, const char* Str, uint32_t Size, uint32_t col){
    TaskSender::AddString(&m_cache->Cache, x, y, Str, Size, col);
}

// 添加一个带描边的字符串
void rendsender::AddStringStroke(float x, float y, const char* Str, uint32_t Size, uint32_t col, uint32_t StrokeColor){
    TaskSender::AddStringStroke(&m_cache->Cache, x, y, Str, Size, col, StrokeColor);
}


