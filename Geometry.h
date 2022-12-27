#ifndef GEOMETRY_H
#define GEOMETRY_H

/// <To achieve a general template for storing 'x/y/z' for any geometric point in space >
/// 
/// </summary>

#include <cassert>

template<typename T, size_t size>
class Vec
{
	Vec() {}
};

typedef Vec<float, 4> Vec4f;
typedef Vec<float, 3> Vec3f;
typedef Vec<float, 2> Vec2f;
typedef Vec<int, 4> Vec4i;
typedef Vec<int, 3> Vec3i;
typedef Vec<int, 2> Vec2i;

// *********** Below are partial template specializations for above mentioned template **************

template<typename T>
class Vec<T, 2>
{
public:
	T x{}, y{};
	Vec() = default;
	Vec(T x1, T y1) : x{ x1 }, y{ y1 }{}
	Vec(const Vec<T, 2>& v) : x{ v.x }, y{ v.y } {}// use "delete" is you don't want constructor intitialization

	T& operator[](int pos)
	{
		assert(pos < 2 && "Index not available; 0 <= index < 2");
		switch (pos) {
		case 0: return x;
		case 1: return y;
		default: assert(false && "Index error : 0 <= index < 2");
		}
	}

	const T& operator[](int pos) const
	{
		assert(pos < 2 && "Index not available; 0 <= index < 2");
		switch (pos) {
		case 0: return x;
		case 1: return y;
		default: assert(false && "Index error : 0 <= index < 2");
		}
	}

	// using "return reference" so it can be chained
	Vec<T, 2>& operator=(const Vec<T, 2>& v)
	{
		//Vec(v);
		if (this == &v)
			return *this;
		this->x = v.x;
		this->y = v.y;
		return *this;
	}

};

template<typename T>
class Vec<T, 3>
{
public:
	T x{}, y{}, z{};
	Vec() = default;
	Vec(T x1, T y1, T z1) : x{ x1 }, y{ y1 }, z{ z1 }{}
	Vec(const Vec<T, 3>& v) : x{ v.x }, y{ v.y }, z{ v.z } {} // use "delete" is you don't want constructor intitialization

	T& operator[](int pos)
	{
		assert(pos < 3 && "Index not available; 0 <= index < 3");
		switch (pos) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		default: assert(false && "Index error : 0 <= index < 3");
		}
	}

	const T& operator[](int pos) const
	{
		assert(pos < 3 && "Index not available; 0 <= index < 3");
		switch (pos) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		default: assert(false && "Index error : 0 <= index < 3");
		}
	}

	Vec<T, 3>& operator=(const Vec<T, 3>& v)
	{
		//Vec(v);
		if (this == &v)
			return *this;
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}

	float norm() { return std::sqrtf(x * x + y * y + z * z); }
	Vec<T, 3>& normalize()
	{
		*this = (*this) * (1 / this->norm());
		return *this;
	}
};

template<typename T>
class Vec<T, 4>
{
public:
	T x{}, y{}, z{}, w{};
	Vec() = default;
	Vec(T x1, T y1, T z1, T w1) : x{ x1 }, y{ y1 }, z{ z1 }, w{ w1 }{}
	Vec(const Vec<T, 4>& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } {} // use "delete" is you don't want constructor intitialization

	T& operator[](int pos)
	{
		assert(pos < 4 && "Index not available; 0 <= index < 4");
		switch (pos) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		default: assert(false && "Index error : 0 <= index < 4");
		}
	}

	const T& operator[](int pos) const
	{
		assert(pos < 4 && "Index not available; 0 <= index < 4");
		switch (pos) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		default: assert(false && "Index error : 0 <= index < 4");
		}
	}

	Vec<T, 4>& operator=(const Vec<T, 4>& v)
	{
		//Vec(v);
		if (this == &v)
			return *this;
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
		return *this;
	}

	float norm() { return std::sqrtf(x * x + y * y + z * z + w * w); }
	Vec<T, 4>& normalize()
	{
		*this = (*this) * (1 / this->norm());
		return *this;
	}
};

// Function templates

// Vector scaling
template<typename T, typename U, size_t sz>
Vec<T, sz> operator*(const Vec<T, sz>& vect, const U& val)
{
	Vec<T, sz> sc_vec;
	for (size_t i = 0; i < sz; ++i)
		sc_vec[i] = vect[i] * val;
	return sc_vec;
}

// Vector - Vector multiplication / dot product of unit vectors
template<typename T, size_t sz>
T operator*(const Vec<T, sz>& lhs, const Vec<T, sz>& rhs)
{
	T sum{};
	for (size_t i = 0; i < sz; ++i)
		sum += lhs[i] * rhs[i];
	return sum;
}

// Vector negation
template<typename T,size_t sz>
Vec<T, sz> operator-(const Vec<T,sz>& opd)
{
	Vec<T, sz> ret;
	for (size_t i = 0; i < sz; ++i)
		ret[i] = opd[i] * -1;
	return ret;
}

// Vector subtraction
template<typename T, size_t sz>
Vec<T, sz> operator-(const Vec<T, sz>& lhs, const Vec<T, sz>& rhs)
{
	Vec<T, sz> ret;
	for (size_t i = 0; i < sz; ++i)
		ret[i] = lhs[i] - rhs[i];
	return ret;
}

// Vector addition
template<typename T, size_t sz>
Vec<T, sz> operator+(const Vec<T, sz>& lhs, const Vec<T, sz>& rhs)
{
	Vec<T, sz> ret;
	for (size_t i = 0; i < sz; ++i)
		ret[i] = lhs[i] + rhs[i];
	return ret;
}

// Vector console output
template<typename T, size_t sz>
std::ostream& operator <<(const std::ostream& out, const Vec<T, sz>& rhs)
{
	for (size_t i = 0; i < sz; ++i)
		out << rhs[i] << "";
	return out;
}

// Vector reflection 
template<typename T, size_t sz>
Vec<T, sz> reflect(const Vec<T,sz>& incident, const Vec<T,sz>& normal) // NOTE: Both are unit vectors | should be normalised
{
	Vec<T,sz> ref{};
	ref = incident - (normal * 2) * (incident * normal);
	return ref;
}

#endif

// C++ GENERAL WISDOM

// fully specialized template class starts with "template<>"
// partial template specialization for class starts with "template<int size>" or something like this
// 
// C++ does not allow any partial template specialisation for member functions. To achieve that, inherit the base class
// and change the function definition for that particular member function