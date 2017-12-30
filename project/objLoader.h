#pragma once

#include <vector>
#include <fstream>
#include <sstream>

struct Point {
	float x;
	float y;
	float z;
};

struct Texture {
	float tu;
	float tv;
};

struct NormalVector {
	float nx;
	float ny;
	float nz;
};

struct Surface {
	int v[3];
	int t[3];
	int n[3];
};

class Obj {
public:
	std::vector<Point> v;    // 所有的点
	std::vector<Texture>  vt;    // 所有的纹理
	std::vector<NormalVector> vn;    // 所有的法向量
	std::vector<Surface> f;    // 所有的面
	Obj(std::fstream &);
	void draw(float);
};

Obj *LoadModel(char *filename);

/* =========================================================================== */

unsigned int LoadGLTexture(char *filename);