#pragma once

#include <vector>
#include "objLoader.h"

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Particle {
	Vector3 pos;                         //current position of the particle
	Vector3 prevPos;                 //last position of the particle
	Vector3 velocity;                 //direction and speed
	Vector3 acceleration;         // acceleration

	float lifetime;    // 粒子已存活的时间
	float totalLifetime;    // 粒子的最长生存时间

	float energy;                   //determines how long the particle is alive
	float size;                        // size of the particle
	float sizeDelta;            // amount to change the size over time

	float weight;               //determines how gravity affects the particle
	float weightDelta;      //change over time
	float color[4];             //current color of the particle
	float colorDelta[4];    //how the color and transparency change with time

	Particle *prev;
	Particle *next;

	Obj *model;
	GLuint texture;
};
void moveParticle(Particle *, float millis);

class ParticleSystem {
public:
	float initialVelocity;    // 粒子初速度大小
	float elevation;    // 粒子仰角大小，单位为弧度

	int modelSelect;    // 模型切换

	ParticleSystem(int maxParticles, Vector3 origin);
	void update(float elapsedTime);
	void render();
	int emit(int numParticles);
	void initializeSystem();
	void killSystem();
	~ParticleSystem();
protected:
	void initializeParticle(int index);
	Particle *particleBuffer;
	Particle *particleList;             // particles for this emitter
	Obj *particleModel;
	GLuint particleTexture;
	int maxParticles;           // maximum number of particles in total
	int numParticles;        // indicies of all free particles
	Vector3 origin;                    // center of the particle system
	Vector3 force;                     // force (gravity, wind, etc.) acting on the system

	float lastUpdateTime;    // last time when updated
	float accumulatedTime; // track when was last particle emitted

	float newParticlesPerSecond;    // how much new particles generated per second

	void addParticle(float);
	void deleteParticle(Particle *);
};

/* ========================================================================================== */

/*
class Fountain : public ParticleSystem {
public:
	Fountain(int maxParticles, Vector3 origin);
	void update(float elapsedTime);
	void render();
	int emit(int numParticles);
	void initializeSystem();
	void killSystem();
	~Fountain();
};
*/