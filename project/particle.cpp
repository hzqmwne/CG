#include <GL/glut.h>
#include <GL/glaux.h>
#include <cmath>
#include "particle.h"
#include "objLoader.h"

const GLfloat Pi = 3.1415926f;

ParticleSystem::ParticleSystem(int maxParticles, Vector3 origin) {
	this->particleList = NULL;
	this->particleBuffer = NULL;
	this->numParticles = maxParticles;
	this->origin = origin;
	this->lastUpdateTime = 0;
	this->accumulatedTime = 0;
	this->newParticlesPerSecond = 4000;
	this->force = { 0, 0, (float)-9.8 };
	this->particleTexture = LoadGLTexture("color.bmp");
	//this->particleModel = LoadModel("model.obj");
	this->particleModel = NULL;
	this->initialVelocity = 13;
	this->elevation = Pi / 2.0*75.0 / 90.0;
}

void ParticleSystem::update(float elapsedTime) {
	Particle *p = this->particleList;
	while(p) {
		//printf("update x:%f y:%f z:%f\n", p->pos.x, p->pos.y, p->pos.z);
		Particle *next = p->next;
		float millis = elapsedTime;
		p->prevPos = p->pos;
		Vector3 prevVelocity = p->velocity;
		p->velocity.x += p->acceleration.x * millis;
		p->velocity.y += p->acceleration.y * millis;
		p->velocity.z += p->acceleration.z * millis;
		p->pos.x += 0.5 * (prevVelocity.x + p->velocity.x) * millis;
		p->pos.y += 0.5 * (prevVelocity.y + p->velocity.y) * millis;
		p->pos.z += 0.5 * (prevVelocity.z + p->velocity.z) * millis;
		float downBound = 0;
		if (p->pos.z <= downBound) {
			this->deleteParticle(p);
		}
		p = next;
	}
	this->lastUpdateTime += elapsedTime;
	float count = (int)((this->lastUpdateTime - this->accumulatedTime) * this->newParticlesPerSecond);
	//printf("count %f\n", count);
	if (count > 0) {
		for (int i = 0; i < count; ++i) {
			this->addParticle();
		}
		this->accumulatedTime = this->lastUpdateTime;
	}
}

void ParticleSystem::render() {
	for (Particle *p = this->particleList; p; p = p->next) {
		glPushMatrix();
		glColor3f(p->color[0], p->color[1], p->color[2]);
		glTranslatef(this->origin.x + p->pos.x, this->origin.y + p->pos.y, this->origin.z + p->pos.z);
		glBindTexture(GL_TEXTURE_2D, p->texture);
		if (p->model != NULL) {
			p->model->draw(0.05);
		}
		else {
			glutSolidSphere(0.02, 10, 10);
		}
		glPopMatrix();
	}
}

int ParticleSystem::emit(int numParticles) {
	return 0;
}

void ParticleSystem::initializeParticle(int index) {
	
}

void ParticleSystem::initializeSystem() {
	for (int i = 0; i < this->numParticles; ++i) {
		Particle *newParticle = new Particle;
		newParticle->next = particleBuffer;
		particleBuffer = newParticle;
	}
}

void ParticleSystem::killSystem() {
	Particle *p;
	Particle *next;
	p = this->particleList;
	while (p) {
		next = p->next;
		delete p;
		p = p->next;
	}
	p = this->particleBuffer;
	while (p) {
		next = p->next;
		delete p;
		p = p->next;
	}
	this->particleList = NULL;
	this->particleBuffer = NULL;
	glDeleteTextures(1, &(this->particleTexture));
}

ParticleSystem::~ParticleSystem() {
	this->killSystem();
}

void ParticleSystem::addParticle() {
	Particle *p = this->particleBuffer;
	if (!p) {
		return;
	}
	this->particleBuffer = p->next;

	p->pos = { 0.1f*rand() / RAND_MAX, 0.1f*rand() / RAND_MAX, 0.5f*rand() / RAND_MAX };
	float theta = (2 * Pi * rand() * 1.0) / RAND_MAX;
	p->model = this->particleModel;
	p->texture = this->particleTexture;
	float v_horizontal = this->initialVelocity * cos(this->elevation);
	float v_vertical = this->initialVelocity * sin(this->elevation);
	//p->velocity = {2*cos(theta),2*sin(theta),12};
	p->velocity = { v_horizontal * cos(theta), v_horizontal * sin(theta), v_vertical };
	p->acceleration = this->force;
	p->prevPos = p->pos;
	p->color[0] = 0.9;
	p->color[1] = 0.9;
	p->color[2] = 0.9;
	p->color[3] = 0;

	p->next = this->particleList;
	p->prev = NULL;
	if (this->particleList) {
		this->particleList->prev = p;
	}
	this->particleList = p;
}

void ParticleSystem::deleteParticle(Particle *p) {
	if (p->next) {
		p->next->prev = p->prev;
	}
	if (p->prev) {
		p->prev->next = p->next;
	}
	else {
		if (p == particleList) {
			this->particleList = p->next;
			this->particleList->prev = NULL;
		}
	}
	p->next = this->particleBuffer;
	this->particleBuffer = p;
}

/* ================================================================================ */

/*
Fountain::Fountain(int maxParticles, Vector3 origin) :ParticleSystem(maxParticles, origin) {
	this->force = { 0, 0, (float)-9.8 };
}

void Fountain::update(float elapsedTime) {
	for (int i = 0; i < maxParticles; ++i) {
	
		Particle p = this->particleList[i];
		float oldx = x;
		float oldy = y;
		float oldz = z;
		float oldvx = vx;
		float oldvy = vy;
		float oldvz = vz;
		vx += ax*millis;
		vz += az*millis;
		x += 0.5*(oldvx + vx)*millis;
		z += 0.5*(oldvz + vz)*millis;
		if (oldvz > 0 && vz <= 0 && z > 0 + R && z*0.9*0.9 < 0 + R) {
			downBound = 0 + R;
			printf("===new dowmBound\n");
		}
		if (x >= rightBound) {    // 处理反弹
			x = oldx;
			vx = -oldvx;
		}
		if (x <= leftBound) {
			x = oldx;
			vx = -oldvx;
		}
		if (z >= upBound) {
			z = oldz;
			vz = -oldvz;
		}
		if (z <= downBound) {    // 每次触底，都要损失大约20%的能量
			z = oldz;
			vx = 0.9*oldvx;
			vy = 0.9*oldvy;
			vz = -0.9*oldvz;
		}
		
	}
}

void Fountain::render() {

}

int Fountain::emit(int numParticles) {
	return 0;
}

void Fountain::initializeSystem() {

}

void Fountain::killSystem() {

}

Fountain::~Fountain() {

}
*/