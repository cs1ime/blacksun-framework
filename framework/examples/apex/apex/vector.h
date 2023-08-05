#pragma once

#ifndef __CSGO_VECTOR_
#define __CSGO_VECTOR_

#include "windows.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

#define mysqrt sqrt
#define myabs abs

struct vec2_t {
	float x, y;
	vec2_t() { this->x = 0.f; this->y = 0.f; }
	vec2_t(float _x, float _y) {
		this->x = _x;
		this->y = _y;
	}
	void init(float _x, float _y) {
		this->x = _x;
		this->y = _y;
	}
	void init() {
		this->x = 0.f;
		this->y = 0.f;
	}
	float dis(const vec2_t& vec) {
		float _x = vec.x - this->x;
		float _y = vec.y - this->y;
		return mysqrt(_x * _x + _y * _y);
	}
	float len() {
		return mysqrt(this->x * this->x + this->y * this->y);
	}
	vec2_t clamp() {
		if (this->y < -89.0f)
			this->y = -89.0f;

		if (this->y > 89.0f)
			this->y = 89.0f;
		if (this->x < -180.0f)
			this->x += 360.0f;

		if (this->x > 180.0f)
			this->x -= 360.0f;
		return *this;
	}
	inline bool operator==(const vec2_t& v) {
		return ((this->x == v.x) && (this->y == v.y));
	}
	inline vec2_t& operator+(const vec2_t& v) {
		this->x += v.x; this->y += v.y;
		return *this;
	}
	inline vec2_t& operator-(const vec2_t& v) {
		this->x -= v.x; this->y -= v.y;
		return *this;
	}
	inline vec2_t& operator*(const vec2_t& v) {
		this->x *= v.x; this->y *= v.y;
		return *this;
	}
	inline vec2_t& operator/(const vec2_t& v) {
		this->x /= v.x; this->y /= v.y;
		return *this;
	}
};
struct vec3_t {
	float x, y, z;
	vec3_t() { this->x = 0.f; this->y = 0.f; this->z = 0.f; }
	vec3_t(float _x, float _y, float _z) {
		this->x = _x;
		this->y = _y;
		this->z = _z;
	}
	void init(float _x, float _y, float _z) {
		this->x = _x;
		this->y = _y;
		this->z = _z;
	}
	void init() {
		this->x = 0.f;
		this->y = 0.f;
		this->z = 0.f;
	}
	inline float dot(const vec3_t v)
	{
		return this->x * v.x + this->y * v.y + this->z * v.z;
	}
	inline float dis(const vec3_t& vec) {
		float _x = vec.x - this->x;
		float _y = vec.y - this->y;
		float _z = vec.z - this->z;
		return mysqrt(_x * _x + _y * _y + _z * _z);
	}
	inline float len() {
		return mysqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}
	inline vec3_t degree_sub(vec3_t v) {
		if (v.x < 0.f) {
			v.x += 360.f;
		}
	}
	inline bool operator==(const vec3_t& v) {
		return ((this->x == v.x) && (this->y == v.y) && (this->z == v.z));
	}
	inline vec3_t& operator+(const vec3_t& v) {
		this->x += v.x; this->y += v.y; this->z += v.z;
		return *this;
	}
	inline vec3_t& operator-(const vec3_t& v) {
		this->x -= v.x; this->y -= v.y; this->z -= v.z;
		return *this;
	}
	inline vec3_t& operator*(const vec3_t& v) {
		this->x *= v.x; this->y *= v.y; this->z *= v.z;
		return *this;
	}
	inline vec3_t& operator/(const vec3_t& v) {
		this->x /= v.x; this->y /= v.y; this->z /= v.z;
		return *this;
	}
};

struct view_matrix_t
{
	float* operator[](int index)
	{
		return matrix[index];
	}
	float matrix[4][4];
};


#endif // !__CSGO_VECTOR_

