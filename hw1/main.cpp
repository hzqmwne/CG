#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include "objLoader.h"
#include "waveIn.h"

float windowWidth = 900.0f, windowHeight = 400.0f;  //��Ļ��ȣ��߶�
const GLfloat Pi = 3.1415926f;  //����Բ����  
const int N = 50;
unsigned int timeInterval = 33/2;    // �����˶���ʱ����
float R = 20;    // С��뾶
float leftBound = 0 + R / 2;    // ��xOzƽ���˶����ĸ��߽�
float rightBound = windowWidth - R / 2;
float downBound = 0 + R / 2;
float upBound = windowHeight - R / 2;

int sgn(float f) {    // ��������1����������-1���㷵��0
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

class Model {    // ����ģ�͵���άλ�á��ٶȡ����ٶ�
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
	az = -9.8/1000/20;    // ģ����������ٶ�
}
void Model::move(unsigned int millis) {    // �ӵ�ǰ״̬��λ�á��ٶȡ����ٶȣ���ʼ�����㾭��millisʱ����µ�״̬
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
	if (x >= rightBound) {    // ������
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
	if (z <= downBound) {    // ÿ�δ��ף���Ҫ��ʧ��Լ20%������
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

float maxScaleRatio(float d, float r, float angle) {    // ���㲻�Ӵ�����ĵ�����������
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
	// ���Դ򿪺͹ر��豸ʱ�Ĵ���
	if (uMsg == WIM_DATA)
	{
		LPWAVEHDR pHdr = (LPWAVEHDR)dwParam1;

		MMRESULT mmres = waveInUnprepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		//�������� 
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
		//����׼������
		mmres = waveInPrepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		mmres = waveInAddBuffer(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
	}

	return TRUE;

}

void init(void) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	//gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);
	GLfloat light0_position[] = { 0.0f + 200, 0.0f - 200, 0.0f + 200, 1.0f };    // 0�Ź�Դ
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
	modelFile.open("model.obj", std::ios::in);    // Ѱ��Ĭ��·���µ�model.obj�ļ�������
	if (!modelFile) {
		return;
	}
	model = new Obj(modelFile);
	modelFile.close();
}

/*
void DrawBall(GLint R, GLfloat cx, GLfloat cy, GLfloat a, GLfloat b, GLfloat c) {
	
	glEnable(GL_BLEND);  //���û�Ϲ��ܣ���ͼ����Χ��ɫ���
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //�㿹���
	glEnable(GL_LINE_SMOOTH);  //�߿����
	glEnable(GL_POLYGON_SMOOTH);  //����ο����
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

void drawAxis() {    // ������������
	glBegin(GL_LINES);
	glVertex3f(-900, 0, 0);
	glVertex3f(900, 0, 00);
	glVertex3f(0, -400, 0);
	glVertex3f(0, 400, 0);
	glVertex3f(0, 0, -400);
	glVertex3f(0, 0, 400);
	glEnd();
}

void display(void) {    // ҳ�滺��ˢ��ʱ�Ļص�����
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // �����ɫ����Ȼ���

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

	glEnable(GL_BLEND);  //���û�Ϲ��ܣ���ͼ����Χ��ɫ���
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //�㿹���
	glEnable(GL_LINE_SMOOTH);  //�߿����
	glEnable(GL_POLYGON_SMOOTH);  //����ο����

	//DrawBall(10, ball.x, ball.z, 0.5, 0.5, 0.5);
	glColor3f(0, 0, 0);
	drawAxis();

	// ����ģ�͵�λ�ú͵������̵����α���
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
	if (model != NULL) {    // ����ɹ����ļ�������objģ�ͣ���ʹ����
		model->draw(40);
	}
	else {    // ���û�е���ɹ�ģ�ͣ�����ÿ⺯����һ������
		glutSolidSphere(20, 10, 10);
	}
	//DrawBall(10,0,0,0,0,0);

	glPopMatrix();

	glutSwapBuffers();  //˫������ʹ�øú���������������������
	glFlush();
}

void timerFunc(int value) {    // ��ʱ�ص�����
	ball.move(timeInterval);    // ÿ��һ��ʱ�䣬����ģ�͵�λ��

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
	glOrtho(0 - 100, 900, 0 - 100, 400, -400, 400);
	gluLookAt(0 + 100, -300, 0 + 100, 0 + 50, 0, 0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);  //�趨��ǰ����Ϊ�Ӿ�����
	glLoadIdentity();
}

void mouse(int button, int state, int x, int y) {    // ������ص�����
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		ball.vx *= 0.9;
		ball.vy *= 0.9;
		ball.vz *= 0.9;
		printf("mouse\n");
	}
}

void mouseMotion(int x, int y) {    // ����϶��ص�����
	ball.vx *= 0.9;
	ball.vy *= 0.9;
	ball.vz *= 0.9;
	printf("mouseMotion\n");
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
	loadModel();    // ��������objģ��
	glutMainLoop();  //�������������������Ѿ������Ĵ��ڽ�������ʱ��ʾ������Щ���ڵ���ȾҲ��ʼ��Ч���¼�����ѭ����ʼ��������ע�����ʾ�ص�����������
	return 0;
}
