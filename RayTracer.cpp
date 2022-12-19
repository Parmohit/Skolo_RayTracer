// RayTracer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <fstream>

#include "Geometry.h"
#include "RayTracer.h"

int main()
{
    // Step1. Write an image to the disk
    std::vector<Material> materials;
    materials.push_back(Material(Vec2f(0.6f, 0.3f), Vec3f(0.4f, 0.4f, 0.3f), 50.0f));
    materials.push_back(Material(Vec2f(0.9f,0.1f), Vec3f(0.3f, 0.1f, 0.1f), 10.0f));

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, materials[0]));  // const L-value can be assigned R-value
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, materials[1]));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, materials[1]));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, materials[0]));

    // Step2. Define the position of light;
    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20.f, 20.f, 20.f), 1.5f));
    lights.push_back(Light(Vec3f(30.0f, 50.0f, -25.0f), 1.8f));
    lights.push_back(Light(Vec3f(30.0f, 20.0f, 30.0f), 1.7f));

    render(spheres, lights);
}

void render(const std::vector<Sphere>& spheres, const std::vector<Light>& lit)
{
    std::vector<Vec3f> pixelInfo((w_height * w_width), Vec3f(0, 0, 0));

    // Code to populate background color in image 
    for (size_t i = 0; i < w_width; ++i)
    {
        for (size_t j = 0; j < w_height; ++j)
        {
            pixelInfo[i + j * w_width] = Vec3f(j / (float)w_height, i / (float)w_width, 0);
        }
    }

    for (size_t i = 0; i < w_width; ++i)
    {
        for (size_t j = 0; j < w_height; ++j)
        {
            float x = (2 * i / (float)w_width - 1) * (tan(fov / 2.f)) * ((float)w_width / (float)w_height);
            float y = -(2 * j / (float)w_height - 1) * tan(fov / 2.f);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            pixelInfo[i + j * w_width] = cast_ray(Vec3f(0.f, 0.f, 0.f), dir, spheres, lit);
        }
    }

    write_to_file("Raytracer.ppm", pixelInfo, w_width, w_height);
}

// Method to create a new file with all the pixel information
void write_to_file(const char* filename, std::vector<Vec3f>& pixelInfo, size_t width, size_t height)
{
    std::ofstream f(filename, std::ios::binary);

    f << "P6\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < width * height; ++i)
    {
        float max = std::max(pixelInfo[i][0], std::max(pixelInfo[i][1], pixelInfo[i][2]));
        if (max > 1) pixelInfo[i] = pixelInfo[i] * (1. / max);
        for (int k = 0; k < 3; ++k)
        {
            f << (char)(255 * std::max(0.f, std::min(1.f, pixelInfo[i][k])));
        }
    }

    f.close();
}

Vec3f cast_ray(const Vec3f& orig, const Vec3f& dir, const std::vector<Sphere>& spheres, const std::vector<Light>& lit) {
    Vec3f hit_pt, N;
    Material material{};
    if (!pixel_depth_check(orig, dir, spheres, material, hit_pt, N)) {
        return Vec3f(0.2f, 0.7f, 0.8f); // background color
    }
    float diffuse_light_intensity{}, specular_light_intensity{};
    for (size_t i = 0; i < lit.size(); ++i)
    {
        Vec3f light_dir = (lit[i].position - hit_pt).normalize();

        diffuse_light_intensity += lit[i].intensity * std::max(0.0f, (light_dir * N));
        specular_light_intensity += lit[i].intensity * powf(std::max(0.0f, dir * reflect(light_dir, N)), material.sp_exp);
    }
    material.diffuse_color = material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1.0f, 1.0f, 1.0f) * specular_light_intensity * material.albedo[1];
    return material.diffuse_color;
}

bool pixel_depth_check(const Vec3f& orig, const Vec3f& dir, const std::vector<Sphere>& spheres, Material& material, Vec3f& hit_pt, Vec3f& normal)
{
    float sphere_dist = std::numeric_limits<float>::max();

    for (size_t i = 0; i < spheres.size(); ++i)
    {
        float dist{};

        if (spheres[i].ray_intersect(orig, dir, dist) && dist < sphere_dist)
        {
            sphere_dist = dist;
            material = spheres[i].materiall;
            hit_pt = orig + dir * dist;
            normal = (hit_pt - spheres[i].centre).normalize();
        }
    }
    return sphere_dist < 1000.f;
}
