// #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#include <GL/glut.h>
#include <GL/glaux.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include "particle.h"
#include "waveIn.h"

float windowWidth = 1000.0f, windowHeight = 600.0f;  //屏幕宽度，高度
const GLfloat Pi = 3.1415926f;  //定义圆周率  
unsigned int timeInterval = 33;    // 计算运动的时间间隔（毫秒）

static int oldx;    // 鼠标上次点击或拖动的坐标
static int oldy;

static int isPaused = 0;    // 暂停控制

ParticleSystem *fountain = NULL;    // 粒子系统

WaveIn *waveIn = NULL;    // 音频采集

static float gluPerspectiveArgs[4] = { 75, 0, 0.1, 1000 };    // gluPerspective函数的默认参数
static float gluLookAtArgs[9] = { 1, -9, 2, 0, 0, 0, 0, 0, 1 };    // gluLookAt函数的默认参数

/* =============================================================== */

float f(float volume) {    // 粒子初速度和音量之间的映射函数
	float x = (volume + 96.0) / 96.0;
	float y = 2.0/Pi * (atan(pow(x, 0.4)*2 - 1) + Pi/4.0);
	return y;
}

int pcm_db_count(const unsigned char* ptr, size_t size) {    // 根据音频采样数据计算音量，返回值范围[-96,0]
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

static BOOL CALLBACK WaveInProc(HWAVEIN hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {    // 音频采集的回调函数
	// 忽略打开和关闭设备时的处理
	if (uMsg == WIM_DATA) {
		LPWAVEHDR pHdr = (LPWAVEHDR)dwParam1;

		MMRESULT mmres = waveInUnprepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		//处理数据 
		int volume = pcm_db_count((unsigned char *)(pHdr->lpData), pHdr->dwBytesRecorded);
		//printf("%d\n", volume);
		fountain->elevation = Pi / 2.0 * f(volume);    // 修改粒子的初始仰角
		fountain->initialVelocity = 2.0 / Pi * 13 * fountain->elevation;    // 修改粒子的初速度大小
		//重新准备数据
		mmres = waveInPrepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		mmres = waveInAddBuffer(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
	}
	return TRUE;
}

/* ======================================================================= */

void perspectiveAndLookAtInit() {    // 为了方便进行视角变换，进行了简单的函数封装
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(gluPerspectiveArgs[0], gluPerspectiveArgs[1], gluPerspectiveArgs[2], gluPerspectiveArgs[3]);
	gluLookAt(gluLookAtArgs[0], gluLookAtArgs[1], gluLookAtArgs[2],
		gluLookAtArgs[3], gluLookAtArgs[4], gluLookAtArgs[5],
		gluLookAtArgs[6], gluLookAtArgs[7], gluLookAtArgs[8]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLfloat light0_position[] = { -9, 0, 1, 1.0f };    // 0号光源

void init(void) {    // 初始化
	srand(time(0));
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	fountain = new ParticleSystem(10000, {0,0,0});
	fountain->initializeSystem();
	//GLfloat light0_position[] = { -9, 0, 1, 1.0f };    // 0号光源
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	GLfloat ambientLight[] = {1.0, 1.0, 1.0, 0.0};    //RGBA
	GLfloat specularLight[] = { 1.0, 1.0, 1.0, 0.0 };    //RGBA
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	waveIn = new WaveIn((DWORD_PTR)WaveInProc);
	waveIn->open();
	waveIn->start();
}

void skybox() {    // 绘制天空盒
	char *bmp[6] = {
		"skybox/up.bmp", 
		"skybox/dn.bmp" , 
		"skybox/lf.bmp" , 
		"skybox/rt.bmp" , 
		"skybox/ft.bmp" , 
		"skybox/bk.bmp" ,
	};
	static GLuint scene[6] = { 0, 0, 0, 0, 0, 0 };
	for (int i = 0; i < 6; ++i) {
		if (scene[i] == 0) {
			scene[i] = LoadGLTexture(bmp[i]);
		}
	}
	Vector3 v[8] = {
		{ 10, 10, 0 },    // 0
		{ -10, 10, 0 },    // 1
		{ -10, -10, 0 },    // 2
		{ 10, -10, 0 },    // 3
		{ 10, 10, 20 },    // 4
		{ -10, 10, 20 },    // 5
		{ -10, -10, 20 },    // 6
		{ 10, -10, 20 },    // 7
	};
	Vector3 vn[8] = {
		{ -1 / sqrt(3), -1 / sqrt(3), 1 / sqrt(3) },
		{ 1 / sqrt(3), -1 / sqrt(3), 1 / sqrt(3) },
		{ 1 / sqrt(3), 1 / sqrt(3), 1 / sqrt(3) },
		{ -1 / sqrt(3), 1 / sqrt(3), 1 / sqrt(3) },
		{ -1 / sqrt(3), -1 / sqrt(3), -1 / sqrt(3) },
		{ 1 / sqrt(3), -1 / sqrt(3), -1 / sqrt(3) },
		{ 1 / sqrt(3), 1 / sqrt(3), -1 / sqrt(3) },
		{ -1 / sqrt(3), 1 / sqrt(3), -1 / sqrt(3) },
	};
	int f[6][4] = {
		{ 7,6,5,4 },    // up
		{ 0,1,2,3 },    // down
		{ 5,6,2,1 },    // left
		{ 7,4,0,3 },    // right
		{ 6,7,3,2 },    // front
		{ 4,5,1,0 },    // back
	};
	float vt[4][2] = {    // 按照象限顺序
		{ 1,1 },
		{ 0,1 },
		{ 0,0 },
		{ 1,0 },
	};
	// up,down,left,right,front,back
	glColor3f(1, 1, 1);
	// 材质数据
	GLfloat ambient[] = { 0.2,0.2,0.2,1.0 };    // 环境反射光
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	GLfloat diffuse[] = { 0.8,0.8,0.8,1.0 };    // 漫反射光
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	GLfloat specular[] = { 0.0,0.0,0.0,1.0 };   // 镜面反射光
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	GLfloat shininess = 0.2;    // 镜面反射系数
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	for (int i = 0; i < 6; ++i) {
		glBindTexture(GL_TEXTURE_2D, scene[i]);
		glBegin(GL_QUADS);
		for (int j = 0; j < 4; ++j) {
			glTexCoord2f(vt[j][0], vt[j][1]);  //纹理    
			glNormal3f(vn[f[i][j]].x, vn[f[i][j]].y, vn[f[i][j]].z);//法向量
			glVertex3f(v[f[i][j]].x * 2, v[f[i][j]].y * 2, v[f[i][j]].z * 2);    // 顶点
		}
		glEnd();
	}
}

void cube() {    // 画正方体
	Vector3 v[8] = {
		{ 1, 1, -1 },    // 0
		{ -1, 1, -1 },    // 1
		{ -1, -1, -1 },    // 2
		{ 1, -1, -1 },    // 3
		{ 1, 1, 1 },    // 4
		{ -1, 1, 1 },    // 5
		{ -1, -1, 1 },    // 6
		{ 1, -1, 1 },    // 7
	};
	Vector3 vn[8] = {
		{ 1 / sqrt(3), 1 / sqrt(3), -1 / sqrt(3) },
		{ -1 / sqrt(3), 1 / sqrt(3), -1 / sqrt(3) },
		{ -1 / sqrt(3), -1 / sqrt(3), -1 / sqrt(3) },
		{ 1 / sqrt(3), -1 / sqrt(3), -1 / sqrt(3) },
		{ 1 / sqrt(3), 1 / sqrt(3), 1 / sqrt(3) },
		{ -1 / sqrt(3), 1 / sqrt(3), 1 / sqrt(3) },
		{ -1 / sqrt(3), -1 / sqrt(3), 1 / sqrt(3) },
		{ 1 / sqrt(3), -1 / sqrt(3), 1 / sqrt(3) },
	};
	int f[6][4] = {
		{ 7,6,5,4 },    // up
		{ 0,1,2,3 },    // down
		{ 5,6,2,1 },    // left
		{ 7,4,0,3 },    // right
		{ 6,7,3,2 },    // front
		{ 4,5,1,0 },    // back
	};
	float vt[4][2] = {    // 按照象限顺序
		{ 1,1 },
		{ 0,1 },
		{ 0,0 },
		{ 1,0 },
	};
	for (int i = 0; i < 6; ++i) {
		glBegin(GL_QUADS);
		for (int j = 0; j < 4; ++j) {
			glTexCoord2f(vt[j][0], vt[j][1]);  //纹理    
			glNormal3f(vn[f[i][j]].x, vn[f[i][j]].y, vn[f[i][j]].z);//法向量
			glVertex3f(v[f[i][j]].x * 2, v[f[i][j]].y * 2, v[f[i][j]].z * 2);    // 顶点
		}
		glEnd();
	}
}

void cylinder() {    // 画圆柱体
	int n = 40;
	for (int i = 0; i < n; ++i) {
		glBegin(GL_QUADS);
		glTexCoord2f(i*1.0/n, 1);
		glNormal3f(cos(i* 2 * Pi / n) / sqrt(2), sin(i* 2 * Pi / n) / sqrt(2), 1 / sqrt(2));
		glVertex3f(cos(i* 2 * Pi / n), sin(i* 2 * Pi / n), 1);
		glTexCoord2f((i+1)*1.0 / n, 1);
		glNormal3f(cos((i + 1)* 2 * Pi / n) / sqrt(2), sin((i + 1)* 2 * Pi / n) / sqrt(2), 1 / sqrt(2));
		glVertex3f(cos((i + 1)* 2 * Pi / n), sin((i + 1)* 2 * Pi / n), 1);
		glTexCoord2f((i + 1)*1.0 / n, 0);
		glNormal3f(cos((i + 1)* 2 * Pi / n) / sqrt(2), sin((i + 1)* 2 * Pi / n) / sqrt(2), -1 / sqrt(2));
		glVertex3f(cos((i + 1)* 2 * Pi / n), sin((i + 1)* 2 * Pi / n), -1);
		glTexCoord2f(i*1.0 / n, 0);
		glNormal3f(cos(i* 2 * Pi / n) / sqrt(2), sin(i* 2 * Pi / n) / sqrt(2), -1 / sqrt(2));
		glVertex3f(cos(i* 2 * Pi / n), sin(i* 2 * Pi / n), -1);
		glEnd();
		glBegin(GL_TRIANGLES);
		glTexCoord2f(0.5,0.5);
		glNormal3f(0,0,1);
		glVertex3f(0,0,1);
		glTexCoord2f(0.5+0.5*cos(i*2*Pi/n),0.5+0.5*sin(i*2*Pi/n));
		glNormal3f(cos(i * 2 * Pi / n) / sqrt(2), sin(i * 2 * Pi / n) / sqrt(2), 1 / sqrt(2));
		glVertex3f(cos(i* 2 * Pi) / n, sin(i* 2 * Pi) / n, 1);
		glTexCoord2f(0.5 + 0.5*cos((i+1) * 2 * Pi / n), 0.5 + 0.5*sin((i+1) * 2 * Pi / n));
		glNormal3f(cos((i+1) * 2 * Pi / n) / sqrt(2), sin((i+1) * 2 * Pi / n) / sqrt(2), 1 / sqrt(2));
		glVertex3f(cos((i + 1)* 2 * Pi / n), sin((i + 1)* 2 * Pi / n), 1);
		glTexCoord2f(0.5,0.5);
		glNormal3f(0,0,-1);
		glVertex3f(0,0,-1);
		glTexCoord2f(0.5 - 0.5*cos(i * 2 * Pi / n), 0.5 + 0.5*sin(i * 2 * Pi / n));
		glNormal3f(cos(i * 2 * Pi / n) / sqrt(2), sin(i * 2 * Pi / n) / sqrt(2), -1 / sqrt(2));
		glVertex3f(cos(i * 2 * Pi) / n, sin(i * 2 * Pi) / n, -1);
		glTexCoord2f(0.5 - 0.5*cos((i + 1) * 2 * Pi / n), 0.5 + 0.5*sin((i + 1) * 2 * Pi / n));
		glNormal3f(cos((i + 1) * 2 * Pi / n) / sqrt(2), sin((i + 1) * 2 * Pi / n) / sqrt(2), -1 / sqrt(2));
		glVertex3f(cos((i + 1) * 2 * Pi / n), sin((i + 1) * 2 * Pi / n), -1);
		glEnd();
	}
}

void display(void) {    // 页面缓冲刷新时的回调函数
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓存

	glPushMatrix();

	glEnable(GL_BLEND);  //启用混合功能，将图形周围颜色混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //点抗锯齿
	glEnable(GL_LINE_SMOOTH);  //线抗锯齿
	glEnable(GL_POLYGON_SMOOTH);  //多边形抗锯齿

	glColor3f(1, 1, 1);
	skybox();    // 绘制天空盒

	glColor4f(0, 0, 0, 1);
	static GLuint marble = 0;
	if (marble == 0) {
		marble = LoadGLTexture("marble.bmp");    // 大理石纹理
	}
	// 绘制一张八仙桌和四个圆凳
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, marble);
	glPushMatrix();
	glTranslatef(10, 0, 1);
	glScalef(1, 1, 0.1);
	cube();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(10, 0, 0);
	glScalef(0.7, 0.7, 1);
	cylinder();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(10-3, 0, 0);
	glScalef(0.3, 0.3, 0.6);
	cylinder();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(10+3, 0, 0);
	glScalef(0.3, 0.3, 0.6);
	cylinder();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(10, 3, 0);
	glScalef(0.3, 0.3, 0.6);
	cylinder();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(10, -3, 0);
	glScalef(0.3, 0.3, 0.6);
	cylinder();
	glPopMatrix();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	
	fountain->render();    // 绘制粒子系统

	glPopMatrix();

	glutSwapBuffers();  //双缓冲下使用该函数交换两个缓冲区内容
	//glFlush();
}

void timerFunc(int value) {    // 定时回调函数
	if (!isPaused) {    // 暂停控制
		fountain->update((timeInterval*1.0) / 1000);    // 更新粒子系统的状态
	}
	glutPostRedisplay();  //标记当前窗口需要重新绘制
	glutTimerFunc(timeInterval, timerFunc, 1);  //创建定时器回调函数
}

void reshape(int w, int h) {    // 窗口大小改变的回调函数
	GLfloat nRange = 200.0f;
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	windowWidth = w;
	windowHeight = h;
	gluPerspectiveArgs[1] = w*1.0 / h;    // 透视投影
	perspectiveAndLookAtInit();
}

void mouse(int button, int state, int x, int y) {    // 鼠标回调函数
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		oldx = x;    // 为后面的mouseMotion函数准备初始数据
		oldy = y;
	}
}

void mouseMotion(int x, int y) {    // 鼠标拖动回调函数
	static float oldtheta = -Pi / 2.0;
	static float oldheight = 1.0;
	float newtheta = oldtheta;
	float newheight = oldheight;
	if (abs(x - oldx) < 100) {    // 计算鼠标位移，调整数据
		newtheta = oldtheta + 2.0*Pi*(oldx - x) / windowWidth;
	}
	if (abs(y - oldy) < 100) {
		newheight += 20 * (y - oldy) / windowHeight * 2;
		newheight = max(newheight, 0.5 * 0);
		newheight = min(newheight, 20);
	}
	gluLookAtArgs[0] = 9.9 * cos(newtheta);
	gluLookAtArgs[1] = 9.9 * sin(newtheta);
	gluLookAtArgs[5] = newheight;
	oldtheta = newtheta;    // 更新全局变量
	oldheight = newheight;
	oldx = x;
	oldy = y;
	perspectiveAndLookAtInit();    // 重新定义观察矩阵
}

void keyboard(unsigned char key, int x, int y) {    // 键盘回调函数
	switch (key) {
	case 27: {    // ESC
		exit(0);
		break;
	}
	case 'p': {
		isPaused = !isPaused;
		break;
	}
	case 'z': {    // 光源位置左移
		light0_position[0] = max(-9, light0_position[0] - 1);
		glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
		break;
	}
	case 'x': {    // 光源位置右移
		light0_position[0] = min(9, light0_position[0] + 1);
		glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
		break;
	}
	case '1': {
		fountain->modelSelect = 1;
		break;
	}
	case '2': {
		fountain->modelSelect = 2;
		break;
	}
	case '3': {
		fountain->modelSelect = 3;
		break;
	}
	case '=': case '+': {
		int num = fountain->waterFlowCount;
		num = min(num + 1, 64);
		fountain->waterFlowCount = num;
		break;
	}
	case '-': {
		int num = fountain->waterFlowCount;
		num = max(num - 1, 1);
		fountain->waterFlowCount = num;
		break;
	}
	case 'k': {
		//printf("Up\n");
		float eyez = gluLookAtArgs[2];
		eyez = min(eyez + 1, 20);
		gluLookAtArgs[2] = eyez;
		perspectiveAndLookAtInit();
		break;
	}
	case 'j': {
		//printf("Down\n");
		float eyez = gluLookAtArgs[2];
		eyez = max(eyez - 1, 1);
		gluLookAtArgs[2] = eyez;
		perspectiveAndLookAtInit();
		break;
	}
	case 'h': {
		//printf("Left\n");
		float foxy = gluPerspectiveArgs[0];
		foxy = max(foxy - 2.5, 25);
		gluPerspectiveArgs[0] = foxy;
		perspectiveAndLookAtInit();
		break;
	}
	case 'l': {
		//printf("Right\n");
		float foxy = gluPerspectiveArgs[0];
		foxy = min(foxy + 2.5, 75);
		gluPerspectiveArgs[0] = foxy;
		perspectiveAndLookAtInit();
		break;
	}
	case 'r': {    // red light
		GLfloat specularLight[] = { 1.0, 0.0, 0.0, 0.0 };
		glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, specularLight);
		break;
	}
	case 'g': {    // green light
		GLfloat specularLight[] = { 0.0, 1.0, 0.0, 0.0 };
		glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, specularLight);
		break;
	}
	case 'b': {    // blue light
		GLfloat specularLight[] = { 0.0, 0.0, 1.0, 0.0 };
		glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, specularLight);
		break;
	}
	case 'w': {    // white light
		GLfloat specularLight[] = { 1.0, 1.0, 1.0, 0.0 };
		glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, specularLight);
		break;
	}
	case 'd': {    // dark light
		GLfloat specularLight[] = { 0.0, 0.0, 0.0, 0.0 };
		glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, specularLight);
		break;
	}
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);  //对GLUT进行初始化，并处理所有的命令行参数；调用于其他任何GLUT函数前
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);  //指定使用RGBA模式或颜色索引模式，使用单缓冲或双缓冲窗口
	glutInitWindowSize(windowWidth, windowHeight);  //指定窗口大小，以像素为单位
	glutInitWindowPosition(100, 100);  //指定窗口左上角的屏幕位置
	glutCreateWindow("project");  //argv[0]显示可执行程序路径;创建一个支持OpenGL渲染环境的窗口，返回一个唯一的标识符标识窗口；在调用glutMainLoop()函数前，该窗口并不显示
	glutDisplayFunc(display);  //显示回调函数，每当GLUT确定一个窗口的内容需要重新显示时，通过该函数注册的那个回调函数就会被执行
	glutReshapeFunc(reshape);  //当窗口大小发生改变时采取行动
	glutMouseFunc(mouse);    // 鼠标事件
	glutMotionFunc(mouseMotion);    // 鼠标拖动事件
	glutKeyboardFunc(keyboard);  //允许把键盘上一个键的按钮与一个函数相关联，通过按键调用函数
	glutTimerFunc(timeInterval, timerFunc, 1);  //设置定时器回调函数
	init();    // 初始化
	glutMainLoop();  //调用以启动程序，所以已经创建的窗口将会在这时显示，对这些窗口的渲染也开始生效。事件处理循环开始启动，已注册的显示回调函数被触发
	return 0;
}
