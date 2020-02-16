#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <memory>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QOpenGLFunctions>

#include "OPI/opi_cpp.h"

struct tColor {
    float r;
    float g;
    float b;
};

class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLCanvas(QWidget *parent = 0);
    //void updatePopulationPointer(std::shared_ptr<OPI::Population> p);
    void drawObject(OPI::Vector3 pos);
    void setOrbit(OPI::Orbit o, OPI::Vector3 p);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

private:
    void draw();
    void drawOrbit(const OPI::Orbit o, const tColor c);
    void drawCoordinateSystem();
    //void drawPopulation();

    OPI::Orbit orbit1;
    OPI::Orbit orbit2;
    OPI::Vector3 pos1;
    OPI::Vector3 pos2;

    //std::shared_ptr<OPI::Population> population;

    bool drawOrbits;

    struct pos2D {
        int x;
        int y;
    };
    pos2D rightClickPosition;
    pos2D middleClickPosition;
    float rotationY;
    float zoom;



signals:

public slots:    

};

#endif // GLCANVAS_H
