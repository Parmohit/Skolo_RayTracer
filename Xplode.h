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
}

#endif
