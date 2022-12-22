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

struct Material
{
	Vec4f albedo{}; // 0 index store diffuse, 1 index stores specular
	Vec3f diffuse_color{};
	float sp_exp{};
	float refractive_index{};

	Material() :albedo{ 1.f,0.f,0.f,0.f }, refractive_index {1.0f}{}
	Material(const Material& m) :albedo{ m.albedo }, diffuse_color{ m.diffuse_color }, sp_exp{ m.sp_exp }, refractive_index{ m.refractive_index }{}
	Material(const Vec4f& a, const Vec3f& color, const float e, const float r_index) : albedo{ a }, diffuse_color{ color }, sp_exp{ e }, refractive_index{ r_index } {}

	// Copy constructor used instead of assignment operator '=' 
	// [[ DEPRECEATED ]]
	/*Material& operator=(const Material& rhs)
	{
		this->albedo = rhs.albedo;
		this->diffuse_color = rhs.diffuse_color;
		this->sp_exp = rhs.sp_exp;
		return *this;
	}*/
};

void render(const std::vector<std::unique_ptr<Sphere>>& spheres, const std::vector<std::unique_ptr<Light>>& lit);
void write_to_file(const char* filename, std::vector<std::unique_ptr<Vec3f>>& pixelInfo, size_t width, size_t height);
Vec3f cast_ray(const Vec3f& orig, const Vec3f& dir, const std::vector<std::unique_ptr<Sphere>>& spheres, const std::vector<std::unique_ptr<Light>>& lit, int depth=0);
bool pixel_depth_check(const Vec3f& orig, const Vec3f& dir, const std::vector<std::unique_ptr<Sphere>>& spheres, Material& material, Vec3f& hit_pt, Vec3f& normal);
Vec3f refract(const Vec3f& I, const Vec3f& N, const float refracted_index, const float inc_index = 1);

class Sphere
{
public:
	Vec3f centre{};
	float radius{};
	Material materiall{};

	Sphere() = delete;

	Sphere(const Vec3f& c, float r, const Material& m) : centre{ c }, radius{ r }, materiall{ m } {}

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
		intersection_pt = c_proj - dist;

		if (intersection_pt < 0) intersection_pt = dist + c_proj;
		if (intersection_pt < 0) return false;

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