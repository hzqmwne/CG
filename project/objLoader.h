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
	std::vector<Point> v;    // ���еĵ�
	std::vector<Texture>  vt;    // ���е�����
	std::vector<NormalVector> vn;    // ���еķ�����
	std::vector<Surface> f;    // ���е���
	Obj(std::fstream &);
	void draw(float);
};

Obj *LoadModel(char *filename);

/* =========================================================================== */

unsigned int LoadGLTexture(char *filename);