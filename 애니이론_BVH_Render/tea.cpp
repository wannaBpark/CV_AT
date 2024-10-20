#include <GL/glew.h>
#include <gl/glut.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>            
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>   
#include <glm/gtx/norm.hpp>  
#include <iostream>

using glm::vec4;
using glm::mat4;
using glm::vec3;
using glm::cross;
using glm::normalize;

int type = 0;
GLuint p;

glm::mat4 LookAt(const vec4& eye, const vec4& at, const vec4& up)
{
    vec4 n = normalize(eye - at);
    vec4 u = vec4(normalize(cross(glm::vec3(up), glm::vec3(n))), 0.0);
    vec4 v = vec4(normalize(cross(glm::vec3(n), glm::vec3(u))), 0.0);
    vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
    mat4 c = mat4(u, v, n, t);
    return glm::translate(c, -glm::vec3(eye));
}

void renderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 viewMatrix = LookAt(vec4(1.0f, 1.0f, 1.0f, 1), vec4(0, 0, 0, 1), vec4(0, 1.0f, 0, 1));
    glUseProgram(0);
    GLint mvLocation = glGetUniformLocation(p, "Mv");

    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();

    glColor3f(1, 0, 0);

    switch (type) {
    case 0:
        glutWireTeapot(0.2);
        break;
    case 1:
        glutWireCube(0.5);
        break;
    case 2:
        glutWireSphere(0.5, 10, 10);
        break;
    case 3:
        glutWireCylinder(0.5, 0.5, 10, 10);
        break;
    default:
        break;
    }

    glUniformMatrix4fv(mvLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Window");

    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewStatus) << std::endl;
        return -1;
    }

    glutDisplayFunc(renderScene);
    glutMainLoop();

    return 0;
}
