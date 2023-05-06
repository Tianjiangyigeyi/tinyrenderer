#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	//顶点
	std::vector<Vec3f> verts_;
	//面
	std::vector<std::vector<Vec2i> > faces_;
	//纹理
	std::vector<Vec3f> textures_;

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int ntextures();
	Vec3f vert(int i);
	std::vector<Vec2i> face(int idx);
	Vec3f texture(int i);
};

#endif //__MODEL_H__
