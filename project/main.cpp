#include <GL/glut.h>
#include <GL/glaux.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include "particle.h"
#include "waveIn.h"

float windowWidth = 400.0f, windowHeight = 400.0f;  //屏幕宽度，高度
const GLfloat Pi = 3.1415926f;  //定义圆周率  
const int N = 50;
unsigned int timeInterval = 33;    // 计算运动的时间间隔

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
	// 忽略打开和关闭设备时的处理
	if (uMsg == WIM_DATA) {
		LPWAVEHDR pHdr = (LPWAVEHDR)dwParam1;

		MMRESULT mmres = waveInUnprepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		//处理数据 
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
		//重新准备数据
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

void drawAxis() {    // 画三个坐标轴
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

void display(void) {    // 页面缓冲刷新时的回调函数
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓存

	glPushMatrix();

	glEnable(GL_BLEND);  //启用混合功能，将图形周围颜色混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //点抗锯齿
	glEnable(GL_LINE_SMOOTH);  //线抗锯齿
	glEnable(GL_POLYGON_SMOOTH);  //多边形抗锯齿

								  //DrawBall(10, ball.x, ball.z, 0.5, 0.5, 0.5);
	glColor3f(0, 0, 0);
	drawAxis();
	fountain->render();
	/*
	particlesystem->render
	*/

	//DrawBall(10,0,0,0,0,0);

	glPopMatrix();

	glutSwapBuffers();  //双缓冲下使用该函数交换两个缓冲区内容
	glFlush();
}

void timerFunc(int value) {    // 定时回调函数
	//ball.move(timeInterval);    // 每隔一定时间，计算模型的位移
	/*
	particlesystem->update
	*/
	fountain->update((timeInterval*1.0) / 1000);
	glutPostRedisplay();  //标记当前窗口需要重新绘制
	glutTimerFunc(timeInterval, timerFunc, 1);  //创建定时器回调函数
}

void reshape(int w, int h) {    // 窗口大小改变的回调函数
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

	glMatrixMode(GL_MODELVIEW);  //设定当前矩阵为视景矩阵
	glLoadIdentity();
}

void mouse(int button, int state, int x, int y) {    // 鼠标点击回调函数
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		/*
		ball.vx *= 0.9;
		ball.vy *= 0.9;
		ball.vz *= 0.9;
		printf("mouse\n");
		*/
	}
}

void mouseMotion(int x, int y) {    // 鼠标拖动回调函数
	/*/
	ball.vx *= 0.9;
	ball.vy *= 0.9;
	ball.vz *= 0.9;
	printf("mouseMotion\n");
	*/
}

void keyboard(unsigned char key, int x, int y) {    // 键盘回调函数
	switch (key) {
	case 27:    // ESC
		exit(0);
		break;
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);  //对GLUT进行初始化，并处理所有的命令行参数；调用于其他任何GLUT函数前
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  //指定使用RGBA模式或颜色索引模式，使用单缓冲或双缓冲窗口
	glutInitWindowSize(windowWidth, windowHeight);  //指定窗口大小，以像素为单位
	glutInitWindowPosition(100, 100);  //指定窗口左上角的屏幕位置
	glutCreateWindow("hw1");  //argv[0]显示可执行程序路径;创建一个支持OpenGL渲染环境的窗口，返回一个唯一的标识符标识窗口；在调用glutMainLoop()函数前，该窗口并不显示
	glutDisplayFunc(display);  //显示回调函数，每当GLUT确定一个窗口的内容需要重新显示时，通过该函数注册的那个回调函数就会被执行
	glutReshapeFunc(reshape);  //当窗口大小发生改变时采取行动
	glutMouseFunc(mouse);    // 鼠标事件
	glutMotionFunc(mouseMotion);    // 鼠标拖动事件
	glutKeyboardFunc(keyboard);  //允许把键盘上一个键的按钮与一个函数相关联，通过按键调用函数
	glutTimerFunc(timeInterval, timerFunc, 1);  //设置定时器回调函数
	init();
	//loadModel();    // 尝试载入obj模型
	glutMainLoop();  //调用以启动程序，所以已经创建的窗口将会在这时显示，对这些窗口的渲染也开始生效。事件处理循环开始启动，已注册的显示回调函数被触发
	return 0;
}
