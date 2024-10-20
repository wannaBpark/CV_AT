#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>


void display(void) //glutDisplayFunc함수에 의해 호출될 콜백함수
{
    glClear(GL_COLOR_BUFFER_BIT); //화면 Clear
    glBegin(GL_POLYGON); //그래픽 Object를 선언
    glVertex3f(-0.5, -0.5, -0.5);//화면에 점을 찍음
    glVertex3f(0.5, -0.5, 0.5);//화면에 점을 찍음
    glVertex3f(0.5, 0.5, 0.0);//화면에 점을 찍음
    glVertex3f(-0.5, 0.5, 0.0); //화면에 점을 찍음
    glEnd();//그래픽 Object 내용 마무리
    glFlush();//버퍼에 있는 내용을 그리도록 함
}
int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // GLUT 라이브러리 초기화
    glutCreateWindow("Test"); // 윈도우 생성
    glutDisplayFunc(display); // 위에서 작성한 display 콜백함수를 넘겨줌
    glutMainLoop();//메인 루프 사용자가 게임종료를 할때까지 계속 무한루프 돌고있음
    return 0; //메인루프가 끝날 때까지 실행되지 않음
}