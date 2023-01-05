#include "Xplode.h"

int main()
{
	Explosion::start_point();
	return 0;
}

std::vector<Vec3f> Explosion::start_point()
{
	std::vector<Vec3f> pixelInfo(Explosion::w_height * Explosion::w_width);

#pragma omp parallel for
	for (int i = 0; i < Explosion::w_width; ++i)
	{
		for (int j = 0; j < Explosion::w_height; ++j)
		{
			float dir_x = i - (Explosion::w_width/2);
			float dir_y = (Explosion::w_height / 2) - j;
			float dir_z = -Explosion::w_height/(2*tan(Explosion::fov/2));

			Vec3f hit{};
			Vec3f color( 0.2, 0.7, 0.8 );
			Vec3f dir{ Vec3f(dir_x, dir_y, dir_z).normalize() };
			if (Explosion::check_point_in(Vec3f(0, 0, 3), dir, hit))
			{
				float noise_level = (Explosion::sphere_radius - hit.norm()) / PerlinNoise3d::noise_amplitude;
				Vec3f light_dir = (Explosion::lightPos - hit).normalize();
				float lightIntensity = std::max(0.4f, light_dir * Explosion::distance_normal_field(hit));

				//float displacement = (sin(16*hit.x) * sin(16*hit.y) * sin(16*hit.z) + 1) / 2;
				color = palette_fire(-0.2f + noise_level * 2.f) * lightIntensity;
			}
			pixelInfo[i + j * Explosion::w_width] = color;
		}
	}

	std::ofstream ofs("./out.ppm", std::ios::binary); // save the framebuffer to file
	ofs << "P6\n" << Explosion::w_width << " " << Explosion::w_height << "\n255\n";
	for (size_t i = 0; i < Explosion::w_height * Explosion::w_width; ++i) {
		for (size_t j = 0; j < 3; j++) {
			ofs << (char)(std::max(0, std::min(255, static_cast<int>(255 * pixelInfo[i][j]))));
		}
	}
	ofs.close();

	return pixelInfo;
}

float Explosion::sphere_dist(Vec3f& pos)
{
	// Below 2 lines represents the concept of DISPLACEMENT MAPPING + IMPLICIT SURFACE formation using more control points
	//Vec3f s = Vec3f(pos).normalize() * Explosion::sphere_radius;
	float displacement = -PerlinNoise3d::fractal_brownian_motion(pos * 3.4f)*PerlinNoise3d::noise_amplitude;


	return pos.norm() - (Explosion::sphere_radius + displacement); // Sphere equation x2 + y2 + z2 - r2 = 0 
}

// Sphere Tracing or Ray Marching 
bool Explosion::check_point_in(const Vec3f& origin, const Vec3f& dir, Vec3f& hit)
{
	if (origin * origin - pow(origin * dir, 2) > pow(Explosion::sphere_radius, 2)) return false; // early discard
	Vec3f pos = origin;
	for (size_t i = 0; i < 128; ++i)
	{
		float d = Explosion::sphere_dist(pos);
		if (d < 0)
		{
			hit = pos;
			return true;
		}
			
		pos = pos + dir * (std::max(0.01f, d));
		
	}
	return false;
}

Vec3f Explosion::distance_normal_field(Vec3f& pos)
{
	float d = Explosion::sphere_dist(pos);

	float nx = Explosion::sphere_dist(pos + Vec3f(0.1f, 0, 0)) - d;
	float ny = Explosion::sphere_dist(pos + Vec3f(0, 0.1f, 0)) - d;
	float nz = Explosion::sphere_dist(pos + Vec3f(0, 0, 0.1f)) - d;

	return Vec3f(nx, ny, nz).normalize();
}

Vec3f Explosion::palette_fire(const float d)
{
	const Vec3f   yellow(1.7, 1.3, 1.0); 
	const Vec3f   orange(1.0, 0.6, 0.0);
	const Vec3f      red(1.0, 0.0, 0.0);
	const Vec3f darkgray(0.2, 0.2, 0.2);
	const Vec3f     gray(0.4, 0.4, 0.4);

	float x = std::max(0.0f, std::min(1.f, d));
	if (x < 0.30f)
	{
		return PerlinNoise3d::lerp(x * 2.5, gray, darkgray);
	}
	else if (x < 0.50f)
	{
		return PerlinNoise3d::lerp(x * 2.5 - 2, darkgray, red);
	}
	else if (x < 0.75f)
	{
		return PerlinNoise3d::lerp(x * 2.5 - 2.5, red, orange);
	}
	else
	{
		return PerlinNoise3d::lerp(x * 4 - 3, orange, yellow);
	}
}

//------------------------------ Implementation for PERLIN NOISE ------------------------------

namespace PerlinNoise3d
{
	template<typename T>
	T lerp(const float t, const T& a0, const T& a1)
	{
		return a0 + (a1 - a0) * std::max(0.f, std::min(t, 1.f));
	}

	float hash(const float h)
	{
		float x = sin(h) * 43758.5453f;
		return x - floor(x);
	}

	float noisee(const Vec3f& pos)
	{
		Vec3f a(floor(pos.x), floor(pos.y), floor(pos.z));
		Vec3f f(pos.x - a.x, pos.x - a.y, pos.z - a.z);
		f = f * (f * (Vec3f(3.f, 3.f, 3.f) - f * 2.f));

		float n = a * Vec3f(1.f, 57.f, 113.f);

		return lerp(f.z,
			lerp(
				f.y,
				lerp(f.x, hash(n + 0.f), hash(n + 1.f)),
				lerp(f.x, hash(n + 57.f), hash(n + 58.f))
			),
			lerp(
				f.y,
				lerp(f.x, hash(n + 113.f), hash(n + 114.f)),
				lerp(f.x, hash(n + 170.f), hash(n + 171.f))
			)
		);

	}

	Vec3f rotate(const Vec3f& v) {
		return Vec3f(Vec3f(0.00, 0.80, 0.60) * v, Vec3f(-0.80, 0.36, -0.48) * v, Vec3f(-0.60, -0.48, 0.64) * v);
	}

	//

	float fractal_brownian_motion(const Vec3f& x) {
		Vec3f p = rotate(x);
		float f = 0;
		f += 0.5000 * noisee(p); p = p * 2.32;
		f += 0.2500 * noisee(p); p = p * 3.03;
		f += 0.1250 * noisee(p); p = p * 2.61;
		f += 0.0625 * noisee(p);
		return f / 0.9375;
	}
}
/*TODO
1. Draw a ring using implicit surface concept.
2. Study blobby molecules and their gaussian sum, affine invariant
3. Perlin's noise
*/