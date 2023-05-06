#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#define INF 255
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;
void triangle(Vec3f *screen_coords, float *zbuffer, TGAImage &image, TGAColor color);
void triangle(Vec3f *screen_coords, float *zbuffer, TGAImage &image, TGAImage &tex, Vec3f *tex_coords, float &intensity);
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
Vec3f Barycentric(Vec3f *vertex, Vec2f p);
const Vec3f camera = Vec3f(0, 0, 3);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    if (x0 == x1 && y0 == y1)
    {
        image.set(x0, y0, color);
        return;
    }
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1))
    {
        steep = true;
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int y = y0;
    int dx = x1 - x0;
    int delta = y1 > y0 ? 1 : -1;
    int derror = delta * (y1 - y0) * 2;
    int error = 0;
    if (steep)
    {
        for (int x = x0; x <= x1; x++)
        {
            image.set(y, x, color);
            error += derror;
            if (error > dx)
            {
                y += delta;
                error -= 2 * dx;
            }
        }
    }
    else
    {
        for (int x = x0; x <= x1; x++)
        {
            image.set(x, y, color);
            error += derror;
            if (error > dx)
            {
                y += delta;
                error -= 2 * dx;
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage tex;
    if (!tex.read_tga_file("african_head_diffuse.tga"))
    {
        std::cerr << "not include african_head_diffuse.tga";
        return 0;
    }
    tex.flip_vertically();
    Vec3f light_dir(0, 0, -1);
    float *zbuffer = new float[width * height];
    for (int i = 0; i < width * height; ++i)
    {
        zbuffer[i] = -INF;
    }
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<Vec2i> face = model->face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec3f tex_coords[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model->vert(face[j].x);
            Vec3f vt = model->texture(face[j].y);
            world_coords[j] = v0;
            // screen_coords[j] = Vec3f((v0.x + 1) * width / 2, (v0.y + 1) * height / 2, v0.z);
            tex_coords[j] = Vec3f(vt.x * tex.get_width(), vt.y * tex.get_height(), 0.);
            screen_coords[j] = Vec3f(((v0.x / (1 - v0.z / camera.z)) + 1) * width / 2 , ((v0.y / (1 - v0.z / camera.z)) + 1) * height / 2, v0.z / (1 - v0.z / camera.z));
            // tex_coords[j] = Vec3f(vt.x * tex.get_width(), vt.y * tex.get_height(), 0.);
        }
        // 计算的不是面的法线, 而是面的法线的反向向量, 因为要和入射光的方向点乘得到光照强度
        Vec3f n = ((world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0])).normalize();
        float intensity = n * light_dir;
        // intensity *= intensity;
        if (intensity > 0)
        {
            triangle(screen_coords, zbuffer, image, tex, tex_coords, intensity);
        }
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}

// with z-buffer
void triangle(Vec3f *screen_coords, float *zbuffer, TGAImage &image, TGAColor color)
{
    float max[2], min[2];
    min[0] = width;
    min[1] = height;
    max[0] = max[1] = 0;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            max[j] = screen_coords[i].raw[j] < max[j] ? max[j] : screen_coords[i].raw[j];
            min[j] = screen_coords[i].raw[j] < min[j] ? screen_coords[i].raw[j] : min[j];
        }
    }
    for (int i = min[0]; i <= max[0]; ++i)
    {
        for (int j = min[1]; j <= max[1]; ++j)
        {
            Vec3f barycentric_coords = Barycentric(screen_coords, Vec2f(i, j));
            if (barycentric_coords.x >= 0 && barycentric_coords.y >= 0 && barycentric_coords.z >= 0)
            {
                float z_new = screen_coords[0].z * barycentric_coords.x + screen_coords[1].z * barycentric_coords.y + screen_coords[2].z * barycentric_coords.z;
                if (z_new > zbuffer[i * width + j])
                {
                    image.set(i, j, color);
                    zbuffer[i * width + j] = z_new;
                }
            }
        }
    }
}

Vec3f Barycentric(Vec3f *vertex, Vec2f p)
{
    Vec3f res;
    Vec2f v[3];
    for (int i = 0; i < 3; ++i)
    {
        v[i].x = vertex[i].x;
        v[i].y = vertex[i].y;
    }
    res.raw[0] = ((v[1] - p) ^ (v[2] - p)) / ((v[1] - v[0]) ^ (v[2] - v[0]));
    res.raw[1] = ((v[2] - p) ^ (v[0] - p)) / ((v[2] - v[1]) ^ (v[0] - v[1]));
    res.raw[2] = 1 - res.x - res.y;
    return res;
}

void triangle(Vec3f *screen_coords, float *zbuffer, TGAImage &image, TGAImage &tex, Vec3f *tex_coords, float &intensity)
{
    float max[2], min[2];
    min[0] = width;
    min[1] = height;
    max[0] = max[1] = 0;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            max[j] = screen_coords[i].raw[j] < max[j] ? max[j] : screen_coords[i].raw[j];
            min[j] = screen_coords[i].raw[j] < min[j] ? screen_coords[i].raw[j] : min[j];
        }
    }
    for (int i = min[0]; i <= max[0]; ++i)
    {
        for (int j = min[1]; j <= max[1]; ++j)
        {
            Vec3f barycentric_coords = Barycentric(screen_coords, Vec2f(i, j));
            if (barycentric_coords.x >= 0 && barycentric_coords.y >= 0 && barycentric_coords.z >= 0)
            {
                float z_new = screen_coords[0].z * barycentric_coords.x + screen_coords[1].z * barycentric_coords.y + screen_coords[2].z * barycentric_coords.z;
                if (z_new > zbuffer[i * width + j])
                {
                    int u, v;
                    u = tex_coords[0].x * barycentric_coords.x + tex_coords[1].x * barycentric_coords.y + tex_coords[2].x * barycentric_coords.z;
                    v = tex_coords[0].y * barycentric_coords.x + tex_coords[1].y * barycentric_coords.y + tex_coords[2].y * barycentric_coords.z;
                    TGAColor color = tex.get(u, v);
                    for (int k = 0; k < 3; ++k)
                    {
                        color.raw[k] *= intensity;
                    }
                    image.set(i, j, color);
                    zbuffer[i * width + j] = z_new / (1 - z_new / camera.z);
                }
            }
        }
    }
}