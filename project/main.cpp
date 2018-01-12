// #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#include <GL/glut.h>
#include <GL/glaux.h>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include "particle.h"
#include "waveIn.h"

float windowWidth = 1000.0f, windowHeight = 600.0f;  //��Ļ��ȣ��߶�
const GLfloat Pi = 3.1415926f;  //����Բ����  
unsigned int timeInterval = 33;    // �����˶���ʱ���������룩

static int oldx;    // ����ϴε�����϶�������
static int oldy;

static int isPaused = 0;    // ��ͣ����

ParticleSystem *fountain = NULL;    // ����ϵͳ

WaveIn *waveIn = NULL;    // ��Ƶ�ɼ�

static float gluPerspectiveArgs[4] = { 75, 0, 0.1, 1000 };    // gluPerspective������Ĭ�ϲ���
static float gluLookAtArgs[9] = { 1, -9, 2, 0, 0, 0, 0, 0, 1 };    // gluLookAt������Ĭ�ϲ���

/* =============================================================== */

float f(float volume) {    // ���ӳ��ٶȺ�����֮���ӳ�亯��
	float x = (volume + 96.0) / 96.0;
	float y = 2.0/Pi * (atan(pow(x, 0.4)*2 - 1) + Pi/4.0);
	return y;
}

int pcm_db_count(const unsigned char* ptr, size_t size) {    // ������Ƶ�������ݼ�������������ֵ��Χ[-96,0]
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

static BOOL CALLBACK WaveInProc(HWAVEIN hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {    // ��Ƶ�ɼ��Ļص�����
	// ���Դ򿪺͹ر��豸ʱ�Ĵ���
	if (uMsg == WIM_DATA) {
		LPWAVEHDR pHdr = (LPWAVEHDR)dwParam1;

		MMRESULT mmres = waveInUnprepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		//�������� 
		int volume = pcm_db_count((unsigned char *)(pHdr->lpData), pHdr->dwBytesRecorded);
		//printf("%d\n", volume);
		fountain->elevation = Pi / 2.0 * f(volume);    // �޸����ӵĳ�ʼ����
		fountain->initialVelocity = 2.0 / Pi * 13 * fountain->elevation;    // �޸����ӵĳ��ٶȴ�С
		//����׼������
		mmres = waveInPrepareHeader(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
		mmres = waveInAddBuffer(waveIn->m_hWaveIn, pHdr, sizeof(WAVEHDR));
	}
	return TRUE;
}

/* ======================================================================= */

void perspectiveAndLookAtInit() {    // Ϊ�˷�������ӽǱ任�������˼򵥵ĺ�����װ
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(gluPerspectiveArgs[0], gluPerspectiveArgs[1], gluPerspectiveArgs[2], gluPerspectiveArgs[3]);
	gluLookAt(gluLookAtArgs[0], gluLookAtArgs[1], gluLookAtArgs[2],
		gluLookAtArgs[3], gluLookAtArgs[4], gluLookAtArgs[5],
		gluLookAtArgs[6], gluLookAtArgs[7], gluLookAtArgs[8]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLfloat light0_position[] = { -9, 0, 1, 1.0f };    // 0�Ź�Դ

void init(void) {    // ��ʼ��
	srand(time(0));
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	fountain = new ParticleSystem(10000, {0,0,0});
	fountain->initializeSystem();
	//GLfloat light0_position[] = { -9, 0, 1, 1.0f };    // 0�Ź�Դ
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

void skybox() {    // ������պ�
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
	float vt[4][2] = {    // ��������˳��
		{ 1,1 },
		{ 0,1 },
		{ 0,0 },
		{ 1,0 },
	};
	// up,down,left,right,front,back
	glColor3f(1, 1, 1);
	// ��������
	GLfloat ambient[] = { 0.2,0.2,0.2,1.0 };    // ���������
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	GLfloat diffuse[] = { 0.8,0.8,0.8,1.0 };    // �������
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	GLfloat specular[] = { 0.0,0.0,0.0,1.0 };   // ���淴���
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	GLfloat shininess = 0.2;    // ���淴��ϵ��
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	for (int i = 0; i < 6; ++i) {
		glBindTexture(GL_TEXTURE_2D, scene[i]);
		glBegin(GL_QUADS);
		for (int j = 0; j < 4; ++j) {
			glTexCoord2f(vt[j][0], vt[j][1]);  //����    
			glNormal3f(vn[f[i][j]].x, vn[f[i][j]].y, vn[f[i][j]].z);//������
			glVertex3f(v[f[i][j]].x * 2, v[f[i][j]].y * 2, v[f[i][j]].z * 2);    // ����
		}
		glEnd();
	}
}

void cube() {    // ��������
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
	float vt[4][2] = {    // ��������˳��
		{ 1,1 },
		{ 0,1 },
		{ 0,0 },
		{ 1,0 },
	};
	for (int i = 0; i < 6; ++i) {
		glBegin(GL_QUADS);
		for (int j = 0; j < 4; ++j) {
			glTexCoord2f(vt[j][0], vt[j][1]);  //����    
			glNormal3f(vn[f[i][j]].x, vn[f[i][j]].y, vn[f[i][j]].z);//������
			glVertex3f(v[f[i][j]].x * 2, v[f[i][j]].y * 2, v[f[i][j]].z * 2);    // ����
		}
		glEnd();
	}
}

void cylinder() {    // ��Բ����
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

void display(void) {    // ҳ�滺��ˢ��ʱ�Ļص�����
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // �����ɫ����Ȼ���

	glPushMatrix();

	glEnable(GL_BLEND);  //���û�Ϲ��ܣ���ͼ����Χ��ɫ���
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);  //�㿹���
	glEnable(GL_LINE_SMOOTH);  //�߿����
	glEnable(GL_POLYGON_SMOOTH);  //����ο����

	glColor3f(1, 1, 1);
	skybox();    // ������պ�

	glColor4f(0, 0, 0, 1);
	static GLuint marble = 0;
	if (marble == 0) {
		marble = LoadGLTexture("marble.bmp");    // ����ʯ����
	}
	// ����һ�Ű��������ĸ�Բ��
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
	
	fountain->render();    // ��������ϵͳ

	glPopMatrix();

	glutSwapBuffers();  //˫������ʹ�øú���������������������
	//glFlush();
}

void timerFunc(int value) {    // ��ʱ�ص�����
	if (!isPaused) {    // ��ͣ����
		fountain->update((timeInterval*1.0) / 1000);    // ��������ϵͳ��״̬
	}
	glutPostRedisplay();  //��ǵ�ǰ������Ҫ���»���
	glutTimerFunc(timeInterval, timerFunc, 1);  //������ʱ���ص�����
}

void reshape(int w, int h) {    // ���ڴ�С�ı�Ļص�����
	GLfloat nRange = 200.0f;
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	windowWidth = w;
	windowHeight = h;
	gluPerspectiveArgs[1] = w*1.0 / h;    // ͸��ͶӰ
	perspectiveAndLookAtInit();
}

void mouse(int button, int state, int x, int y) {    // ���ص�����
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		oldx = x;    // Ϊ�����mouseMotion����׼����ʼ����
		oldy = y;
	}
}

void mouseMotion(int x, int y) {    // ����϶��ص�����
	static float oldtheta = -Pi / 2.0;
	static float oldheight = 1.0;
	float newtheta = oldtheta;
	float newheight = oldheight;
	if (abs(x - oldx) < 100) {    // �������λ�ƣ���������
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
	oldtheta = newtheta;    // ����ȫ�ֱ���
	oldheight = newheight;
	oldx = x;
	oldy = y;
	perspectiveAndLookAtInit();    // ���¶���۲����
}

void keyboard(unsigned char key, int x, int y) {    // ���̻ص�����
	switch (key) {
	case 27: {    // ESC
		exit(0);
		break;
	}
	case 'p': {
		isPaused = !isPaused;
		break;
	}
	case 'z': {    // ��Դλ������
		light0_position[0] = max(-9, light0_position[0] - 1);
		glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
		break;
	}
	case 'x': {    // ��Դλ������
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
	glutInit(&argc, argv);  //��GLUT���г�ʼ�������������е������в����������������κ�GLUT����ǰ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);  //ָ��ʹ��RGBAģʽ����ɫ����ģʽ��ʹ�õ������˫���崰��
	glutInitWindowSize(windowWidth, windowHeight);  //ָ�����ڴ�С��������Ϊ��λ
	glutInitWindowPosition(100, 100);  //ָ���������Ͻǵ���Ļλ��
	glutCreateWindow("project");  //argv[0]��ʾ��ִ�г���·��;����һ��֧��OpenGL��Ⱦ�����Ĵ��ڣ�����һ��Ψһ�ı�ʶ����ʶ���ڣ��ڵ���glutMainLoop()����ǰ���ô��ڲ�����ʾ
	glutDisplayFunc(display);  //��ʾ�ص�������ÿ��GLUTȷ��һ�����ڵ�������Ҫ������ʾʱ��ͨ���ú���ע����Ǹ��ص������ͻᱻִ��
	glutReshapeFunc(reshape);  //�����ڴ�С�����ı�ʱ��ȡ�ж�
	glutMouseFunc(mouse);    // ����¼�
	glutMotionFunc(mouseMotion);    // ����϶��¼�
	glutKeyboardFunc(keyboard);  //����Ѽ�����һ�����İ�ť��һ�������������ͨ���������ú���
	glutTimerFunc(timeInterval, timerFunc, 1);  //���ö�ʱ���ص�����
	init();    // ��ʼ��
	glutMainLoop();  //�������������������Ѿ������Ĵ��ڽ�������ʱ��ʾ������Щ���ڵ���ȾҲ��ʼ��Ч���¼�����ѭ����ʼ��������ע�����ʾ�ص�����������
	return 0;
}
