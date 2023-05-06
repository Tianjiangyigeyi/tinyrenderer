#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#define INF 0x3f3f3f3f

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void triangle(Vec3f *screen_coords, float *zbuffer, TGAImage &image, TGAColor color);
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
bool InsideTriangle(Vec2i t0, Vec2i t1, Vec2i t2, int x, int y);
Vec3f Barycentric(Vec3f *vertex, Vec2f p);

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
    Vec3f light_dir(0, 0, -1);
    float *zbuffer = new float[width * height];
    for (int i = 0; i < width * height; ++i)
    {
        zbuffer[i] = -INF;
    }
    /////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        // Vec2i screen_coords[3];
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model->vert(face[j]);
            world_coords[j] = v0;
            // screen_coords[j] = Vec2i((v0.x + 1) * width / 2, (v0.y + 1) * height / 2);
            // from [-1,1] to [0, width or height]
            screen_coords[j] = Vec3f((v0.x + 1) * width / 2, (v0.y + 1) * height / 2, v0.z);
        }
        // 计算的不是面的法线, 而是面的法线的反向向量, 因为要和入射光的方向点乘得到光照强度
        Vec3f n = ((world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0])).normalize();
        float intensity = n * light_dir;
        intensity *= intensity;
        if (intensity > 0)
        {
            // triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
            triangle(screen_coords, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}

bool InsideTriangle(Vec2i t0, Vec2i t1, Vec2i t2, int x, int y)
{
    Vec2i tx = Vec2i(x, y);
    int cross1 = (tx - t0) ^ (t1 - t0);
    int cross2 = (tx - t1) ^ (t2 - t1);
    int cross3 = (tx - t2) ^ (t0 - t2);
    if (cross1 * cross2 < 0 || cross2 * cross3 < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    int max_x = 0, max_y = 0, min_x = width, min_y = height;
    if (t0.y > t1.y)
        std::swap(t0, t1);
    if (t0.y > t2.y)
        std::swap(t0, t2);
    if (t1.y > t2.y)
        std::swap(t1, t2);
    max_x = t0.x > max_x ? t0.x : max_x;
    min_x = t0.x < min_x ? t0.x : min_x;
    max_x = t1.x > max_x ? t1.x : max_x;
    min_x = t1.x < min_x ? t1.x : min_x;
    max_x = t2.x > max_x ? t2.x : max_x;
    min_x = t2.x < min_x ? t2.x : min_x;

    // max_y = t0.y > max_y ? t0.y : max_y;
    min_y = t0.y;
    // max_y = t1.y > max_y ? t1.y : max_y;
    // min_y = t1.y < min_y ? t1.y : min_y;
    max_y = t2.y;
    // min_y = t2.y < min_y ? t2.y : min_y;

    for (int i = min_x; i < max_x; ++i)
    {
        for (int j = min_y; j < max_y; ++j)
        {
            if (InsideTriangle(t0, t1, t2, i, j))
            {
                image.set(i, j, color);
            }
        }
    }
}

//with z-buffer
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
    for (int i = min[0]; i <= max[0] ; ++i)
    {
        for (int j = min[1]; j <= max[1] ; ++j)
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