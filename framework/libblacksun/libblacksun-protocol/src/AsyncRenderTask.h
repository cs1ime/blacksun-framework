#pragma once

#ifndef _ASYNC_RENDER_TASK_
#define _ASYNC_RENDER_TASK_

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include "oldnames.h"
#include "util.h"

#define __forceinline __attribute__((always_inline)) inline

#include <protocol-universal-rend.h>

namespace TaskRecv {
    __forceinline void InitNewTaskBuffer(TaskBuffer *Buffer) {
        Buffer->ele_count = 0;
        Buffer->have_render_data = false;
        //Buffer.lock = 0;

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

        pthread_mutex_init(&Buffer->lock, &attr);

        pthread_mutexattr_destroy(&attr);

        Buffer->Share.PlaneHeight = 0;
        Buffer->Share.PlaneWidth = 0;
        Buffer->SizeOfBuffer = sizeof(Buffer);
        Buffer->Magic = 'LUXE';
    }
    __forceinline void UpdateShareData(TaskBuffer* Buffer, TaskShareData& Share) {
        Buffer->Share = Share;
    }

    __forceinline void RenderTask(TaskBuffer* Task, TaskDispatchRoutine Dispatch) {
        uint64_t f=util::GetTickCount();
        uint8_t *pData = &Task->Data;
        uint32_t *pSec_ele_count = (uint32_t*)(pData + DRAW_BUFFER_NEED_SIZE);
        uint8_t *pSecData = pData + DRAW_BUFFER_NEED_SIZE + sizeof(uint32_t);
        for (int i = 0; i < *pSec_ele_count; i++) {
            TaskElement* ele = (TaskElement*)pSecData;
            Dispatch(ele);
            pSecData += ele->SizeOfBlock;
        }
    }
    __forceinline bool ProcessTask(TaskBuffer* Task, TaskDispatchRoutine Dispatch) {
		if(Task->Magic!='LUXE')
			return false;
        if (Task->have_render_data == false) {
            if (Task->have_sec_render_data) {
                RenderTask(Task, Dispatch);
                return true;
            }
            else {
                return false;
            }
        }

        //if (InterlockedCompareExchange16((LONG*)&Task.lock, 1, 0) == 1)
        //	return false;

        if (pthread_mutex_trylock(&Task->lock) != 0)
            return false;


        uint8_t * pData = &Task->Data;

        uint8_t * pSecData = pData + DRAW_BUFFER_NEED_SIZE + sizeof(uint32_t);
        uint32_t * pSec_ele_count = (uint32_t*)(pData + DRAW_BUFFER_NEED_SIZE);
        *pSec_ele_count = Task->ele_count;
        memcpy(pSecData, pData, Task->SizeOfBuffer);

        Task->have_render_data = false;
        Task->have_sec_render_data = true;
        //Task.lock = 0;

        pthread_mutex_unlock(&Task->lock);


        RenderTask(Task, Dispatch);

        return true;


    }
}
namespace TaskSender {
	__forceinline void Begin(TaskSenderCache* Cache) {
		Cache->SizeOfBuffer = 0;
		Cache->ele_count = 0;
	}
	__forceinline bool PresentToTask(TaskBuffer* Task, TaskSenderCache* Cache) {
		if (Task->have_render_data)
			return false;
		//if (InterlockedCompareExchange16((LONG*)&Task->lock, 1, 0) == 1)
		//	return false;
		//if (pthread_mutex_trylock(&Task->lock) != 0)
		//	return false;
        pthread_mutex_lock(&Task->lock);

		Task->SizeOfBuffer = Cache->SizeOfBuffer;
		Task->ele_count = Cache->ele_count;
		memcpy(&Task->Data, &Cache->Data, Task->SizeOfBuffer);
		Task->have_render_data = true;

		//Task->lock = 0;

		pthread_mutex_unlock(&Task->lock);

		return true;
	}

	__forceinline bool CheckTask(TaskSenderCache* ctx) {
		if (DRAW_BUFFER_NEED_SIZE - ctx->SizeOfBuffer < ELEMENT_BLOCK_MAX_SIZE) {
			return false;
		}
		return true;
	}
	__forceinline void AddLine(TaskSenderCache* ctx, float x1, float y1, float x2, float y2, uint32_t color) {
		if (!CheckTask(ctx))
			return;
		TaskElement* pEle = (TaskElement*)((uint8_t*)&ctx->Data + ctx->SizeOfBuffer);

		pEle->Type = ELEMENT_LINE;
		ElementLine* ele = (ElementLine*)&pEle->Data;
		ele->x1 = x1;
		ele->x2 = x2;
		ele->y1 = y1;
		ele->y2 = y2;
		ele->col = color;

		pEle->SizeOfBlock = TaskElementHeaderSize + sizeof(ElementLine);
		ctx->ele_count++;
		ctx->SizeOfBuffer += pEle->SizeOfBlock;
	}
	__forceinline void AddBox(TaskSenderCache* ctx, float x, float y, float w, float h, uint32_t col) {
		AddLine(ctx, x, y, x + w, y, col);//top
		AddLine(ctx, x, y, x, y + h, col);//left
		AddLine(ctx, x + w, y, x + w, y + h, col);//right
		AddLine(ctx, x, y + h, x + w, y + h, col);//bottom
	}
	__forceinline void AddCornBox(TaskSenderCache* ctx, float x, float y, float w, float h, uint32_t col) {
		float xm = w / 4;
		float ym = h / 4.5;

		AddLine(ctx, x, y, x + xm, y, col); AddLine(ctx, x, y, x, y + ym, col);
		AddLine(ctx, x, y + h, x + xm, y + h, col); AddLine(ctx, x, y + h, x, y + h - ym, col);
		AddLine(ctx, x + w, y, x + w - xm, y, col); AddLine(ctx, x + w, y, x + w, y + ym, col);
		AddLine(ctx, x + w, y + h, x + w - xm, y + h, col); AddLine(ctx, x + w, y + h, x + w, y + h - ym, col);
	}
	__forceinline void AddCircle(TaskSenderCache* ctx, float centerx, float centery, float rad, long pointCount, uint32_t col) {
		if (!CheckTask(ctx))
			return;
		TaskElement* pEle = (TaskElement*)((uint8_t*)&ctx->Data + ctx->SizeOfBuffer);

		pEle->Type = ELEMENT_CIRCLE;
		ElementCircle* ele = (ElementCircle*)&pEle->Data;
		ele->centerx = centerx;
		ele->centery = centery;
		ele->rad = rad;
		ele->pointCount = pointCount;
		ele->col = col;

		pEle->SizeOfBlock = TaskElementHeaderSize + sizeof(ElementCircle);
		ctx->ele_count++;
		ctx->SizeOfBuffer += pEle->SizeOfBlock;
	}
	__forceinline void AddSoildCircle(TaskSenderCache* ctx, float centerx, float centery, float rad, long pointCount, uint32_t col) {
		if (!CheckTask(ctx))
			return;
		TaskElement* pEle = (TaskElement*)((uint8_t*)&ctx->Data + ctx->SizeOfBuffer);

		pEle->Type = ELEMENT_SOILDCIRCLE;
		ElementSoildCircle* ele = (ElementSoildCircle*)&pEle->Data;
		ele->centerx = centerx;
		ele->centery = centery;
		ele->rad = rad;
		ele->pointCount = pointCount;
		ele->col = col;

		pEle->SizeOfBlock = TaskElementHeaderSize + sizeof(ElementSoildCircle);
		ctx->ele_count++;
		ctx->SizeOfBuffer += pEle->SizeOfBlock;
	}
	__forceinline void FillRect(TaskSenderCache* ctx, float x, float y, float w, float h, uint32_t col) {
		if (!CheckTask(ctx))
			return;
		TaskElement* pEle = (TaskElement*)((uint8_t*)&ctx->Data + ctx->SizeOfBuffer);

		pEle->Type = ELEMENT_SOILDRECT;
		ElementSoildRect* ele = (ElementSoildRect*)&pEle->Data;
		ele->x = x;
		ele->y = y;
		ele->w = w;
		ele->h = h;
		ele->col = col;

		pEle->SizeOfBlock = TaskElementHeaderSize + sizeof(ElementSoildRect);
		ctx->ele_count++;
		ctx->SizeOfBuffer += pEle->SizeOfBlock;
	}
	__forceinline void AddSoildTriangle(TaskSenderCache* ctx, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t col) {
		if (!CheckTask(ctx))
			return;
		TaskElement* pEle = (TaskElement*)((uint8_t*)&ctx->Data + ctx->SizeOfBuffer);

		pEle->Type = ELEMENT_SOILDTRIANGLE;
		ElementSoildTriangle* ele = (ElementSoildTriangle*)&pEle->Data;
		ele->x1 = x1;
		ele->y1 = y1;
		ele->x2 = x2;
		ele->y2 = y2;
		ele->x3 = x3;
		ele->y3 = y3;
		ele->col = col;

		pEle->SizeOfBlock = TaskElementHeaderSize + sizeof(ElementSoildTriangle);
		ctx->ele_count++;
		ctx->SizeOfBuffer += pEle->SizeOfBlock;
	}
	__forceinline void AddString(TaskSenderCache* ctx, float x, float y, const char* Str, uint32_t Size, uint32_t col) {
		if (!CheckTask(ctx))
			return;
		TaskElement* pEle = (TaskElement*)((uint8_t*)&ctx->Data + ctx->SizeOfBuffer);

		pEle->Type = ELEMENT_STRING;
		ElementString* ele = (ElementString*)&pEle->Data;
		int len = strlen(Str);
		if (len > ELEMENT_STRING_MAX_LENGTH) {
			len = ELEMENT_STRING_MAX_LENGTH;
		}
		char* pstr = (char*)&ele->str;
		memcpy(pstr, Str, len);
		pstr[len] = 0;
		ele->col = col;
		ele->x = x;
		ele->y = y;
		ele->size = Size;

		pEle->SizeOfBlock = TaskElementHeaderSize + sizeof(ElementString) + len;
		ctx->ele_count++;
		ctx->SizeOfBuffer += pEle->SizeOfBlock;

	}
	__forceinline void AddStringStroke(TaskSenderCache* ctx, float x, float y, const char* Str, uint32_t Size, uint32_t col, uint32_t StrokeColor) {
		if (!CheckTask(ctx))
			return;
		TaskElement* pEle = (TaskElement*)((uint8_t*)&ctx->Data + ctx->SizeOfBuffer);

		pEle->Type = ELEMENT_STROKESTRING;
		ElementStrokeString* ele = (ElementStrokeString*)&pEle->Data;
		int len = strlen(Str);
		if (StrokeColor == 0x22334455)
			len = wcslen((wchar_t *)Str) * 2;
		if (len > ELEMENT_STRING_MAX_LENGTH) {
			len = ELEMENT_STRING_MAX_LENGTH;
		}
		char* pstr = (char*)&ele->str;
		memcpy(pstr, Str, len);
		pstr[len] = 0;
		if (StrokeColor == 0x22334455)
			pstr[len + 1] = 0;
		ele->col = col;
		ele->StrokeCol = StrokeColor;
		ele->x = x;
		ele->y = y;
		ele->size = Size;

		pEle->SizeOfBlock = TaskElementHeaderSize + sizeof(ElementStrokeString) + len;
		ctx->ele_count++;
		ctx->SizeOfBuffer += pEle->SizeOfBlock;
	}


}

#endif // !_ASYNC_RENDER_TASK_RECV_

