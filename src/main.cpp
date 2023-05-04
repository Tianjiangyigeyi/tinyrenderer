#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
bool InsideTriangle(Vec2i t0, Vec2i t1, Vec2i t2, int x, int y);

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
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model->vert(face[j]);
            world_coords[j] = v0;
            screen_coords[j] = Vec2i((v0.x + 1) * width / 2, (v0.y + 1) * height / 2);
        }
        Vec3f n = ((world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0])).normalize();
        float intensity = n * light_dir;
        if (intensity > 0)
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
    }

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

