#include <GL/glut.h>
#include <GL/glaux.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include "particle.h"
#include "waveIn.h"

float windowWidth = 400.0f, windowHeight = 400.0f;  //��Ļ��ȣ��߶�
const GLfloat Pi = 3.1415926f;  //����Բ����  
const int N = 50;
unsigned int timeInterval = 33;    // �����˶���ʱ����

ParticleSystem *fountain = NULL;

WaveIn *waveIn = NULL;

/* =============================================================== */

int pcm_db_count(const unsigned char* ptr, size_t size) {
	int ndb = 0;

	short int value;

	int i;
	long v = 0;
	for (i = 0; i < (int)size; i += 2) {
		value = *(short *)(ptr + i);
		v += abs(value);
	}

	v = v / (size / 2);

	if (v != 0) {
		ndb = (int)(20.0*log10((double)v / 65535.0));
	}
	else {
		ndb = -96;
	}

	return ndb;
}

static BOOL CALLBACK WaveInProc(HWAVEIN hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
	// ���Դ򿪺͹ر��豸ʱ�Ĵ���
	if (uMsg == WIM_DATA) {
		LPWAVEHDR pHdr = (LPWAVEHDR)dwParam1;

		MMRESULT mmres = waveInUnprepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		//�������� 
		int volume = pcm_db_count((unsigned char *)(pHdr->lpData), pHdr->dwBytesRecorded);
		printf("%d\n", volume);
		fountain->elevation = Pi / 2.0 * pow((volume + 96.0) / 96.0, 0.3);
		fountain->initialVelocity = 2.0 / Pi * 13 * fountain->elevation;
		/*
		if (volume > -40) {
			ball.vx += (sgn(ball.vx) == 0 ? 1 : sgn(ball.vx))*0.01;
			ball.vz += (sgn(ball.vz) == 0 ? 1 : sgn(ball.vz))*0.03;
			printf("volume %d\n", volume);
		}
		*/
		//����׼������
		mmres = waveInPrepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		mmres = waveInAddBuffer(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
	}
	return TRUE;
}

/* ======================================================================= */

void init(void) {
	srand(time(0));
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	fountain = new ParticleSystem(10000, {0,0,0});
	fountain->initializeSystem();

	waveIn = new WaveIn((DWORD_PTR)WaveInProc);
	waveIn->open();
	waveIn->start();
}

void drawAxis() {    // ������������
	glBegin(GL_LINES);
	glVertex3f(-5, 0, 0);
	glVertex3f(5, 0, 00);
	glVertex3f(0, -5, 0);
	glVertex3f(0, 5, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 10);

	glVertex3f(1, 0, 0.5);
	glVertex3f(1, 0, -0.5);
	glVertex3f(-0.5, 0, 1);
	glVertex3f(0.5, 0, 1);
	glVertex3f(-0.5, 1, 0);
	glVertex3f(0.5, 1, 0);
	glEnd();
}

void display(void) {    // ҳ�滺��ˢ��ʱ�Ļص�����
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // �����ɫ����Ȼ���

	glPushMatrix();

	glEnable(GL_BLEND);  //���û�Ϲ��ܣ���ͼ����Χ��ɫ���
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //�㿹���
	glEnable(GL_LINE_SMOOTH);  //�߿����
	glEnable(GL_POLYGON_SMOOTH);  //����ο����

								  //DrawBall(10, ball.x, ball.z, 0.5, 0.5, 0.5);
	glColor3f(0, 0, 0);
	drawAxis();
	fountain->render();
	/*
	particlesystem->render
	*/

	//DrawBall(10,0,0,0,0,0);

	glPopMatrix();

	glutSwapBuffers();  //˫������ʹ�øú���������������������
	glFlush();
}

void timerFunc(int value) {    // ��ʱ�ص�����
	//ball.move(timeInterval);    // ÿ��һ��ʱ�䣬����ģ�͵�λ��
	/*
	particlesystem->update
	*/
	fountain->update((timeInterval*1.0) / 1000);
	glutPostRedisplay();  //��ǵ�ǰ������Ҫ���»���
	glutTimerFunc(timeInterval, timerFunc, 1);  //������ʱ���ص�����
}

void reshape(int w, int h) {    // ���ڴ�С�ı�Ļص�����
	GLfloat nRange = 200.0f;
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/*
	if (w <= h) {
	glOrtho(-nRange, nRange, -nRange*h / w, nRange*h / w, -nRange, nRange);
	}
	else {
	glOrtho(-nRange*w / h, nRange*w / h, -nRange, nRange, -nRange, nRange);
	}
	*/
	glOrtho(-5, 5, -1, 9, 0, 20);
	gluLookAt(1, -10, 1, 0, 0, 0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);  //�趨��ǰ����Ϊ�Ӿ�����
	glLoadIdentity();
}

void mouse(int button, int state, int x, int y) {    // ������ص�����
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		/*
		ball.vx *= 0.9;
		ball.vy *= 0.9;
		ball.vz *= 0.9;
		printf("mouse\n");
		*/
	}
}

void mouseMotion(int x, int y) {    // ����϶��ص�����
	/*/
	ball.vx *= 0.9;
	ball.vy *= 0.9;
	ball.vz *= 0.9;
	printf("mouseMotion\n");
	*/
}

void keyboard(unsigned char key, int x, int y) {    // ���̻ص�����
	switch (key) {
	case 27:    // ESC
		exit(0);
		break;
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);  //��GLUT���г�ʼ�������������е������в����������������κ�GLUT����ǰ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  //ָ��ʹ��RGBAģʽ����ɫ����ģʽ��ʹ�õ������˫���崰��
	glutInitWindowSize(windowWidth, windowHeight);  //ָ�����ڴ�С��������Ϊ��λ
	glutInitWindowPosition(100, 100);  //ָ���������Ͻǵ���Ļλ��
	glutCreateWindow("hw1");  //argv[0]��ʾ��ִ�г���·��;����һ��֧��OpenGL��Ⱦ�����Ĵ��ڣ�����һ��Ψһ�ı�ʶ����ʶ���ڣ��ڵ���glutMainLoop()����ǰ���ô��ڲ�����ʾ
	glutDisplayFunc(display);  //��ʾ�ص�������ÿ��GLUTȷ��һ�����ڵ�������Ҫ������ʾʱ��ͨ���ú���ע����Ǹ��ص������ͻᱻִ��
	glutReshapeFunc(reshape);  //�����ڴ�С�����ı�ʱ��ȡ�ж�
	glutMouseFunc(mouse);    // ����¼�
	glutMotionFunc(mouseMotion);    // ����϶��¼�
	glutKeyboardFunc(keyboard);  //����Ѽ�����һ�����İ�ť��һ�������������ͨ���������ú���
	glutTimerFunc(timeInterval, timerFunc, 1);  //���ö�ʱ���ص�����
	init();
	//loadModel();    // ��������objģ��
	glutMainLoop();  //�������������������Ѿ������Ĵ��ڽ�������ʱ��ʾ������Щ���ڵ���ȾҲ��ʼ��Ч���¼�����ѭ����ʼ��������ע�����ʾ�ص�����������
	return 0;
}
