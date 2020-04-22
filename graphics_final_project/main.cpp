#define GL_SILENCE_DEPRECATION // silence deprecation warnings

float M_PI = 3.1415926535;

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include "Bmp.h"
#include "Cylinder.h"
#include "Icosphere.h"

// GLUT CALLBACK functions
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void toOrtho();
void toPerspective();
GLuint loadTexture(const char* fileName, bool wrap=true);


// constants
const int   SCREEN_WIDTH    = 1000;
const int   SCREEN_HEIGHT   = 700;
const float CAMERA_DISTANCE = 4.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;


// global variables
void *font = GLUT_BITMAP_8_BY_13;
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
int drawMode;
GLuint texId;
int imageWidth;
int imageHeight;

// cylinder: min sectors = 3, min stacks = 1
Cylinder cylinder1(0.069f, 0.069f, 1.4f, 70, 8, false); // baseRadius, topRadius, height, sectors, stacks, flat shading
Cylinder cylinder2(0.069f, 0.069f, 2.2f, 70, 8, false);        // baseRadius, topRadius, height, sectors, stacks, smooth(default)
Cylinder cylinder3(0.069f, 0.069f, 2.0f, 70, 8, false);        // baseRadius, topRadius, height, sectors, stacks, smooth(default)
Cylinder cylinder4(0.069f, 0.069f, 2.2f, 70, 8, false);        // baseRadius, topRadius, height, sectors, stacks, smooth(default)
Cylinder cylinder5(0.069f, 0.069f, 1.0f, 70, 8, false);        // baseRadius, topRadius, height, sectors, stacks, smooth(default)
int subdivision = 5;
Icosphere sphere(0.050601, subdivision, false);    // radius, subdivision, smooth
Icosphere sphere2(0.069f, subdivision, false);    // radius, subdivision, smooth

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // init global vars
    initSharedMem();

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();

    
    
    // load BMP image
    texId = loadTexture("grid512.bmp", true);

    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    
//    gluLookAt(0, 0, 300, 0, 0, 0, 0, 1, 0);
    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);

    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

//      track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points

    //cylinder1.setBaseRadius(2);
    //cylinder1.setTopRadius(2);
    //cylinder1.setHeight(2);
    cylinder2.printSelf();

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up global vars
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.5f, .5f, .5f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.8f, .8f, .8f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 1, 0}; // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// load raw image as a texture
///////////////////////////////////////////////////////////////////////////////
GLuint loadTexture(const char* fileName, bool wrap)
{
    Image::Bmp bmp;
    if(!bmp.read(fileName))
        return 0;     // exit if failed load image

    // get bmp info
    int width = bmp.getWidth();
    int height = bmp.getHeight();
    const unsigned char* data = bmp.getDataRGB();
    GLenum type = GL_UNSIGNED_BYTE;    // only allow BMP with 8-bit per channel

    // We assume the image is 8-bit, 24-bit or 32-bit BMP
    GLenum format;
    int bpp = bmp.getBitCount();
    if(bpp == 8)
        format = GL_LUMINANCE;
    else if(bpp == 24)
        format = GL_RGB;
    else if(bpp == 32)
        format = GL_RGBA;
    else
        return 0;               // NOT supported, exit

    // gen texture ID
    GLuint texture;
    glGenTextures(1, &texture);

    // set active texture and configure it
    glBindTexture(GL_TEXTURE_2D, texture);

    // select modulate to mix texture with color for shading
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // copy texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    //glGenerateMipmap(GL_TEXTURE_2D);

    // build our texture mipmaps
    switch(bpp)
    {
    case 8:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 1, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
        break;
    case 24:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
        break;
    case 32:
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        break;
    }

    return texture;
}



///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo()
{
    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    //gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1); // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);

    ss << "Base Radius: " << cylinder2.getBaseRadius() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight-TEXT_HEIGHT, color, font);
    ss.str("");

    ss << "Top Radius: " << cylinder2.getTopRadius() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight-(2*TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Height: " << cylinder2.getHeight() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight-(3*TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Sector Count: " << cylinder2.getSectorCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight-(4*TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Stack Count: " << cylinder2.getStackCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight-(5*TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Vertex Count: " << cylinder2.getVertexCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight-(6*TEXT_HEIGHT), color, font);
    ss.str("");

    ss << "Index Count: " << cylinder2.getIndexCount() << std::ends;
    drawString(ss.str().c_str(), 1, screenHeight-(7*TEXT_HEIGHT), color, font);
    ss.str("");

    drawString("Press SPACE to change sectors/stacks.", 1, 1, color, font);

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/*scalex - scaling of sphere around x-axis
  scaley - scaling of sphere around y-axis
  r - radius of sphere
 */
void drawHalfSphere(int scaley, int scalex, GLfloat r) {
  int i, j;
  GLfloat v[scalex*scaley][3];

  for (i=0; i<scalex; ++i) {
    for (j=0; j<scaley; ++j) {
      v[i*scaley+j][0]=r*cos(j*2*M_PI/scaley)*cos(i*M_PI/(2*scalex));
      v[i*scaley+j][1]=r*sin(i*M_PI/(2*scalex));
      v[i*scaley+j][2]=r*sin(j*2*M_PI/scaley)*cos(i*M_PI/(2*scalex));
    }
  }

  glBegin(GL_QUADS);
    for (i=0; i<scalex-1; ++i) {
      for (j=0; j<scaley; ++j) {
        glVertex3fv(v[i*scaley+j]);
        glVertex3fv(v[i*scaley+(j+1)%scaley]);
        glVertex3fv(v[(i+1)*scaley+(j+1)%scaley]);
        glVertex3fv(v[(i+1)*scaley+j]);
      }
    }
  glEnd();
}

std::vector<float> cross(std::vector<float> a, std::vector<float> b)
{
    std::vector<float> return_vec;
    return_vec.push_back(a[1]*b[2] - a[2]*b[1]);
    return_vec.push_back(a[2]*b[0] - a[0]*b[2]);
    return_vec.push_back(a[0]*b[1] - a[1]*b[0]);
    return return_vec;
}

void draw_cylinder(GLfloat radius,
                   GLfloat height,
                   GLubyte R,
                   GLubyte G,
                   GLubyte B)
{
    GLfloat x              = 0.0;
    GLfloat y              = 0.0;
    GLfloat angle          = 0.0;
    GLfloat angle_stepsize = 0.1;

    /** Draw the tube */
    glColor3d(R, G, B);
    glBegin(GL_QUAD_STRIP);
    angle = 0.0;
        while( angle < 2*M_PI ) {
            x = radius * cos(angle);
            y = radius * sin(angle);
            glVertex3f(x, y , height);
            glVertex3f(x, y , 0.0);
            angle = angle + angle_stepsize;
        }
        glVertex3f(radius, 0.0, height);
        glVertex3f(radius, 0.0, 0.0);
    glEnd();

    /** Draw the circle on top of cylinder */
    glColor3d(R, G, B);
    glBegin(GL_POLYGON);
    angle = 0.0;
        while( angle < 2*M_PI )
        {
            x = radius * cos(angle);
            y = radius * sin(angle);
            glVertex3f(x, y , height);
            angle = angle + angle_stepsize;
        }
        glVertex3f(radius, 0.0, height);
    glEnd();
}

void cylinder_between(float x1, float y1, float z1, float x2, float y2, float z2, float rad1, float rad2)
{
    std::vector<float> v = {x2-x1, y2-y1, z2-z1};
    float height = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    std::vector<float> axis;
    if(hypot(v[0], v[1]) < 0.001)
    {
        axis.push_back(1);
        axis.push_back(0);
        axis.push_back(0);
    }
    else{
        axis = cross(v, {0, 0, 1});
    }
    float angle = -atan2(hypot(v[0], v[1]), v[2])*180/M_PI;
    
    glPushMatrix();
    glTranslated(x1, y1, z1);
    glRotated(angle, axis[0], axis[1], axis[2]);
//    Cylinder cylinderx(rad1, rad2, height, 70, 8, false); // baseRadius, topRadius, height, sectors, stacks, flat shading
//    cylinderx.draw();
    draw_cylinder(rad1, height, 255, 255, 255);
//    glutSolidCone(rad1, height, 32, 16);
    glPopMatrix();
}

//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//    glPopMatrix();
    
    
    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform camera
    glTranslatef(0, 0, -cameraDistance);
//    glRotatef(cameraAngleX, 1, 0, 0);   // pitch
//    glRotatef(cameraAngleY, 0, 1, 0);   // heading

    // transform model
//    glRotatef(90, 1, 0, 0);

    // set material
    float ambient[]  = {0.5f, 0.5f, 0.5f, 1};
    float diffuse[]  = {0.8f, 0.8f, 0.8f, 1};
    float specular[] = {1.0f, 1.0f, 1.0f, 1};
    float shininess  = 128;
    glMaterialfv(GL_FRONT, GL_AMBIENT,   ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    // line color
    float lineColor[] = {0.2f, 0.2f, 0.2f, 1};
    
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);
    
    cylinder_between(0, 0, 0, 0, 1, 1, 0.067, .070);
    cylinder_between(0, 0, 0, 0, 0, 1, 0.067, .070);
    cylinder_between(0, 0, 0, 1, 0, 1, 0.067, .070);
    cylinder_between(0, 0, 0, 1, 1, 1, 0.067, .070);
    
    glPushMatrix();
    glColor3f(1, 0, 0);
    sphere2.draw();
    glColor3f(1, 1, 1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(0, 1, 1);
    glColor3f(1, 0, 0);
    sphere2.draw();
    glColor3f(1, 1, 1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(0, 0, 1);
    glColor3f(1, 0, 0);
    sphere2.draw();
    glColor3f(1, 1, 1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(1, 0, 1);
    glColor3f(1, 0, 0);
    sphere2.draw();
    glColor3f(1, 1, 1);
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(1, 1, 1);
    glColor3f(1, 0, 0);
    sphere2.draw();
    glColor3f(1, 1, 1);
    glPopMatrix();
    
//    glPushMatrix();
//    glColor3f(1, 0, 0);
//    sphere2.draw();
//    glColor3f(1, 1, 1);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(0, 0.85, 0.015);
//    glColor3f(1, 0, 0);
//    sphere2.draw();
//    glColor3f(1, 1, 1);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(0, 0, 0.283);
//    glRotated(90, 0, 1, 0);
//    cylinder5.draw();
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(0.5, 0, 0.283);
//    glColor3f(1, 0, 0);
//    sphere2.draw();
//    glColor3f(1, 1, 1);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(-0.5, 0, 0.283);
//    glColor3f(1, 0, 0);
//    sphere2.draw();
//    glColor3f(1, 1, 1);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(0, 0, -0.583);
//    glColor3f(1, 0, 0);
//    sphere2.draw();
//    glColor3f(1, 1, 1);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(0.2499, 0, -0.15);
//    glRotated(30, 0, 1, 0);
//    cylinder5.draw();
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(-0.2499, 0, -0.15);
//    glRotated(-30, 0, 1, 0);
//    cylinder5.draw();
//    glPopMatrix();
//
//
//
//    glPushMatrix();
//    glTranslated(0.26, 0.41, 0.15);
//    glRotated(90, 1, 0, 0);
//    glRotated(30, 0, 1, 0);
//    glRotated(-15, 1, 0, 0);
//    cylinder5.draw();
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(-0.26, 0.41, 0.15);
//    glRotated(90, 1, 0, 0);
//    glRotated(-30, 0, 1, 0);
//    glRotated(-15, 1, 0, 0);
//    cylinder5.draw();
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslated(0, 0.41, -0.29);
//    glRotated(90, 1, 0, 0);
//    glRotated(35, 1, 0, 0);
//    cylinder5.draw();
//    glPopMatrix();

    /*
    // using GLU quadric object
    GLUquadricObj* obj = gluNewQuadric();
    gluQuadricDrawStyle(obj, GLU_FILL);  GLU_FILL, GLU_LINE, GLU_SILHOUETTE, GLU_POINT
    gluQuadricNormals(obj, GL_SMOOTH);
    gluQuadricTexture(obj, GL_TRUE);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    gluCylinder(obj, 2.0, 2.0, 2.0 50, 50);  base, top, height, slices, stacks
    */

    glBindTexture(GL_TEXTURE_2D, 0);

//    showInfo();     // print max range of glDrawRangeElements

    glPopMatrix();

    glutSwapBuffers();
}

/*
    Function: frame_buffer_coordinates
 
    Description: From the viewport, gets the frame buffer
    width and height.
 
    Parameters: None
 
    Pre-Conditions: None
 
    Post-Conditions: None
 
    Returns: Tuple of GLint, GLint. The first value being
    frame buffer width, second being frame buffer height.
 */
std::tuple<GLint, GLint> frame_buffer_coordinates()
{
    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint fbWidth = dims[2];
    GLint fbHeight = dims[3];
    
    return std::make_tuple(fbWidth, fbHeight);
} // end frame_buffer_coordinates

void reshapeCB(int w, int h)
{
    GLint fbWidth;
    GLint fbHeight;
    std::tie(fbWidth, fbHeight) = frame_buffer_coordinates();

    glViewport (0, 0, (GLsizei) fbWidth, (GLsizei) fbHeight);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(30, (GLfloat) fbWidth/(GLfloat) fbHeight, 1.0, 23.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef (0.0, 0.0, -5.0);
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        clearSharedMem();
        exit(0);
        break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        drawMode = ++drawMode % 3;
        if(drawMode == 0)        // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            //glEnable(GL_LIGHTING);
       }
        else if(drawMode == 1)  // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            //glDisable(GL_LIGHTING);
        }
        else                    // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            //glDisable(GL_LIGHTING);
        }
        break;

    case ' ':
    {
        int count = cylinder1.getSectorCount();
        if(count < 36)
            count += 4;
        else
            count = 4;
        cylinder1.setSectorCount(count);
        cylinder2.setSectorCount(count);
        cylinder1.setStackCount(count/4);
        cylinder2.setStackCount(count/4);
        break;
    }

    default:
        ;
    }
}


void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotionCB(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}
