#pragma once
#include "Common.h"
#include "SDLInclude.h"

//#define HRect SDL_Rect

#define HBox2D HBox2<float>

#define HBox2Vec2(T) glm::vec<2, T, glm::defaultp>

template<typename T>
struct HBox2
{
public:
	using FReal = T;
	glm::vec<2, T, glm::defaultp> _min;
	glm::vec<2, T, glm::defaultp> _max;

	HBox2<T>() { }

	HBox2<T>(const glm::vec<2, T, glm::defaultp>& inMin, const glm::vec<2, T, glm::defaultp>& inMax)
		: _min(inMin)
		, _max(inMax)
	{ }

	bool operator==(const HBox2<T>& other) const
	{
		return (_min == other._min) && (_max == other._max);
	}

	bool operator!=(const HBox2<T>& other) const
	{
		return !(*this == other);
	}

	bool Equals(const HBox2<T>& other) const
	{
		return _min.x == other._min.x
			&& _min.y == other._min.y
			&& _max.x == other._max.x
			&& _max.y == other._max.y;
	}

	HBox2<T>& operator+=(const glm::vec<2, T, glm::defaultp>& other)
	{
		_min.x = glm::min(_min.x, other.x);
		_min.y = glm::min(_min.y, other.y);
		_max.x = glm::max(_max.x, other.x);
		_max.y = glm::max(_max.y, other.y);
		return *this;
	}

	HBox2<T>& operator+=(const HBox2<T>& other)
	{
		_min.x = glm::min(_min.x, other._min.x);
		_min.y = glm::min(_min.y, other._min.y);
		_max.x = glm::max(_max.x, other._max.x);
		_max.y = glm::max(_max.y, other._max.y);
		return *this;
	}

	glm::vec<2, T, glm::defaultp> GetClosestPointTo(const glm::vec<2, T, glm::defaultp>& point) const
	{
		glm::vec<2, T, glm::defaultp> closestPoint = point;
		if (point.x < _min.x)
		{
			closestPoint.x = _min.x;
		}
		else if (point.y > _max.y)
		{
			closestPoint.x = _max.x;
		}
		if (point.y < _min.y)
		{
			closestPoint.y = _min.y;
		}
		else if (point.y > _max.y)
		{
			closestPoint.y = _max.y;
		}
		return closestPoint;
	}

	bool Intersect(const HBox2<T>& other) const
	{
		if ((_min.x > other._max.x) || (other._min.x > _max.x))
		{
			return false;
		}
		if ((_min.y > other._max.y) || (other._min.y > _max.y))
		{
			return false;
		}
		return true;
	}

	HBox2<T> Overlap(const HBox2<T>& other) const
	{
		if (Intersect(other) == false)
		{
			static HBox2<T> emptyBox;
			return emptyBox;
		}
		HBox2Vec2(T) minVector, maxVector;
		minVector.x = glm::max(_min.x, other._min.X);
		maxVector.x = glm::min(_max.x, other._max.x);
		minVector.y = glm::max(_min.y, other._min.y);
		maxVector.y = glm::min(_max.y, other._max.y);
		return HBox2<T>(minVector, maxVector);
	}

	HString ToString() const
	{
		return HString::printf("Min=(%s), Max=(%s)", HString::FromVec2(_min).c_str(), HString::FromVec2(_max).c_str());
	}


};