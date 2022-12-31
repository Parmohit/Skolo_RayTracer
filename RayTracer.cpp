// RayTracer.cpp : This file contains the 'main' function. Program execution begins and ends here.
//

#include <iostream>
#include <fstream>
#include <cmath>
#include "RayTracer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace rtPrime
{
    int envmap_width, envmap_height;
    std::vector<Vec3f> envmap;

    vec_uptr<Vec3f> raytraced_image()
    {
        // Step0. Read an image from disk
        int n = -1;
        unsigned char* pixmap = stbi_load("envmap.jpg", &envmap_width, &envmap_height, &n, 0);
        if (!pixmap || 3 != n) {
            std::cerr << "Error: can not load the environment map" << std::endl;
            return vec_uptr<Vec3f>(0);
        }
        envmap = std::vector<Vec3f>(envmap_width * envmap_height);
        for (int j = envmap_height - 1; j >= 0; j--) {
            for (int i = 0; i < envmap_width; i++) {
                envmap[i + j * envmap_width] = Vec3f(pixmap[(i + j * envmap_width) * 3 + 0], pixmap[(i + j * envmap_width) * 3 + 1], pixmap[(i + j * envmap_width) * 3 + 2]) * (1 / 255.);
            }
        }
        stbi_image_free(pixmap);

        // Rotation factor
        theta_rotation = (theta_rotation + 1) % INT_MAX;

        // Step1. Write an image to the disk
        std::vector<Material> materials;
        materials.push_back(Material(Vec4f(0.6f, 0.1f, 0.1f, 0.0f), Vec3f(0.4f, 0.4f, 0.3f), 50.f, 1.0f));
        materials.push_back(Material(Vec4f(0.9f, 0.1f, 0.0f, 0.0f), Vec3f(0.3f, 0.1f, 0.1f), 10.f, 1.0f));
        materials.push_back(Material(Vec4f(0.0f, 10.0f, 0.8f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f), 1425.f, 1.0f));
        materials.push_back(Material(Vec4f(0.0f, 0.5f, 0.1f, 0.8f), Vec3f(0.6f, 0.7f, 0.8f), 125.0f, 1.5f));

        vec_uptr<Sphere> spheres;
        spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(-3, 0, -16), 2, materials[0])));  // const L-value can be assigned R-value
        spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(-1.0, -1.5, -12), 2, materials[3])));
        spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(1.5, -0.5, -18), 3, materials[1])));
        spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(7, 5, -18), 4, materials[2])));

        // Step2. Define the position of light;

        vec_uptr<Light> lights;
        //lights.push_back(std::make_unique<Light>((Light(Vec3f(-25.f, 0.f, -40.f), 30.f))));

        lights.push_back(std::make_unique<Light>(Light(Vec3f(-20, 20, 20), 1.5)));
        lights.push_back(std::make_unique<Light>(Light(Vec3f(30, 50, -25), 1.8)));
        lights.push_back(std::make_unique<Light>(Light(Vec3f(30, 20, 30), 1.7)));

        return render(spheres, lights);
    }

    vec_uptr<Vec3f> render(const vec_uptr<Sphere>& spheres, const vec_uptr<Light>& lit)
    {
        vec_uptr<Vec3f> pixelInfo((w_height * w_width));

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
                float x = i - w_width / 2.;
                float y = w_height / 2. - j;
                float z = -w_height / (2 * tan(fov / 2));
                pixelInfo[i + j * w_width] = std::make_unique<Vec3f>(cast_ray(Vec3f(0.f, 0.f, 0.f), Vec3f(x, y, z).normalize(), spheres, lit, 0));
            }
        }

        return pixelInfo;

        // Commenting out to not generate a new .ppm file
        //write_to_file("Raytracer.ppm", pixelInfo, w_width, w_height);
    }

    // Method to create a new file with all the pixel information
    void write_to_file(const char* filename, vec_uptr<Vec3f>& pixelInfo, size_t width, size_t height)
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

    Vec3f cast_ray(const Vec3f& orig, const Vec3f& dir, const vec_uptr<Sphere>& spheres, const vec_uptr<Light>& lit, int depth) {
        Vec3f hit_pt, N;
        Material material{};
        if (depth > 5 || !pixel_depth_check(orig, dir, spheres, material, hit_pt, N)) {
            return background_color(orig, dir);
        }

        // Reflection Recursion
        Vec3f reflec_dir = reflect(dir, N).normalize();
        Vec3f reflec_orig = reflec_dir * N < 0 ? hit_pt - N * 1e-3 : hit_pt + N * 1e-3;
        Vec3f reflec_color = cast_ray(reflec_orig, reflec_dir, spheres, lit, depth + 1);

        // Refraction recursion
        Vec3f refrac_dir = refract(dir, N, material.refractive_index, 1.0f).normalize();
        Vec3f refr_orig = refrac_dir * N < 0 ? hit_pt - N * 1e-2 : hit_pt + N * 1e-2;
        Vec3f refrac_color = cast_ray(refr_orig, refrac_dir, spheres, lit, depth + 1);

        float diffuse_light_intensity{}, specular_light_intensity{};
        for (size_t i = 0; i < lit.size(); ++i)
        {
            Vec3f light_dir = (lit[i]->position - hit_pt).normalize();
            float light_dist = (lit[i]->position - hit_pt).norm();

            // Shadow prediction
            Vec3f shadow_orig = (light_dir * N) < 0 ? hit_pt - N * 1e-3 : hit_pt + N * 1e-3;
            Vec3f shad_inters_pt, shad_N;
            Material tmpmaterial{};
            if ((light_dir * N < 0) || (pixel_depth_check(shadow_orig, light_dir, spheres, tmpmaterial, shad_inters_pt, shad_N) && (shad_inters_pt - shadow_orig).norm() < light_dist))
                continue;

            diffuse_light_intensity += lit[i]->intensity * std::max(0.0f, (light_dir * N));
            specular_light_intensity += powf(std::max(0.0f, reflect(light_dir, N) * dir), material.sp_exp) * lit[i]->intensity;
        }
        material.diffuse_color = material.diffuse_color * (diffuse_light_intensity * material.albedo[0] + specular_light_intensity * material.albedo[1]) + reflec_color * material.albedo[2] + refrac_color * material.albedo[3];
        return material.diffuse_color;
    }

    bool pixel_depth_check(const Vec3f& orig, const Vec3f& dir, const vec_uptr<Sphere>& spheres, Material& material, Vec3f& hit_pt, Vec3f& normal)
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

        float board_dist = std::numeric_limits<float>::max();
        /*if (fabs(dir.y) > 1e-3)
        {
            float d = -(orig.y + 4) / dir.y;
            Vec3f pt = orig + dir * d;

            if (d > 0 && fabs(pt.x) < 10 && pt.z <-10 && pt.z >-30)
            {
                board_dist = d;
                if (board_dist < sphere_dist)
                    normal = Vec3f(0.0f, 1.0f, 0.0f);
                int ff = int(pt.x) + int(pt.z);
                material.diffuse_color = (int(pt.x+1000) + int(pt.z)) & 1 ? Vec3f(1, 1, 1) : Vec3f(1, .7, .3);
            }
        }*/

        return std::min(board_dist, sphere_dist) < 1000.f;
    }

    Vec3f refract(const Vec3f& I, const Vec3f& N, const float refracted_indx, const float inc_indx)
    {
        float cosi = -std::max(-1.0f, std::min(1.0f, I * N));
        Vec3f normal_refr = N;
        float ref_ind_dest = refracted_indx, ref_ind_incident = inc_indx;
        if (cosi < 0)
        {
            cosi = -cosi;
            normal_refr = -N;
            std::swap(ref_ind_dest, ref_ind_incident);
        }

        float ind_ratio = ref_ind_incident / ref_ind_dest;
        //check for total internal reflection
        float d = 1 - ind_ratio * ind_ratio * (1 - cosi * cosi);

        if (d < 0)
            return Vec3f(1.0f, 0.0f, 0.0f);
        else
            return (I * ind_ratio + normal_refr * (ind_ratio * cosi - std::sqrtf(d)));
    }

    Vec3f background_color(const Vec3f& orig, const Vec3f& dir)
    {
        // NOTE: for fish eye effect, we can use asin, acos but for straight/plain image use atan2
        // Also, the reflection using sin and cos are fisheyed, but with atan2, it'll fade to infinity

        int x_raw = (int)std::abs((std::atan2(dir.z, dir.x) / (2 * R_PI)) * envmap_width + theta_rotation * 0.05 * envmap_width / (2 * R_PI)) % envmap_width;
        int y_raw = std::abs((std::acos(dir.y) / R_PI) * envmap_height);        // +  envmap_height/M_PI

        int x = std::max(0, std::min(x_raw, envmap_width - 1));
        int y = std::max(0, std::min(y_raw, envmap_height - 1));
        return envmap[x + y * envmap_width];
    }
}
/* TASKs PENDING

1. Add a cube to 3d system.
2. Integrate with imgui to use mouse movement to look around.~ in-progress 
3. Add duck to viewport*/