#include "Xplode.h"

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
				Vec3f light_dir = (Explosion::lightPos - hit).normalize();
				float lightIntensity = std::max(0.4f, light_dir * distance_normal_field(hit));

				//float displacement = (sin(16*hit.x) * sin(16*hit.y) * sin(16*hit.z) + 1) / 2;
				color = Vec3f(1, 1, 1) * lightIntensity;
			}
			pixelInfo[i + j * Explosion::w_width] = color;
		}
	}

	return pixelInfo;
}

float Explosion::sphere_dist(Vec3f& pos)
{
	// Below 2 lines represents the concept of DISPLACEMENT MAPPING + IMPLICIT SURFACE formation using more control points
	Vec3f s = Vec3f(pos).normalize() * Explosion::sphere_radius;
	float displacement = sin(16 * s.x) * sin(16 * s.y) * sin(16 * s.z) * 0.2;


	return pos.norm() - (Explosion::sphere_radius + displacement); // Sphere equation x2 + y2 + z2 - r2 = 0 
}

bool Explosion::check_point_in(const Vec3f& origin, const Vec3f& dir, Vec3f& hit)
{
	Vec3f pos = origin;
	for (size_t i = 0; i < 128; ++i)
	{
		float d = sphere_dist(pos);
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
	float d = sphere_dist(pos);

	float nx = sphere_dist(pos + Vec3f(0.1f, 0, 0)) - d;
	float ny = sphere_dist(pos + Vec3f(0, 0.1f, 0)) - d;
	float nz = sphere_dist(pos + Vec3f(0, 0, 0.1f)) - d;

	return Vec3f(nx, ny, nz).normalize();
}


/*TODO
1. Draw a ring using implicit surface concept.
2. Study blobby molecules and their gaussian sum, affine invariant
*/