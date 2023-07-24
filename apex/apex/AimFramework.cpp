#include "AimFramework.h"
#include <math.h>
#include <stdint.h>
#include "AimVec.h"
#include <iostream>
#include <util.h>
#include <random>
#include <chrono>
#include <thread>

static uint64_t GetRealTime()
{
	auto now = std::chrono::system_clock::now();
	auto dur = now.time_since_epoch();
	auto mill = std::chrono::duration_cast<std::chrono::milliseconds>(dur);

	return mill.count();
}

//#define p1x(v1)printf((""#v1"=%08llX\r\n"),v1)
#define p1d(v1)printf((""#v1"=%08lld\r\n"),v1)
#define p1f(v1)printf((""#v1"=%f\r\n"),v1)


#define p1x
//#define p1d
//#define p1f

static float getrand()
{
	std::random_device rd;
	std::uniform_real_distribution<float>u(0.f, 5.5f);
	std::mt19937 gen(rd());
	return u(gen);
}


void AimFramework::processAimEvent()
{
	if(!m_isEnableAim)
		return;
	uint64_t t = GetRealTime();
	

	//smtx.lock_shared();
	float width=(float)m_width;
	float height=(float)m_height;
	aimvec2_t screen = { width,height };
	aimvec2_t center = screen / aimvec2_t(2.f,2.f);
	aimvec2_t m_targetPosition;
	m_targetPosition.x = m_latestPosition.x;
	m_targetPosition.y = m_latestPosition.y;

	//smtx.unlock_shared();
	if (t - latestUpdateTime > 50)
	{
		currentUpdateTime = t;
		m_targetPosition.x = m_latestPosition.x;
		m_targetPosition.y = m_latestPosition.y;
		m_latestMoveDistance = m_targetPosition - center;
		latestUpdateTime = GetRealTime();
	}

	//float mouseMoveSpeed=3000.f; //假设鼠标每秒移动3000个像素

	float inch = sqrt(width*width + height*height);
	float step = 22.f * (inch / 2202.f);
	//float step = (mouseMoveSpeed / m_aimRate);
	
	p1f(step);
	aimvec2_t real_delta = m_targetPosition - center;
	aimvec2_t delta = m_targetPosition - center;
	delta=delta / aimvec2_t(6.5f,6.5f);
	// float alpha = atan(delta.x / delta.y);
	float alpha = atan(delta.x / delta.y);
	aimvec2_t move_step = aimvec2_t(step*sin(alpha), step*cos(alpha));
	float ra = getrand();
	aimvec2_t rand_reduce = aimvec2_t(ra*sin(alpha), ra*cos(alpha));
	move_step.x = abs(move_step.x); move_step.y= abs(move_step.y);
	aimvec2_t move_edge = move_step * aimvec2_t(2.f, 2.f);
	{
		aimvec2_t move = delta;

		if (move.x > move_edge.x)
			move.x = move_step.x;
		if (move.y > move_edge.y)
			move.y = move_step.y;
		if (move.x < -move_edge.x)
			move.x = -move_step.x;
		if (move.y < -move_edge.y)
			move.y = -move_step.y;

		p1f(move.x);
		p1f(move.y);

		{
			if ((int)move.x != 0 || (int)move.y != 0)
			{
				mouse_move(static_cast<int>(move.x), static_cast<int>(move.y));
			}
		}
		
	}
	
}
void AimFramework::launchThread()
{
	m_thread = new std::thread{[&]{
		while(!m_threadCancel)
		{
			processAimEvent();

			int &&sleep_time=1000000/m_aimRate;
			std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
		}
	}};
	return;
}


