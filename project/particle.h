#pragma once

#include <vector>
#include "objLoader.h"

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Particle {    // 粒子（定义为结构体而不是类是为了方便利用结构体的整体赋值）
	Vector3 pos;                         // 粒子当前位置
	Vector3 prevPos;                 // 粒子上一次更新时的位置
	Vector3 velocity;                 // 粒子前速度
	Vector3 acceleration;         // 粒子当前加速度

	float lifetime;    // 粒子已存活的时间
	float totalLifetime;    // 粒子的最长生存时间，存活时间超出此时间的粒子将被删除

	float color[4];             // 粒子的颜色，包含alpha值
	Particle *prev;    // Particle结构体被组织为双向链表
	Particle *next;

	Obj *model;    // 粒子所使用的模型
	GLuint texture;    // 粒子所使用的纹理
};
void moveParticle(Particle *, float millis);    // 计算粒子经过millis秒后的新状态并更新粒子

class ParticleSystem {    // 粒子系统
public:
	float initialVelocity;    // 粒子初速度大小
	float elevation;    // 粒子初始仰角大小，单位为弧度

	int modelSelect;    // 模型切换
	int waterFlowCount;    // 水流数目

	ParticleSystem(int maxParticles, Vector3 origin);    // 参数分别是粒子的最大数目和粒子系统的坐标位置
	void update(float elapsedTime);    // 更新粒子系统中粒子的状态
	void render();    // 绘制所有粒子
	void initializeSystem();    // 初始化粒子系统，创建空白粒子并赋值给particleBuffer
	void killSystem();    // 释放粒子系统分配的内存空间
	~ParticleSystem();
protected:
	Particle *particleBuffer;    // 初始化时为创建足够多的粒子作为缓存
	Particle *particleList;             // 粒子系统中的粒子列表
	Obj *particleModel;    // 新粒子使用的模型
	GLuint particleTexture;    // 新粒子使用的纹理
	int maxParticles;           // 粒子的最大数目（也是particleBuffer的长度）
	int numParticles;        // 当前的粒子数目（也是particleList的长度）
	Vector3 origin;                    // 粒子系统的坐标，用于整体平移
	Vector3 force;                     // 系统中的加速度（例如重力、风力等）

	float lastUpdateTime;    // 上次更新粒子系统（调用update）的时间（秒）

	float newParticlesPerSecond;    // 每秒钟产生多少新的粒子
	float accumulatedTime;    // 上一次添加粒子的（调用addParticle）时间

	void addParticle(float);    // 添加新的粒子（从particleBuffer中取出，放到particleList中）
	void deleteParticle(Particle *);    // 删除粒子（从particleList中取出，放到particleBuffer中）
};
