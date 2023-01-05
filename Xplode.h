#ifndef XPLODE_H
#define XPLODE_H

#include "Geometry.h"
#include <fstream>

namespace Explosion
{
	// Constants
	constexpr int w_width = 640;
	constexpr int w_height = 480;
	constexpr float fov = R_PI / 3.0;
	constexpr float sphere_radius = 1.5f;

	const Vec3f lightPos (10.f, 10.f, 10.f);

	std::vector<Vec3f> start_point();
	bool check_point_in(const Vec3f& origin, const Vec3f& dir, Vec3f& hit);
	Vec3f distance_normal_field(Vec3f& pos);
	float sphere_dist(Vec3f& pos);
	Vec3f palette_fire(const float d);
}

// Implementation of PERLIN_ALGORITHM 
namespace PerlinNoise3d
{
	constexpr float noise_amplitude = 1.0;
	float fractal_brownian_motion(const Vec3f& x);
	template<typename T> T lerp(const float t, const T& a0, const T& a1);
	float hash(const float h);
	float noisee(const Vec3f& pos);
	Vec3f rotate(const Vec3f& v);
}

#endif
