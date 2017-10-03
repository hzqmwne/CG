#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>

float windowWidth = 900.0f, windowHeight = 400.0f;  //��Ļ��ȣ��߶�
const GLfloat Pi = 3.1415926f;  //����Բ����  
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
	glEnable(GL_BLEND);  //���û�Ϲ��ܣ���ͼ����Χ��ɫ���
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //�㿹���
	glEnable(GL_LINE_SMOOTH);  //�߿����
	glEnable(GL_POLYGON_SMOOTH);  //����ο����
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

	glutSwapBuffers();  //˫������ʹ�øú���������������������
	glFlush();
}

void timerFunc(int value) {
	ball.move(timeInterval);

	glutPostRedisplay();  //��ǵ�ǰ������Ҫ���»���
	glutTimerFunc(timeInterval, timerFunc, 1);  //������ʱ���ص�����
}

void reshape(int w, int h)
{
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)  //������ά����
		glOrtho(0.0f, windowWidth, 0.0f, windowHeight * h / w, 1.0f, -1.0f);
	else
		glOrtho(0.0f, windowWidth * w / h, 0.0f, windowHeight, 1.0f, -1.0f);

	glMatrixMode(GL_MODELVIEW);  //�趨��ǰ����Ϊ�Ӿ�����
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
	glutInit(&argc, argv);  //��GLUT���г�ʼ�������������е������в����������������κ�GLUT����ǰ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  //ָ��ʹ��RGBAģʽ����ɫ����ģʽ��ʹ�õ������˫���崰��
	glutInitWindowSize(windowWidth, windowHeight);  //ָ�����ڴ�С��������Ϊ��λ
	glutInitWindowPosition(100, 100);  //ָ���������Ͻǵ���Ļλ��
	glutCreateWindow("Whatever");  //argv[0]��ʾ��ִ�г���·��;����һ��֧��OpenGL��Ⱦ�����Ĵ��ڣ�����һ��Ψһ�ı�ʶ����ʶ���ڣ��ڵ���glutMainLoop()����ǰ���ô��ڲ�����ʾ
	glutDisplayFunc(display);  //��ʾ�ص�������ÿ��GLUTȷ��һ�����ڵ�������Ҫ������ʾʱ��ͨ���ú���ע����Ǹ��ص������ͻᱻִ��
	//glutReshapeFunc(reshape);  //�����ڴ�С�����ı�ʱ��ȡ�ж�
	glutKeyboardFunc(keyboard);  //����Ѽ�����һ�����İ�ť��һ�������������ͨ���������ú���
	glutTimerFunc(timeInterval, timerFunc, 1);  //���ö�ʱ���ص�����
	init();
	glutMainLoop();  //�������������������Ѿ������Ĵ��ڽ�������ʱ��ʾ������Щ���ڵ���ȾҲ��ʼ��Ч���¼�����ѭ����ʼ��������ע�����ʾ�ص�����������
	return 0;
}
