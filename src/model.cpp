#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

/**
 * @brief Construct a new Model:: Model object, 读入顶点信息和面的信息, 不涉及面的纹理和法线
 * 输出一条cerr提示程序员读入的顶点个数和面个数
 *
 * @param filename 文件的相对地址
 */
Model::Model(const char *filename) : verts_(), faces_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
        return;
    std::string line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++)
                iss >> v.raw[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 2, "f "))
        {
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash)
            {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model()
{
}

/**
 * @brief 返回顶点的数量
 *
 * @return int
 */
int Model::nverts()
{
    return (int)verts_.size();
}

/**
 * @brief 返回面的数量
 *
 * @return int
 */
int Model::nfaces()
{
    return (int)faces_.size();
}

/**
 * @brief 返回idx对应的面
 *
 * @param idx
 * @return std::vector<int>
 */
std::vector<int> Model::face(int idx)
{
    return faces_[idx];
}

/**
 * @brief 返回i对应的顶点
 *
 * @param i
 * @return Vec3f
 */
Vec3f Model::vert(int i)
{
    return verts_[i];
}
