#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include "objLoader.h"
#include "waveIn.h"

float windowWidth = 900.0f, windowHeight = 400.0f;  //屏幕宽度，高度
const GLfloat Pi = 3.1415926f;  //定义圆周率  
const int N = 50;
unsigned int timeInterval = 33/2;    // 计算运动的时间间隔
float R = 20;    // 小球半径
float leftBound = 0 + R / 2;    // 在xOz平面运动的四个边界
float rightBound = windowWidth - R / 2;
float downBound = 0 + R / 2;
float upBound = windowHeight - R / 2;

int sgn(float f) {    // 正数返回1，负数返回-1，零返回0
	return f > 0 ? 1 : (f < 0 ? -1 : 0);
}

/*
float min(float a, float b) {
	return a < b ? a : b;
}

float max(float a, float b) {
	return a > b ? a : b;
}
*/

class Model {    // 保存模型的三维位置、速度、加速度
public:
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat vx;
	GLfloat vy;
	GLfloat vz;
	GLfloat ax;
	GLfloat ay;
	GLfloat az;
	Model();
	void move(unsigned int);
	GLfloat v();
};
Model::Model() {
	x = leftBound;
	y = 0;
	z = 300 - 8;
	vx = 1.0/5;
	vy = 0;
	vz = 0;
	ax = 0;
	ay = 0;
	az = -9.8/1000/20;    // 模拟的重力加速度
}
void Model::move(unsigned int millis) {    // 从当前状态（位置、速度、加速度）开始，计算经过millis时间后新的状态
	//printf("%f %f %f %f %f %f %u\n", x, z, vx, vz, ax, az, millis);
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
GLfloat Model::v() {
	return sqrt(vx*vx + vy*vy + vz*vz);
}

Model ball = Model();
Obj *model = NULL;

WaveIn *waveIn = NULL;

float maxScaleRatio(float d, float r, float angle) {    // 计算不接触地面的的最大拉伸比例
	float arcAngle = (Pi / 180) * angle;
	if (sin(arcAngle) == 0) {
		return d / r;
	}
	float sinA2 = r / (d / sin(arcAngle));
	if (sinA2 >= 1) {
		return -1;
	}
	float tanA2 = sinA2 / sqrt(1 - sinA2*sinA2);
	float tanA1 = tan(arcAngle);
	return fabs(tanA1 / tanA2);
}

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

static BOOL CALLBACK WaveInProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
	// 忽略打开和关闭设备时的处理
	if (uMsg == WIM_DATA)
	{
		LPWAVEHDR pHdr = (LPWAVEHDR)dwParam1;

		MMRESULT mmres = waveInUnprepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		//处理数据 
		int volume = pcm_db_count((unsigned char *)(pHdr->lpData), pHdr->dwBytesRecorded);
		if (volume > -40) { 
			ball.vx += (sgn(ball.vx) == 0 ? 1 : sgn(ball.vx))*0.01;
			ball.vz += (sgn(ball.vz) == 0 ? 1 : sgn(ball.vz))*0.03;
			printf("volume %d\n", volume); 
		}
		/*
		if (NULL != m_pWavFile)
		{
			wav_write_data(m_pWavFile, (unsigned char *)(pHdr->lpData), pHdr->dwBytesRecorded);
		}
		*/
		//重新准备数据
		mmres = waveInPrepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		mmres = waveInAddBuffer(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
	}

	return TRUE;

}

void init(void) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	//gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);
	GLfloat light0_position[] = { 0.0f + 200, 0.0f - 200, 0.0f + 200, 1.0f };    // 0号光源
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	waveIn = new WaveIn((DWORD_PTR)WaveInProc);
	waveIn->open();
	waveIn->start();
}

void loadModel() {
	std::fstream modelFile;
	modelFile.open("model.obj", std::ios::in);    // 寻找默认路径下的model.obj文件并加载
	if (!modelFile) {
		return;
	}
	model = new Obj(modelFile);
	modelFile.close();
}

/*
void DrawBall(GLint R, GLfloat cx, GLfloat cy, GLfloat a, GLfloat b, GLfloat c) {
	
	glEnable(GL_BLEND);  //启用混合功能，将图形周围颜色混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //点抗锯齿
	glEnable(GL_LINE_SMOOTH);  //线抗锯齿
	glEnable(GL_POLYGON_SMOOTH);  //多边形抗锯齿
	glColor3f(a, b, c);

	glBegin(GL_POLYGON);
	//glBegin(GL_POINTS);
	for (int i = 0; i < N; i++)
	{
		int xi = cx + R*cos(2 * Pi*i / N);
		int zi = cy + R*sin(2 * Pi*i / N);
		glVertex3f(xi, 0, zi);
	}
	glEnd();
}
*/

void drawAxis() {    // 画三个坐标轴
	glBegin(GL_LINES);
	glVertex3f(-900, 0, 0);
	glVertex3f(900, 0, 00);
	glVertex3f(0, -400, 0);
	glVertex3f(0, 400, 0);
	glVertex3f(0, 0, -400);
	glVertex3f(0, 0, 400);
	glEnd();
}

void display(void) {    // 页面缓冲刷新时的回调函数
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓存

	float acosValue;
	if (ball.v() == 0) {
		acosValue = 0;
	}
	else {
		acosValue = (180 / Pi)*acos(ball.vx / sqrt(ball.vx*ball.vx + ball.vz*ball.vz));
	}
	float angle = sgn(ball.vz)*acosValue;
	float v = sqrt(ball.vx*ball.vx + ball.vy*ball.vy + ball.vz*ball.vz);
	float maxVerticalScaleRatio = maxScaleRatio((float)ball.z, R, fabs(acosValue - 90));

	glPushMatrix();

	glEnable(GL_BLEND);  //启用混合功能，将图形周围颜色混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //点抗锯齿
	glEnable(GL_LINE_SMOOTH);  //线抗锯齿
	glEnable(GL_POLYGON_SMOOTH);  //多边形抗锯齿

	//DrawBall(10, ball.x, ball.z, 0.5, 0.5, 0.5);
	glColor3f(0, 0, 0);
	drawAxis();

	// 处理模型的位置和弹跳过程的塑形变形
	glTranslatef(ball.x, ball.y, ball.z);
	if (maxVerticalScaleRatio < 1) {
		glRotatef(0, 0, -1, 0);
		glScalef(1, 1, ball.z / R);
	}
	else {
		glRotatef(angle, 0, -1, 0);
		glScalef(min(2, min(maxVerticalScaleRatio, v / 0.8 + 1)), 1, 1);
		glRotatef(-angle, 0, -1, 0);
	}

	glColor3f(0.5, 0.5, 0.5);
	if (model != NULL) {    // 如果成功从文件导入了obj模型，则使用它
		model->draw(40);
	}
	else {    // 如果没有导入成功模型，则调用库函数画一个球面
		glutSolidSphere(20, 10, 10);
	}
	//DrawBall(10,0,0,0,0,0);

	glPopMatrix();

	glutSwapBuffers();  //双缓冲下使用该函数交换两个缓冲区内容
	glFlush();
}

void timerFunc(int value) {    // 定时回调函数
	ball.move(timeInterval);    // 每隔一定时间，计算模型的位移

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
	glOrtho(0 - 100, 900, 0 - 100, 400, -400, 400);
	gluLookAt(0 + 100, -300, 0 + 100, 0 + 50, 0, 0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);  //设定当前矩阵为视景矩阵
	glLoadIdentity();
}

void mouse(int button, int state, int x, int y) {    // 鼠标点击回调函数
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		ball.vx *= 0.9;
		ball.vy *= 0.9;
		ball.vz *= 0.9;
		printf("mouse\n");
	}
}

void mouseMotion(int x, int y) {    // 鼠标拖动回调函数
	ball.vx *= 0.9;
	ball.vy *= 0.9;
	ball.vz *= 0.9;
	printf("mouseMotion\n");
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
	loadModel();    // 尝试载入obj模型
	glutMainLoop();  //调用以启动程序，所以已经创建的窗口将会在这时显示，对这些窗口的渲染也开始生效。事件处理循环开始启动，已注册的显示回调函数被触发
	return 0;
}
