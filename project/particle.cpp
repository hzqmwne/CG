#include <GL/glut.h>
#include <GL/glaux.h>
#include <cmath>
#include "particle.h"
#include "objLoader.h"

const GLfloat Pi = 3.1415926f;

void moveParticle(Particle *p, float millis) {    // 更新粒子的运动学属性
	p->prevPos = p->pos;
	Vector3 prevVelocity = p->velocity;
	p->velocity.x += p->acceleration.x * millis;
	p->velocity.y += p->acceleration.y * millis;
	p->velocity.z += p->acceleration.z * millis;
	p->pos.x += 0.5 * (prevVelocity.x + p->velocity.x) * millis;
	p->pos.y += 0.5 * (prevVelocity.y + p->velocity.y) * millis;
	p->pos.z += 0.5 * (prevVelocity.z + p->velocity.z) * millis;
}

ParticleSystem::ParticleSystem(int maxParticles, Vector3 origin) {    // 初始化必要的参数
	this->particleList = NULL;
	this->particleBuffer = NULL;
	this->numParticles = maxParticles;
	this->origin = origin;
	this->lastUpdateTime = 0;
	this->accumulatedTime = 0;
	this->newParticlesPerSecond = 4000;
	this->force = { 0, 0, (float)-9.8 };
	this->particleTexture = LoadGLTexture("water.bmp");
	this->particleModel = LoadModel("model.obj");
	this->initialVelocity = 12;
	this->elevation = Pi / 180.0 * 75.0;    // 初始仰角设为75度
	this->modelSelect = 1;
	this->waterFlowCount = 4;
}

void ParticleSystem::update(float elapsedTime) {
	Particle *p = this->particleList;
	while(p) {
		Particle *next = p->next;
		moveParticle(p, elapsedTime);    // 更新粒子
		p->lifetime += elapsedTime;
		p->color[3] = pow(1 - p->lifetime / p->totalLifetime, 2);
		if (p->lifetime > p->totalLifetime) {    // 删除超过生命周期的粒子
			this->deleteParticle(p);
		}
		p = next;
	}
	this->lastUpdateTime += elapsedTime;
	float delayTime = this->lastUpdateTime - this->accumulatedTime;    // 计算距离上一次添加新粒子的时间间隔
	int count = (int)(delayTime * this->newParticlesPerSecond);    // 计算这段时间新增的粒子数量
	if (count > 0) {
		for (int i = 0; i < count; ++i) {
			this->addParticle(5*delayTime / count * i);
		}
		this->accumulatedTime = this->lastUpdateTime;
	}
}

void ParticleSystem::render() {
	for (Particle *p = this->particleList; p; p = p->next) {
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
		glBindTexture(GL_TEXTURE_2D, p->texture);    // 纹理
		if (this->modelSelect == 1) {    // 模型选择
			if (p->model != NULL) {
				p->model->draw(0.05);
			}
		}
		else if(this->modelSelect == 2) {
			glutSolidSphere(0.03, 10, 10);
		}
		glDepthMask(GL_TRUE);
		glPopMatrix();
	}
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
	Particle *p = this->particleBuffer;
	if (!p) {
		return;
	}
	this->particleBuffer = p->next;

	int num = this->waterFlowCount;
	float disturbance = Pi / 180.0 * (rand()*1.0 / RAND_MAX * 2 - 1);    // 对初速度的微扰
	float theta = (2.0 * Pi) * (rand() % num) / num + disturbance;
	p->model = this->particleModel;
	p->texture = this->particleTexture;
	float v_horizontal = this->initialVelocity * cos(this->elevation + disturbance);
	float v_vertical = this->initialVelocity * sin(this->elevation + disturbance);
	p->velocity = { v_horizontal * cos(theta), v_horizontal * sin(theta), v_vertical };
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
