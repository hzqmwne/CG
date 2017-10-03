#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>

float windowWidth = 900.0f, windowHeight = 400.0f;  //屏幕宽度，高度
const GLfloat Pi = 3.1415926f;  //定义圆周率  
const int N = 50;
unsigned int timeInterval = 33/2;

class Model {
public:
	GLfloat x;
	GLfloat y;
	GLfloat vx;
	GLfloat vy;
	GLfloat ax;
	GLfloat ay;
	Model();
	void move(unsigned int);
};
Model::Model() {
	x = 0;
	y = 300;
	vx = 1.0/5;
	vy = 0;
	ax = 0;
	ay = -9.8/1000/20;
}
void Model::move(unsigned int millis) {
	printf("%f %f %f %f %f %f %u\n", x, y, vx, vy, ax, ay, millis);
	float oldx = x;
	float oldy = y;
	float oldvx = vx;
	float oldvy = vy;
	vx += ax*millis;
	vy += ay*millis;
	x += 0.5*(oldvx + vx)*millis;
	y += 0.5*(oldvy + vy)*millis;
	if (x >= windowWidth) {
		x = 2 * windowWidth - oldx;
		vx = -oldvx;
	}
	if (x <= 0) {
		x = oldx;
		vx = -oldvx;
	}
	if (y >= windowHeight) {
		y = 2 * windowHeight - oldy;
		vy = -oldvy;
	}
	if (y <= 0) {
		y = oldy;
		vy = -oldvy;
	}
}

Model ball = Model();

void init(void) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);
}

void DrawBall(GLint R, GLfloat cx, GLfloat cy, GLfloat a, GLfloat b, GLfloat c) {
	glEnable(GL_BLEND);  //启用混合功能，将图形周围颜色混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //点抗锯齿
	glEnable(GL_LINE_SMOOTH);  //线抗锯齿
	glEnable(GL_POLYGON_SMOOTH);  //多边形抗锯齿
	glColor3f(a, b, c);
	glBegin(GL_POLYGON);
	for (int i = 0; i < N; i++)
	{
		int xi = cx + R*cos(2 * Pi*i / N);
		int yi = cy + R*sin(2 * Pi*i / N);
		glVertex2f(xi, yi);
	}
	glEnd();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen And Depth Buffer

	DrawBall(10, ball.x, ball.y, 0, 0, 0);

	glutSwapBuffers();  //双缓冲下使用该函数交换两个缓冲区内容
	glFlush();
}

void timerFunc(int value) {
	ball.move(timeInterval);

	glutPostRedisplay();  //标记当前窗口需要重新绘制
	glutTimerFunc(timeInterval, timerFunc, 1);  //创建定时器回调函数
}

void reshape(int w, int h)
{
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)  //构建三维坐标
		glOrtho(0.0f, windowWidth, 0.0f, windowHeight * h / w, 1.0f, -1.0f);
	else
		glOrtho(0.0f, windowWidth * w / h, 0.0f, windowHeight, 1.0f, -1.0f);

	glMatrixMode(GL_MODELVIEW);  //设定当前矩阵为视景矩阵
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);  //对GLUT进行初始化，并处理所有的命令行参数；调用于其他任何GLUT函数前
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  //指定使用RGBA模式或颜色索引模式，使用单缓冲或双缓冲窗口
	glutInitWindowSize(windowWidth, windowHeight);  //指定窗口大小，以像素为单位
	glutInitWindowPosition(100, 100);  //指定窗口左上角的屏幕位置
	glutCreateWindow("Whatever");  //argv[0]显示可执行程序路径;创建一个支持OpenGL渲染环境的窗口，返回一个唯一的标识符标识窗口；在调用glutMainLoop()函数前，该窗口并不显示
	glutDisplayFunc(display);  //显示回调函数，每当GLUT确定一个窗口的内容需要重新显示时，通过该函数注册的那个回调函数就会被执行
	//glutReshapeFunc(reshape);  //当窗口大小发生改变时采取行动
	glutKeyboardFunc(keyboard);  //允许把键盘上一个键的按钮与一个函数相关联，通过按键调用函数
	glutTimerFunc(timeInterval, timerFunc, 1);  //设置定时器回调函数
	init();
	glutMainLoop();  //调用以启动程序，所以已经创建的窗口将会在这时显示，对这些窗口的渲染也开始生效。事件处理循环开始启动，已注册的显示回调函数被触发
	return 0;
}
