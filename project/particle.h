#pragma once

#include <vector>
#include "objLoader.h"

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Particle {    // ���ӣ�����Ϊ�ṹ�����������Ϊ�˷������ýṹ������帳ֵ��
	Vector3 pos;                         // ���ӵ�ǰλ��
	Vector3 prevPos;                 // ������һ�θ���ʱ��λ��
	Vector3 velocity;                 // ����ǰ�ٶ�
	Vector3 acceleration;         // ���ӵ�ǰ���ٶ�

	float lifetime;    // �����Ѵ���ʱ��
	float totalLifetime;    // ���ӵ������ʱ�䣬���ʱ�䳬����ʱ������ӽ���ɾ��

	float color[4];             // ���ӵ���ɫ������alphaֵ
	Particle *prev;    // Particle�ṹ�屻��֯Ϊ˫������
	Particle *next;

	Obj *model;    // ������ʹ�õ�ģ��
	GLuint texture;    // ������ʹ�õ�����
};
void moveParticle(Particle *, float millis);    // �������Ӿ���millis������״̬����������

class ParticleSystem {    // ����ϵͳ
public:
	float initialVelocity;    // ���ӳ��ٶȴ�С
	float elevation;    // ���ӳ�ʼ���Ǵ�С����λΪ����

	int modelSelect;    // ģ���л�
	int waterFlowCount;    // ˮ����Ŀ

	ParticleSystem(int maxParticles, Vector3 origin);    // �����ֱ������ӵ������Ŀ������ϵͳ������λ��
	void update(float elapsedTime);    // ��������ϵͳ�����ӵ�״̬
	void render();    // ������������
	void initializeSystem();    // ��ʼ������ϵͳ�������հ����Ӳ���ֵ��particleBuffer
	void killSystem();    // �ͷ�����ϵͳ������ڴ�ռ�
	~ParticleSystem();
protected:
	Particle *particleBuffer;    // ��ʼ��ʱΪ�����㹻���������Ϊ����
	Particle *particleList;             // ����ϵͳ�е������б�
	Obj *particleModel;    // ������ʹ�õ�ģ��
	GLuint particleTexture;    // ������ʹ�õ�����
	int maxParticles;           // ���ӵ������Ŀ��Ҳ��particleBuffer�ĳ��ȣ�
	int numParticles;        // ��ǰ��������Ŀ��Ҳ��particleList�ĳ��ȣ�
	Vector3 origin;                    // ����ϵͳ�����꣬��������ƽ��
	Vector3 force;                     // ϵͳ�еļ��ٶȣ����������������ȣ�

	float lastUpdateTime;    // �ϴθ�������ϵͳ������update����ʱ�䣨�룩

	float newParticlesPerSecond;    // ÿ���Ӳ��������µ�����
	float accumulatedTime;    // ��һ��������ӵģ�����addParticle��ʱ��

	void addParticle(float);    // ����µ����ӣ���particleBuffer��ȡ�����ŵ�particleList�У�
	void deleteParticle(Particle *);    // ɾ�����ӣ���particleList��ȡ�����ŵ�particleBuffer�У�
};
