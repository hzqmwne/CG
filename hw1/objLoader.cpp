#include "objLoader.h"
#include <string>
#include <gl/glut.h>

Obj::Obj(std::fstream &ifs) {
	std::string s;
	while (getline(ifs, s)) {
		if (s.length()<2)continue;
		if (s[0] == 'v') {
			if (s[1] == 't') {//vt 0.581151 0.979929 纹理
				std::istringstream in(s);
				Texture tex;
				std::string head;
				in >> head >> tex.tu >> tex.tv;
				vt.push_back(tex);
			}
			else if (s[1] == 'n') {//vn 0.637005 -0.0421857 0.769705 法向量
				std::istringstream in(s);
				NormalVector nor;
				std::string head;
				in >> head >> nor.nx >> nor.ny >> nor.nz;
				vn.push_back(nor);
			}
			else {//v -53.0413 158.84 -135.806 点
				std::istringstream in(s);
				Point poi;
				std::string head;
				in >> head >> poi.x >> poi.y >> poi.z;
				v.push_back(poi);
			}
		}
		else if (s[0] == 'f') {//f 2443//2656 2442//2656 2444//2656 面
			for (int k = s.size() - 1; k >= 0; k--) {
				if (s[k] == '/')s[k] = ' ';
			}
			std::istringstream in(s);
			Surface sur;
			std::string head;
			in >> head;
			int i = 0;
			while (i<3) {
				if (v.size() != 0) {
					in >> sur.v[i];
					sur.v[i] -= 1;
				}
				if (vt.size() != 0) {
					in >> sur.t[i];
					sur.t[i] -= 1;
				}
				if (vn.size() != 0) {
					in >> sur.n[i];
					sur.n[i] -= 1;
				}
				i++;
			}
			f.push_back(sur);
		}
	}

}

void Obj::draw(int yu) {    // 绘制模型，yu是放大倍数，用于调整大小
	for (int i = 0; i<f.size(); i++) {
		//glBegin(GL_POINTS);
		glBegin(GL_POLYGON);
		for (int j = 0; j < 3; j++) {
			if (vt.size() != 0)glTexCoord2f(vt[f[i].t[j]].tu, vt[f[i].t[j]].tv);  //纹理    
			if (vn.size() != 0)glNormal3f(vn[f[i].n[j]].nx, vn[f[i].n[j]].ny, vn[f[i].n[j]].nz);//法向量
			glVertex3f(v[f[i].v[j]].x * yu, v[f[i].v[j]].y * yu, v[f[i].v[j]].z * yu);        // 上顶点
			//printf("%f %f %f\n", v[f[i].v[j]].x * yu, v[f[i].v[j]].y * yu, v[f[i].v[j]].z * yu);
		}
		glEnd();// 三角形绘制结束    


				/*if(m_pic.VN.size()!=0){
				glBegin(GL_LINES);                            // 绘制三角形
				glVertex3f(m_pic.V[m_pic.F[i].V[0]].X/YU,m_pic.V[m_pic.F[i].V[0]].Y/YU, m_pic.V[m_pic.F[i].V[0]].Z/YU);        // 上顶点
				glVertex3f(m_pic.V[m_pic.F[i].V[0]].X/YU+m_pic.VN[m_pic.F[i].N[0]].NX
				,m_pic.V[m_pic.F[i].V[0]].Y/YU+m_pic.VN[m_pic.F[i].N[0]].NY
				, m_pic.V[m_pic.F[i].V[0]].Z/YU+m_pic.VN[m_pic.F[i].N[0]].NZ);                    // 左下
				glEnd();                                // 三角形绘制结束
				}*/
	}

}
