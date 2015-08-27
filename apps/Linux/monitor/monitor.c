#include "monitor.h"
#include "device.h"
#include <stdlib.h>
#include <unistd.h>

/*  Poor man's approximation of PI */
#define PI 3.1415926535898
/*  Macro for sin & cos in degrees */
#define Cos(th) cos(PI/180*(th))
#define Sin(th) sin(PI/180*(th))

/*  Globals */
double dim=3.0; /* dimension of orthogonal box */
char *windowName = "Gesture Monitor";
int windowWidth=810;
int windowHeight=810;

/*  Various global state */
int toggleAxes = 1;   /* toggle axes on and off */
int toggleValues = 1; /* toggle values on and off */
int toggleMode = 0; /* projection mode */
int th = -45;   /* azimuth of view angle */
int ph = 30;   /* elevation of view angle */
int fov = 55; /* field of view for perspective */
int asp = 1;  /* aspect ratio */

/*  Cube vertices */
GLfloat vertA[3] = { 0.5, 0.5, 0.5};
GLfloat vertB[3] = {-0.5, 0.5, 0.5};
GLfloat vertC[3] = {-0.5,-0.5, 0.5};
GLfloat vertD[3] = { 0.5,-0.5, 0.5};
GLfloat vertE[3] = { 0.5, 0.5,-0.5};
GLfloat vertF[3] = {-0.5, 0.5,-0.5};
GLfloat vertG[3] = {-0.5,-0.5,-0.5};
GLfloat vertH[3] = { 0.5,-0.5,-0.5};

/* Human readable strings for the gestures */
static const char *gestures[] = {
    "-                       ",
    "Flick West > East       ",
    "Flick East > West       ",
    "Flick South > North     ",
    "Flick North > South     ",
    "Circle clockwise        ",
    "Circle counter-clockwise"
};

int frameCount = 0;
data_t data;

void updateDevice();

/*
 * project()
 * ------
 * Sets the projection
 */
void project() 
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if (toggleMode) {
    /* perspective */
    gluPerspective(fov,asp,dim/4,4*dim);
  }
  else {
    /* orthogonal projection*/
    glOrtho(-dim*asp,+dim*asp, -dim,+dim, -dim,+dim);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/*
 * drawAxes()
 * ------
 * Draw the axes
 */
void drawAxes(int enableX, int enableY, int enableZ) 
{
  if (toggleAxes) {
    /*  Length of axes */
    double len = 2.0;
    glBegin(GL_LINES);
    if(enableX){
    glColor3f(1.0,0,0);
    glVertex3d(0,0,0);
    glVertex3d(len,0,0);
    }
    if(enableY){
    glColor3f(0,1.0,0);
    glVertex3d(0,0,0);
    glVertex3d(0,len,0);
    }
    if(enableZ){
    glColor3f(0,0,1.0);
    glVertex3d(0,0,0);
    glVertex3d(0,0,len);
    }
    glEnd();
    /*  Label axes */
    if(enableX){
    glColor3f(1.0,0,0);
    glRasterPos3d(len,0,0);
    print("X");
    }
    if(enableY){ 
    glColor3f(0,1.0,0);
    glRasterPos3d(0,len,0);
    print("Y");
    }
    if(enableZ){
    glColor3f(0,0,1.0);
    glRasterPos3d(0,0,len);
    print("Z");
    }
  }
}

/*
 *  drawValues()
 *  ------
 *  Draw the values in the lower left corner
 */
void drawValues()
{
  if (toggleValues) {
    glColor3f(0.8,0.8,0.8);
    printAt(5,100,"Position %d %d %d", data.out_pos->x, data.out_pos->y, data.out_pos->z);
    printAt(5,50,"Geture %s", gestures[data.last_gesture]);
  }
}

/*
 *  drawShape()
 *  ------
 *  Draw the GLUT shape
 */
void drawShape()
{
  /* Cube */
  glBegin(GL_QUADS);
  /* front => ABCD yellow */
  glColor3f(1.0,1.0,0.0);
  glVertex3fv(vertA);
  glVertex3fv(vertB);
  glVertex3fv(vertC);
  glVertex3fv(vertD);
  /* back => FEHG red */
  glColor3f(1.0,0.0,0.0);
  glVertex3fv(vertF);
  glVertex3fv(vertE);
  glVertex3fv(vertH);
  glVertex3fv(vertG);
  /* right => EADH green */
  glColor3f(0.0,1.0,0.0);
  glVertex3fv(vertE);
  glVertex3fv(vertA);
  glVertex3fv(vertD);
  glVertex3fv(vertH);
  /* left => BFGC blue */
  glColor3f(0.0,0.0,1.0);
  glVertex3fv(vertB);
  glVertex3fv(vertF);
  glVertex3fv(vertG);
  glVertex3fv(vertC);
  /* top => EFBA turquoise */
  glColor3f(0.0,1.0,1.0);
  glVertex3fv(vertE);
  glVertex3fv(vertF);
  glVertex3fv(vertB);
  glVertex3fv(vertA);
  /* bottom => DCGH pink */
  glColor3f(1.0,0.0,1.0);
  glVertex3fv(vertD);
  glVertex3fv(vertC);
  glVertex3fv(vertG);
  glVertex3fv(vertH);
  glEnd();
}

void drawPoint(){
  float x = (float)data.out_pos->x / 65535.0f * 2;
  float y = (float)data.out_pos->y / 65535.0f * 2;
  float z = (float)data.out_pos->z / 65535.0f * 2;

  glPointSize(10.0f);
  glBegin(GL_POINTS);
  glColor3f(0.8,0.2,0.2);
  glVertex3d(x, y, z);
  glEnd();
}

/*
 *  display()
 *  ------
 *  Display the scene
 */
void display()
{
  /*  Clear the image */
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  /*  Enable Z-buffering in OpenGL */
  glEnable(GL_DEPTH_TEST);

  /*  Reset previous transforms */
  glLoadIdentity();
  
  glViewport(0,0,windowWidth,windowHeight);
  drawValues();

  /*  Orthogonal - set world orientation */
    glRotatef(ph,1,0,0);
    glRotatef(th,0,1,0);
    glRotatef(90,1,0,0);
    glRotatef(180,1,1,0);

  /*  Draw  */
  drawAxes(1,1,1);
  drawPoint();

  float width = windowWidth/3;
  float height = windowHeight/3;
// X-Y
  glViewport(0, 0, width,height);
  glLoadIdentity();
  drawAxes(1,1,0);
  drawPoint();

// Y-Z
  glViewport(width, 0, width,height);
  glLoadIdentity();
  glRotatef(90, 0,1,0);
  drawAxes(0,1,1);
  drawPoint();

// X-Z
  glViewport(width*2, 0, width,height);
  glLoadIdentity();
  glRotatef(-90,1,0,0);
  drawAxes(1,0,1);
  drawPoint();


  /*  Flush and swap */
  glFlush();
  glutSwapBuffers();

  updateDevice();
  glutPostRedisplay();
}

/*
 *  reshape()
 *  ------
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
  asp = (height>0) ? (double)width/height : 1;
  glViewport(0,0, width,height);
  project();
}

/*
 *  windowKey()
 *  ------
 *  GLUT calls this routine when a non-special key is pressed
 */
void windowKey(unsigned char key,int x,int y)
{
  /*  Exit on ESC */
  if (key == 27) {
    printf("Bye");
    free_device(&data);
    exit(0);
  }
  else if (key == 'a' || key == 'A') toggleAxes = 1-toggleAxes;
  else if (key == 'v' || key == 'V') toggleValues = 1-toggleValues;
  else if (key == 'm' || key == 'M') toggleMode = 1-toggleMode;
  /*  Change field of view angle */
  else if (key == '-' && key>1) fov--;
  else if (key == '+' && key<179) fov++;
  /*  Change dimensions */
  else if (key == 'D') dim += 0.1;
  else if (key == 'd' && dim>1) dim -= 0.1;

  project();
  glutPostRedisplay();
}

/*
 *  windowSpecial()
 *  ------
 *  GLUT calls this routine when an arrow key is pressed
 */
void windowSpecial(int key,int x,int y)
{
  /*  Right arrow key - increase azimuth by 5 degrees */
  if (key == GLUT_KEY_RIGHT) th += 5;
  /*  Left arrow key - decrease azimuth by 5 degrees */
  else if (key == GLUT_KEY_LEFT) th -= 5;
  /*  Up arrow key - increase elevation by 5 degrees */
  else if (key == GLUT_KEY_UP) ph += 5;
  /*  Down arrow key - decrease elevation by 5 degrees */
  else if (key == GLUT_KEY_DOWN) ph -= 5;

  /*  Keep angles to +/-360 degrees */
  th %= 360;
  ph %= 360;

  project();
  glutPostRedisplay();
}

/*
 *  windowMenu
 *  ------
 *  Window menu is the same as the keyboard clicks
 */
void windowMenu(int value)
{
  windowKey((unsigned char)value, 0, 0);
}

void updateDevice(){
  frameCount++;
  if(data.running)
  {
    update_device(&data);
  }
}

void init_data(data_t *data)
{
    /* The demo should be running */
    data->running = 1;

    /* Render menu on startup. */
    data->menu_current = -1;

    /* Automatic calibration enabled by default */
    data->auto_calib = 1;
}

void initDevice() {
  memset(&data, 0, sizeof(data));
  init_data(&data);
  init_device(&data);
}

/*
 *  main()
 *  ----
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
  initDevice();
  updateDevice();

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(windowWidth,windowHeight);
  glutInitWindowPosition(0,0);
  glutCreateWindow(windowName);

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(windowKey);
  glutSpecialFunc(windowSpecial);

  glutCreateMenu(windowMenu);
  glutAddMenuEntry("Toggle Axes [a]",'a');
  glutAddMenuEntry("Toggle Values [v]",'v');
  glutAddMenuEntry("Toggle Mode [m]",'m');
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  printf("Start !!!");
  glutMainLoop();

  return 0;
}
