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

class ParticleSystem {
public:
	float initialVelocity;    // ���ӳ��ٶȴ�С
	float elevation;    // �������Ǵ�С����λΪ����

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

	void addParticle();
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