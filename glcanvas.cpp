#include "glcanvas.h"

#include <vector>
#include <iostream>
#include <cmath>

GLCanvas::GLCanvas(QWidget *parent) :
    QOpenGLWidget(parent)
{
    //population = NULL;
    drawOrbits = false;
    orbit1 = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    orbit2 = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    rotationY = 0.0;
}

void GLCanvas::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);

    static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    //earth.load("earth");
    //satellite.load("simpleSatellite");
}

void GLCanvas::paintGL()
{
    float xRot = 0.0;
    float yRot = 0.0;
    float zRot = -27.5;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -50-zoom);
    glRotatef(xRot, 1.0, 0.0, 0.0);
    glRotatef(rotationY, 0.0, 1.0, 0.0);
    glRotatef(zRot, 0.0, 0.0, 1.0);
    //glScalef(0.2,0.2,0.2);

    //earth.draw();

    glRotatef(90.0, 1.0, 0.0, 0.0); // flip coordinate system
    drawOrbit(orbit1, {1.0, 1.0, 0.0});
    drawOrbit(orbit2, {1.0, 1.0, 1.0});
    drawObject(pos1);
    drawObject(pos2);
    //drawPopulation();
    drawCoordinateSystem();
}

void GLCanvas::resizeGL(int width, int height)
{
    const float aspectRatio =
            (width >= height) ? (float)width / (float)height : (float)height / (float)width;

    const float fov = 40.0;
    const float nearClip = 0.1;
    const float farClip = 6000.0;

    const float top = nearClip * tan(fov/360.0 * M_PI);
    const float bottom = top * -1.0;
    const float right = aspectRatio * top;
    const float left = right * -1.0;

    //const float viewportSize = 20.0;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (width >= height)
        glFrustum(left, right, bottom, top, nearClip, farClip);
    else
        glFrustum(bottom, top, left, right, nearClip, farClip);

#ifdef QT_OPENGL_ES_1
    //glOrthof(-2, +2, -2, +2, 1.0, 15.0);
    //glOrthof(0.0, width, 0.0, height, -1.0, 1.0);
#else
    //glOrtho(-2, +2, -2, +2, 1.0, 15.0);
#endif
    glMatrixMode(GL_MODELVIEW);
}

void GLCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::RightButton)
    {
        int deltaX = event->x() - rightClickPosition.x;
        rotationY = fmod((float)deltaX, 360.0);
        repaint();
    }
    if (event->buttons() == Qt::MiddleButton)
    {
        int deltaY = event->y() - middleClickPosition.y;
        zoom = deltaY;
        repaint();
    }
}

void GLCanvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        rightClickPosition = { event->x(), event->y() };
    }
    if (event->button() == Qt::MiddleButton)
    {
        middleClickPosition = { event->x(), event->y() };
    }
}

void GLCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    // nothing to do here at the moment
}

void GLCanvas::drawObject(OPI::Vector3 pos)
{
    glPushMatrix();
    //glTranslated(pos.x, pos.y, pos.z);
    glScalef(0.3,0.3,0.3);
    //satellite.draw();
    glPopMatrix();
}

void GLCanvas::setOrbit(OPI::Orbit o, OPI::Vector3 p)
{
    orbit1 = o;
    pos1 = p;
}

void GLCanvas::drawOrbit(const OPI::Orbit o, const tColor c)
{
    const float rad2deg = 180.0 / M_PI;
    const float deg2rad = M_PI / 180.0;
    const float radiusMajor = o.semi_major_axis / 1000.0;
    const float radiusMinor = radiusMajor * sqrt(1.0 - pow(o.eccentricity, 2.0));
    const float focalDistance = sqrt(pow(radiusMajor, 2.0) - pow(radiusMinor, 2.0));
    const float perigeeDistance = fmod((o.raan + o.arg_of_perigee + M_PI)*rad2deg, 360.0);

    std::vector<float> vertices;
    for (int i=0; i<=360; i+=6)
    {
        vertices.push_back(cos(i*deg2rad) * radiusMajor);
        vertices.push_back(sin(i*deg2rad) * radiusMinor);
        vertices.push_back(0.0);
    }

    GLuint orbitVBO;
    glGenBuffers(1, &orbitVBO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glPushMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(c.r, c.g, c.b);
    glRotatef(o.inclination*rad2deg, cos(o.raan), sin(o.raan), 0.0);
    glRotatef(perigeeDistance, 0.0, 0.0, 1.0);
    glTranslatef(focalDistance, 0.0, 0.0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glVertexPointer(3, GL_FLOAT, 0, NULL);

    glDrawArrays(GL_LINE_LOOP, 0, vertices.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glPopMatrix();    
}

/*
void GLCanvas::drawPopulation()
{
    glPushMatrix();
    // The propagators generate cartesian coordinates in the ECI reference
    // frame (z axis towards North pole) while OpenGL uses a right-hand
    // system (y axis points upwards). This means that the coordinate system
    // must be rotated before drawing.
    // TODO: Specify coordinate system in OPI.
    glRotatef(90.0, 1.0, 0.0, 0.0);
    if (population != NULL)
    {
        for (int i=0; i<population->getSize(); i++)
        {
            OPI::Vector3 pos = population->getCartesianPosition()[i]/1000.0;
            drawObject(pos.x, pos.y, pos.z);
            if (drawOrbits) {
                OPI::Orbit orbit = population->getOrbit()[i];
                if (orbit.eol < 1.0) drawOrbit(orbit);
            }
        }
    }
    // revert the rotation here
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glPopMatrix();
}
*/

void GLCanvas::drawCoordinateSystem()
{
    int lineLength = 40;
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);
        glVertex3f(-lineLength, 0.0, 0.0);
        glVertex3f(lineLength, 0.0, 0.0);
        glColor3f(0.0, 1.0, 0.0);
        glVertex3f(0.0, -lineLength, 0.0);
        glVertex3f(0.0, lineLength, 0.0);
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(0.0, 0.0, -lineLength);
        glVertex3f(0.0, 0.0, lineLength);
    glEnd();
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

/*
void GLCanvas::setDrawOrbits(bool state)
{
    drawOrbits = state;
    repaint();
}
*/
