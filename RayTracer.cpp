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
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, Vec3f(0.4f, 0.4f, 0.3f)));  // const L-value can be assigned R-value
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, Vec3f(0.3f, 0.1f, 0.1f)));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, Vec3f(0.3f, 0.1f, 0.1f)));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, Vec3f(0.4f, 0.4f, 0.3f)));

    // Step2. Define the position of light;
    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(17.f, 12.f, -30.f), 3.0f));

    render(spheres, lights);
}

void render(const std::vector<Sphere>& spheres, const std::vector<Light>& lit)
{
    std::vector<Vec3f> pixelInfo((w_height * w_width), Vec3f(0, 0, 0));

    // Code to populate color in image 
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
void write_to_file(const char* filename, const std::vector<Vec3f>& pixelInfo, size_t width, size_t height)
{
    std::ofstream f(filename, std::ios::binary);

    f << "P6\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < width * height; ++i)
    {
        for (int k = 0; k < 3; ++k)
        {
            f << (char)(255 * std::max(0.f, std::min(1.f, pixelInfo[i][k])));
        }
    }

    f.close();
}

Vec3f cast_ray(const Vec3f& orig, const Vec3f& dir, const std::vector<Sphere>& spheres, const std::vector<Light>& lit) {
    Vec3f material, hit_pt, N;
    if (!pixel_depth_check(orig, dir, spheres, material, hit_pt, N)) {
        return Vec3f(0.2f, 0.7f, 0.8f); // background color
    }
    for (size_t i = 0; i < lit.size(); ++i)
    {
        Vec3f light_dir = (lit[i].position - hit_pt).normalize();
        material = material * lit[i].intensity * std::max(0.0f, (light_dir * N));
    }

    return material;
}

bool pixel_depth_check(const Vec3f& orig, const Vec3f& dir, const std::vector<Sphere>& spheres, Vec3f& material, Vec3f& hit_pt, Vec3f& normal)
{
    float sphere_dist = std::numeric_limits<float>::max();

    for (size_t i = 0; i < spheres.size(); ++i)
    {
        float dist{};

        if (spheres[i].ray_intersect(orig, dir, dist) && dist < sphere_dist)
        {
            sphere_dist = dist;
            material = spheres[i].material;
            hit_pt = orig + dir * dist;
            normal = (hit_pt - spheres[i].centre).normalize();
        }
    }
    return sphere_dist < 1000.f;
}
