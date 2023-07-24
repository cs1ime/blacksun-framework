#pragma once

#include <math.h>

struct aimvec2_t {
	float x, y;
	aimvec2_t() { this->x = 0.f; this->y = 0.f; }
	aimvec2_t(float _x, float _y) {
		this->x = _x;
		this->y = _y;
	}
	float dis(const aimvec2_t& vec) {
		float _x = vec.x - this->x;
		float _y = vec.y - this->y;
		return sqrt(_x * _x + _y * _y);
	}
	float len() {
		return sqrt(this->x * this->x + this->y * this->y);
	}
	aimvec2_t clamp() {
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
	inline bool operator==(const aimvec2_t v) {
		return ((this->x == v.x) && (this->y == v.y));
	}
	inline aimvec2_t operator+(const aimvec2_t &v) {
		aimvec2_t thiz = *this;
		thiz.x += v.x; thiz.y += v.y;
		return thiz;
	}
	inline aimvec2_t operator-(const aimvec2_t &v) {
		aimvec2_t thiz = *this;
		thiz.x -= v.x; thiz.y -= v.y;
		return thiz;
	}
	inline aimvec2_t operator*(const aimvec2_t &v) {
		aimvec2_t thiz = *this;
		thiz.x *= v.x; thiz.y *= v.y;
		return thiz;
	}
	inline aimvec2_t operator/(const aimvec2_t &v) {
		aimvec2_t thiz = *this;
		thiz.x /= v.x; thiz.y /= v.y;
		return thiz;
	}
};


struct aimvec3_t {
	float x, y, z;
	aimvec3_t() { this->x = 0.f; this->y = 0.f; this->z = 0.f; }
	aimvec3_t(float _x, float _y, float _z) {
		this->x = _x;
		this->y = _y;
		this->z = _z;
	}
	inline float dot(const aimvec3_t v)
	{
		return this->x * v.x + this->y * v.y + this->z * v.z;
	}
	inline float dis(const aimvec3_t& vec) {
		float _x = vec.x - this->x;
		float _y = vec.y - this->y;
		float _z = vec.z - this->z;
		return sqrt(_x * _x + _y * _y + _z * _z);
	}
	inline float len() {
		return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}
	inline bool operator==(const aimvec3_t v) {
		return ((this->x == v.x) && (this->y == v.y) && (this->z == v.z));
	}
	inline aimvec3_t operator+(const aimvec3_t v) {
		aimvec3_t thiz = *this;
		thiz.x += v.x; thiz.y += v.y; thiz.z += v.z;
		return thiz;
	}
	inline aimvec3_t operator-(const aimvec3_t v) {
		aimvec3_t thiz = *this;
		thiz.x -= v.x; thiz.y -= v.y; thiz.z -= v.z;
		return thiz;
	}
	inline aimvec3_t operator*(const aimvec3_t v) {
		aimvec3_t thiz = *this;
		thiz.x *= v.x; thiz.y *= v.y; thiz.z *= v.z;
		return thiz;
	}
	inline aimvec3_t operator/(const aimvec3_t v) {
		aimvec3_t thiz = *this;
		thiz.x /= v.x; thiz.y /= v.y; thiz.z /= v.z;
		return thiz;
	}
};

