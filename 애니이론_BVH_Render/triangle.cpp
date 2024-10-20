#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>


void display(void) //glutDisplayFunc�Լ��� ���� ȣ��� �ݹ��Լ�
{
    glClear(GL_COLOR_BUFFER_BIT); //ȭ�� Clear
    glBegin(GL_POLYGON); //�׷��� Object�� ����
    glVertex3f(-0.5, -0.5, -0.5);//ȭ�鿡 ���� ����
    glVertex3f(0.5, -0.5, 0.5);//ȭ�鿡 ���� ����
    glVertex3f(0.5, 0.5, 0.0);//ȭ�鿡 ���� ����
    glVertex3f(-0.5, 0.5, 0.0); //ȭ�鿡 ���� ����
    glEnd();//�׷��� Object ���� ������
    glFlush();//���ۿ� �ִ� ������ �׸����� ��
}
int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // GLUT ���̺귯�� �ʱ�ȭ
    glutCreateWindow("Test"); // ������ ����
    glutDisplayFunc(display); // ������ �ۼ��� display �ݹ��Լ��� �Ѱ���
    glutMainLoop();//���� ���� ����ڰ� �������Ḧ �Ҷ����� ��� ���ѷ��� ��������
    return 0; //���η����� ���� ������ ������� ����
}