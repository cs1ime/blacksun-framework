#pragma once

#include <shared_mutex>
#include <functional>
#include <thread>
#include "AimVec.h"

using fntype_mouse_move = void(*)(int, int);

class AimFramework
{
public:
	AimFramework()
	{

	}
	~AimFramework()
	{
		cancelThread();
	}
	AimFramework& setFunction(fntype_mouse_move fn)
	{
		mouse_move = fn;
		return *this;
	}
	AimFramework& setRate(int rate)
	{
		m_aimRate = rate;
		return *this;
	}
	AimFramework& setResolution(int w, int h)
	{
		m_width = w;
		m_height = h;
		return *this;
	}
	void enableAim()
	{
		smtx.lock();
		m_isEnableAim = true;
		smtx.unlock();
	}
	void disableAim()
	{
		smtx.lock();
		m_isEnableAim = false;
		smtx.unlock();
	}
	void updateTargetPosition(int x, int y)
	{
		//smtx.lock();
		m_latestPosition = { (float)x,(float)y };
		//smtx.unlock();
	}
	void launchThread();
	void cancelThread()
	{
		
		if(m_thread!=nullptr)
		{
			smtx.lock();
			m_threadCancel=true;
			m_thread->join();
			delete m_thread;
			m_thread=nullptr;
			smtx.unlock();
		}
	}
private:
	void processAimEvent();

	bool m_threadCancel=false;

	std::thread *m_thread;
	std::shared_mutex smtx;

	int m_aimRate = 0;
	int m_width = 0;
	int m_height = 0;

	bool m_isEnableAim = false;
	aimvec2_t m_latestPosition = { 0,0 };
	aimvec2_t m_latestMoveDistance = { 0,0 };

	uint64_t latestUpdateTime = 0;
	uint64_t currentUpdateTime = 0;

	aimvec2_t m_accumulation={0,0};
	bool m_isaccumulation=false;

	fntype_mouse_move mouse_move = nullptr;

};


