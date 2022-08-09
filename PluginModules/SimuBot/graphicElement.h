#ifndef Element_H
#define Element_H

#include <QGraphicsItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QGraphicsScene>
#include <QObject>
#include <QtMath>
#include <QtTest/QTest>
#include <QTimer>

//#define DEBUG_INTERNAL_ASSERV
//#define DEBUG_INTERNAL_ASSERV_01

static const double Pi = 3.14159265358979323846264338327950288419717;
static const double TwoPi = 2.0 * Pi;

class Coord

{
public:
    Coord();
    Coord(float _x, float _y, float _teta, bool _ortho);
    void init(float _x, float _y, float _teta, bool _ortho);
    float x;
    float y;
    float teta;
    bool ortho;
};

class GraphicElement : public QObject , public QGraphicsPolygonItem
{
 Q_OBJECT

public:
    GraphicElement();
    GraphicElement(float R, float G, float B);
    GraphicElement(QPolygonF forme, float R, float G, float B);

    //QRectF boundingRect() const;
    //QPainterPath shape() const;
//    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
//               QWidget *widget);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    qreal angle;
    qreal x_reel;
    qreal y_reel;
    qreal x_offset, y_offset, angle_offset;
    qreal x_asserv_init, y_asserv_init, angle_asserv_init;
    QColor color;
    //pour les déplacements non instantanés (avec asservissement)
    bool asservEnabled;
    double vitesse;
    qreal x_internal_hold;
    qreal y_internal_hold;
    qreal teta_internal_hold;
    qreal x_internal_target;
    qreal y_internal_target;
    qreal teta_internal_target;
    float m_target;
    float p_target;
    int count_step_target;
    int nb_step_target;
    double dX_target;
    double dY_target;
    double force_G;
    double force_D;
    bool internalTargetsInitialized;

    void initInternalAsserv(float toX, float toY);

public:
    qreal getTheta(void);
    qreal getX(void);
    qreal getY(void);
    qreal getX_terrain(void);
    qreal getY_terrain(void);
    bool isRelativToBot;
    int sensOrtho;
    void setAsservInit(qreal x_init, qreal y_init, qreal angle_init);
    void setSpeed(double newVitesse);
    bool isConvergenceXY;
    bool isConvergenceTeta;

    double getErrorDistance(double x_target, double y_target);
    double getErrorAngle(double x_target, double y_target);
    double getErrorTeta(double teta_target);
    void setTargetXY(float toX, float toY);
    void setTargetTeta(float toTeta);
    void stepToTarget(float *forceG, float *forceD);
    void startInternalAsserv();
    void stopInternalAsserv();
    void getForcesAsserv(float *forceG, float *forceD);
signals:
    void center(qreal x_new, qreal y_new);
    void isDoubleClicked();
    void newConvergence();

public slots:
    void replace(qreal x_new, qreal y_new);
    void raz(qreal x_new, qreal y_new, qreal angle_new);
    void raz(Coord cTerrain, Coord cAsserv);
    void display_XY(qreal x_reel_new,qreal y_reel_new);
    //void display_Y(qreal y_reel_new);
    void display_theta(qreal theta_reel_new);
    void stepInternalAsserv(void);

};

#endif
