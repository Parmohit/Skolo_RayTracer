// RayTracer.cpp : This file contains the 'main' function. Program execution begins and ends here.
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
    materials.push_back(Material(Vec2f(0.6f, 0.3f), Vec3f(0.4f, 0.4f, 0.3f), 50.f));
    materials.push_back(Material(Vec2f(0.9f,0.1f), Vec3f(0.3f, 0.1f, 0.1f), 10.f));

    std::vector<std::unique_ptr<Sphere>> spheres;
    spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(-3, 0, -16), 2, materials[0])));  // const L-value can be assigned R-value
    spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(-1.0, -1.5, -12), 2, materials[1])));
    spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(1.5, -0.5, -18), 3, materials[1])));
    spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(7, 5, -18), 4, materials[0])));

    // Step2. Define the position of light;

    // opitmization to unique_ptr
    std::vector<std::unique_ptr<Light>> lights;
    //lights.push_back(std::make_unique<Light>((Light(Vec3f(-25.f, 0.f, -40.f), 30.f))));
    
    lights.push_back(std::make_unique<Light>(Light(Vec3f(-20, 20,  20), 1.5)));
    lights.push_back(std::make_unique<Light>(Light(Vec3f( 30, 50, -25), 1.8)));
    lights.push_back(std::make_unique<Light>(Light(Vec3f( 30, 20,  30), 1.7)));

    render(spheres, lights);
    return 0;
}

void render(const std::vector<std::unique_ptr<Sphere>>& spheres, const std::vector<std::unique_ptr<Light>>& lit)
{
    std::vector<std::unique_ptr<Vec3f>> pixelInfo((w_height * w_width));

    // Code to populate background color in image 
    /*for (size_t i = 0; i < w_width; ++i)
    {
        for (size_t j = 0; j < w_height; ++j)
        {
            pixelInfo[i + j * w_width] = Vec3f(j / (float)w_height, i / (float)w_width, 0);
        }
    }*/

#pragma omp parallel for
    for (size_t i = 0; i < w_width; ++i)
    {
        for (size_t j = 0; j < w_height; ++j)
        {
            float x = (2 * i / (float)w_width - 1) * (tan(fov / 2.f)) * (w_width / (float)w_height);
            float y = -(2 * j / (float)w_height - 1) * tan(fov / 2.f);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            pixelInfo[i + j * w_width] = std::make_unique<Vec3f>(cast_ray(Vec3f(0.f, 0.f, 0.f), dir, spheres, lit));
        }
    }

    write_to_file("Raytracer.ppm", pixelInfo, w_width, w_height);
}

// Method to create a new file with all the pixel information
void write_to_file(const char* filename, std::vector<std::unique_ptr<Vec3f>>& pixelInfo, size_t width, size_t height)
{
    std::ofstream f(filename, std::ios::binary);

    f << "P6\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < width * height; ++i)
    {
        float max = std::max(pixelInfo[i]->x, std::max(pixelInfo[i]->y, pixelInfo[i]->z));
        if (max > 1) *pixelInfo[i] = *(pixelInfo[i]) * (1. / max);
        for (int k = 0; k < 3; ++k)
        {
            f << (char)(255 * std::max(0.f, std::min(1.f, (*pixelInfo[i])[k])));
        }
    }

    f.close();
}

Vec3f cast_ray(const Vec3f& orig, const Vec3f& dir, const std::vector<std::unique_ptr<Sphere>>& spheres, const std::vector<std::unique_ptr<Light>>& lit) {
    Vec3f hit_pt, N;
    Material material{};
    if (!pixel_depth_check(orig, dir, spheres, material, hit_pt, N)) {
        return Vec3f(0.2f, 0.7f, 0.8f); // background color
    }
    float diffuse_light_intensity{}, specular_light_intensity{};
    for (size_t i = 0; i < lit.size(); ++i)
    {
        Vec3f light_dir = (lit[i]->position - hit_pt).normalize();
        float light_dist = (lit[i]->position - hit_pt).norm();

        Vec3f shadow_orig = (light_dir * N) < 0 ? hit_pt - N*1e-3  : hit_pt + N*1e-3;
        Vec3f shad_inters_pt, shad_N;
        Material tmpmaterial{};
        if (pixel_depth_check(shadow_orig, light_dir, spheres, tmpmaterial, shad_inters_pt, shad_N) && (shad_inters_pt - shadow_orig).norm() < light_dist)
            continue;

        diffuse_light_intensity += lit[i]->intensity * std::max(0.0f, (light_dir * N));
        specular_light_intensity += powf(std::max(0.0f, reflect(light_dir, N) * dir), material.sp_exp) * lit[i]->intensity;
    }
    material.diffuse_color = material.diffuse_color * (diffuse_light_intensity * material.albedo[0] + specular_light_intensity * material.albedo[1]);
    return material.diffuse_color;
}

bool pixel_depth_check(const Vec3f& orig, const Vec3f& dir, const std::vector<std::unique_ptr<Sphere>>& spheres, Material& material, Vec3f& hit_pt, Vec3f& normal)
{
    float sphere_dist = std::numeric_limits<float>::max();

    for (size_t i = 0; i < spheres.size(); ++i)
    {
        float dist{};

        if (spheres[i]->ray_intersect(orig, dir, dist) && dist < sphere_dist)
        {
            sphere_dist = dist;
            material = spheres[i]->materiall;
            hit_pt = orig + dir * dist;
            normal = (hit_pt - spheres[i]->centre).normalize();
        }
    }
    return sphere_dist < 1000.f;
}
