#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "Geometry.h"

// Don't want to slow down the exection time? use "contexpr"
constexpr int w_width = 1024;
constexpr int w_height = 768;
constexpr auto M_PI = 3.14159265358979323846;
constexpr auto fov = M_PI / 2;

class Sphere;
class Light;

void write_to_file(const char* filename, const std::vector<Vec3f>& pixelInfo, size_t width, size_t height);

class Sphere
{
public:
	Vec3f centre{};
	float radius{};
	Vec3f material{};

	Sphere() = delete;

	Sphere(const Vec3f& c, float r, const Vec3f& color) : centre{ c }, radius{ r }, material{ color } {}

	// R0	: Starting point of ray
	// dir	: Direction of ray(unit vector)
	 bool ray_intersect(const Vec3f& R0, const Vec3f& dir, float& intersection_pt) const
	{
		// Distance from centre of sphere to ray origin
		Vec3f vpc = centre - R0; 
		// Find projection of vpc on dir
		float c_proj = vpc * dir;	
		// Find projection perpendicular distance from centre of sphere
		float d = vpc * vpc - c_proj * c_proj;

		if (d > radius * radius) return false;
		// Find distance from projection to intersetion point
		float dist = std::sqrtf(radius * radius - d);
		// Find distance from ray starting point till intersection point
		double t0 = dist - c_proj;

		if (t0 < 0) t0 = dist + c_proj;
		if (t0 < 0) return false;

		intersection_pt = t0;
		return true;
	}
};

struct Light
{
	Vec3f position{};
	float intensity{};
	Light(const Vec3f& pos, float strength) : position{ pos }, intensity{ strength } {}
};

#endif