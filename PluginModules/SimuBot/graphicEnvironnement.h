#ifndef ENVIRONNEMENT_H
#define ENVIRONNEMENT_H

#include <QGraphicsScene>
#include "graphicElement.h"
#include <QPointF>

struct Coordonnees{
    float X;
    float Y;
    float theta;
};

class GraphicEnvironnement : public QGraphicsScene
{
    Q_OBJECT

public:
    GraphicEnvironnement();
    float fromDimensionReelle(float dim);
    float toDimensionReelle(float dim);
    static Coordonnees fromCoordonneesReelles(QPointF coord_reelles,float angle_reel);
    static Coordonnees toCoordonneesReelles(QPointF coord_view, float angle_view);

};

#endif // ENVIRONNEMENT_H
