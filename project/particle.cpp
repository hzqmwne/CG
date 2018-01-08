#include <GL/glut.h>
#include <GL/glaux.h>
#include <cmath>
#include "particle.h"
#include "objLoader.h"

const GLfloat Pi = 3.1415926f;

void moveParticle(Particle *p, float millis) {
	p->prevPos = p->pos;
	Vector3 prevVelocity = p->velocity;
	p->velocity.x += p->acceleration.x * millis;
	p->velocity.y += p->acceleration.y * millis;
	p->velocity.z += p->acceleration.z * millis;
	p->pos.x += 0.5 * (prevVelocity.x + p->velocity.x) * millis;
	p->pos.y += 0.5 * (prevVelocity.y + p->velocity.y) * millis;
	p->pos.z += 0.5 * (prevVelocity.z + p->velocity.z) * millis;
}

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
	this->particleModel = LoadModel("model.obj");
	this->initialVelocity = 12;
	this->elevation = Pi / 180.0 * 75.0;
	this->modelSelect = 1;
}

void ParticleSystem::update(float elapsedTime) {
	Particle *p = this->particleList;
	while(p) {
		//printf("update x:%f y:%f z:%f\n", p->pos.x, p->pos.y, p->pos.z);
		Particle *next = p->next;
		moveParticle(p, elapsedTime);
		p->lifetime += elapsedTime;
		p->color[3] = pow(1 - p->lifetime / p->totalLifetime, 2);
		float downBound = 0;
		//if (p->pos.z <= downBound) {
		if (p->lifetime > p->totalLifetime) {
			this->deleteParticle(p);
		}
		p = next;
	}
	this->lastUpdateTime += elapsedTime;
	float delayTime = this->lastUpdateTime - this->accumulatedTime;
	int count = (int)(delayTime * this->newParticlesPerSecond);
	//printf("count %f\n", count);
	if (count > 0) {
		for (int i = 0; i < count; ++i) {
			this->addParticle(5*delayTime / count * i);
		}
		this->accumulatedTime = this->lastUpdateTime;
	}
}

void ParticleSystem::render() {
	for (Particle *p = this->particleList; p; p = p->next) {
		//glDisable(GL_TEXTURE_2D);
		glPushMatrix();
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(p->color[0], p->color[1], p->color[2], p->color[3]);
		GLfloat ambientAndDiffuse[] = { 0.2, 0.2, 0.2, p->color[3] };    // 漫反射光
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);
		GLfloat specular[] = { 0.8,0.8,0.8,p->color[3] };   // 镜面反射光
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		GLfloat shininess = 0.8;    // 镜面反射系数
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
		glTranslatef(this->origin.x + p->pos.x, this->origin.y + p->pos.y, this->origin.z + p->pos.z);
		glBindTexture(GL_TEXTURE_2D, p->texture);
		/*
		glBegin(GL_POLYGON);
		glVertex3f(0,0,0);
		glVertex3f(0,0.3,0);
		glVertex3f(0.3,0,0);
		glEnd();
		*/
		if (this->modelSelect == 1) {
			if (p->model != NULL) {
				p->model->draw(0.05);
			}
		}
		else if(this->modelSelect == 2) {
			glutSolidSphere(0.03, 10, 10);
		}
		glDepthMask(GL_TRUE);
		glPopMatrix();
		//glEnable(GL_TEXTURE_2D);
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

void ParticleSystem::addParticle(float delay) {
	//printf("delay %f\n", delay);
	Particle *p = this->particleBuffer;
	if (!p) {
		return;
	}
	this->particleBuffer = p->next;

	int num = RAND_MAX - RAND_MAX + 4;
	float disturbance = Pi / 180.0 * (rand()*1.0 / RAND_MAX * 2 - 1);
	float theta = (2.0 * Pi) * (rand() % num) / num + disturbance;
	p->model = this->particleModel;
	p->texture = this->particleTexture;
	float v_horizontal = this->initialVelocity * cos(this->elevation + disturbance);
	float v_vertical = this->initialVelocity * sin(this->elevation + disturbance);
	//p->velocity = {2*cos(theta),2*sin(theta),12};
	p->velocity = { v_horizontal * cos(theta), v_horizontal * sin(theta), v_vertical };
	//p->pos = { 0.1f*rand() / RAND_MAX, 0.1f*rand() / RAND_MAX, 0.1f*rand() / RAND_MAX };
	p->pos = { 0, 0, 0 };
	moveParticle(p, delay);
	p->acceleration = this->force;
	p->prevPos = p->pos;
	p->totalLifetime = abs(2.0*v_vertical / p->acceleration.z);
	p->lifetime = 0;
	p->color[0] = 0.9;
	p->color[1] = 0.9;
	p->color[2] = 0.9;
	p->color[3] = 1;

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