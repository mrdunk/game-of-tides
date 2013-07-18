#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void display(void)
{
    cout<<"display\n";
    /* clear all pixels */
    glClear (GL_COLOR_BUFFER_BIT);

    /* draw white polygon (rectangle) with corners at
     * (0.25, 0.25, 0.0) and (0.75, 0.75, 0.0)
     */
    glColor3f (1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f (0.1, 0.1, 0.2);
    glVertex3f (0.75, 0.1, 0.2);
    glVertex3f (0.75, 0.75, 0.2);
    glVertex3f (0.1, 0.75, 0.2);
    glEnd();

    glColor3f (1.0, 0.5, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f (0.05, 0.05, 0.5);
    glVertex3f (0.65, 0.05, 0.5);
    glVertex3f (0.65, 0.65, 0.5);
    glVertex3f (0.05, 0.65, 0.5);
    glEnd();

    glColor3f (1.0, 1.0, 0.5);
    glBegin(GL_POLYGON);
    glVertex3f (0.2, 0.2, 0.5);
    glVertex3f (0.2, 0.85, 0.5);
    glVertex3f (0.85, 0.85, 0.5);
    glVertex3f (0.85, 0.2, 0.5);
    glEnd();

    /* don't wait!
     * start processing buffered OpenGL routines
     */
    //glFlush ();
    glutSwapBuffers();
}

void myGlDebug(){
    int i;
    int NumberOfExtensions;
    int OpenGLVersion[2];

    cout<<"OpenGL version = "<<glGetString(GL_VERSION)<<endl;

    //This is the new way for getting the GL version.
    //It returns integers. Much better than the old glGetString(GL_VERSION).
    glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
    cout<<"OpenGL major version = "<<OpenGLVersion[0]<<endl;
    cout<<"OpenGL minor version = "<<OpenGLVersion[1]<<endl<<endl;

    //The old method to get the extension list is obsolete.
    //You must use glGetIntegerv and glGetStringi
    glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
    //We don't need any extensions. Useless code.
    for(i=0; i<NumberOfExtensions; i++)
    {
        const GLubyte *ccc=glGetStringi(GL_EXTENSIONS, i);
    }

}

void init (void)
{
    /* select clearing (background) color */
    glClearColor (0.0, 0.0, 0.0, 0.0);

    /* initialize viewing values */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    /* only draw front faces. */
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glScalef(0.2, 0.2, 0.2);
}

void resize (int win_width, int win_height)
{
    //win_width = glutGet(GLUT_WINDOW_WIDTH);
    //win_height = glutGet(GLUT_WINDOW_HEIGHT);
    cout<<"win_width:  "<<win_width<<"\n";
    cout<<"win_height: "<<win_height<<"\n";
}

void click(int button, int state, int x, int y){
    /* http://pages.cpsc.ucalgary.ca/~tfalders/CPSC453/GLUT.html */
    cout<<"button:  "<<button<<"\tstate: "<<state<<"\tx: "<<x<<"  y: "<<y<<"\n";
    //glutPostRedisplay();
}

void timer(int value){
    cout<<"timer "<<value<<"\n";
    glutTimerFunc(5000, timer, 1);
}

/*
 * Declare initial window size, position, and display mode
 * (single buffer and RGBA). Open window with "hello"
 * in its title bar. Call initialization routines.
 * Register callback function to display graphics.
 * Enter main loop and process events.
 */
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
//    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
//    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);


    glutInitWindowSize (250, 250);
    //glutInitWindowPosition (100, 100);
    glutCreateWindow ("hello");
    GLenum err=glewInit();
    if(err!=GLEW_OK)
    {
        //Problem: glewInit failed, something is seriously wrong.
        cout<<"glewInit failed, aborting."<<endl;
        exit(1);
    }
    init ();


    myGlDebug();

    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutMouseFunc(click);
    glutTimerFunc(1000, timer, 1);
    glutMainLoop();

    return 0; /* ISO C requires main to return int. */
}


