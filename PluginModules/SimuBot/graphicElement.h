#ifndef Element_H
#define Element_H

#include <QGraphicsItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QGraphicsScene>
#include <QObject>
#include <QtTest/QTest>

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



public:
    qreal getTheta(void);
    qreal getX(void);
    qreal getY(void);
    qreal getX_terrain(void);
    qreal getY_terrain(void);
    bool isRelativToBot;
   int sensOrtho;
   void setAsservInit(qreal x_init, qreal y_init, qreal angle_init);

signals:
    void center(qreal x_new, qreal y_new,float speed);

public slots:
    void replace(qreal x_new, qreal y_new,float speed);
    void raz(qreal x_new, qreal y_new, qreal angle_new);
    void raz(Coord cTerrain, Coord cAsserv);
    void display_XY(qreal x_reel_new,qreal y_reel_new);
    //void display_Y(qreal y_reel_new);
    void display_theta(qreal theta_reel_new);
};

#endif
