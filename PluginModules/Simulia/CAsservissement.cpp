#include <QDebug>

#include "CAsservissement.h"


CAsservissement::CAsservissement()
{

}

CAsservissement::~CAsservissement()
{

}

// _________________________________________________________
float CAsservissement::BornageAngle(float angle)
{
    qDebug() << "CAsservissement::BornageAngle(" << angle << ")";
    return angle;
}
// _________________________________________________________
void CAsservissement::CommandeMouvementXY(float x, float y)
{
    qDebug() << "CAsservissement::CommandeMouvementXY(" << x << ", " << y << ")";
}
// _________________________________________________________
void CAsservissement::CommandeMouvementDistanceAngle(float distance, float angle)
{
    qDebug() << "CAsservissement::CommandeMouvementDistanceAngle(" << distance << ", " << angle << ")";
}
// _________________________________________________________
void CAsservissement::CommandeManuelle(float cde_G, float cde_D)
{
    qDebug() << "CAsservissement::CommandeManuelle(" << cde_G << ", " << cde_D << ")";
}
// _________________________________________________________
void CAsservissement::CommandeMouvementXY_A(float x, float y)
{
    qDebug() << "CAsservissement::CommandeMouvementXY_A(" << x << ", " << y << ")";
}
// _________________________________________________________
void CAsservissement::CommandeMouvementXY_B(float x, float y)
{
    qDebug() << "CAsservissement::CommandeMouvementXY_B(" << x << ", " << y << ")";
}
// _________________________________________________________
void CAsservissement::CommandeMouvementXY_TETA(float x, float y, float teta)
{
    qDebug() << "CAsservissement::CommandeMouvementXY_TETA(" << x << ", " << y << "," << teta << ")";
}
// _________________________________________________________
void CAsservissement::CommandeVitesseMouvement(float vit_avance, float vit_angle)
{
    qDebug() << "CAsservissement::CommandeVitesseMouvement(" << vit_avance << ", " << vit_angle << ")";
}
// _________________________________________________________
void CAsservissement::CalculsMouvementsRobots(void)
{
}
// _________________________________________________________
void CAsservissement::setPosition_XYTeta(float x, float y, float teta)
{
    qDebug() << "CAsservissement::setPosition_XYTeta(" << x << ", " << y << ", " << teta << ")";
}
// _________________________________________________________
void CAsservissement::setIndiceSportivite(float idx)
{
    qDebug() << "CAsservissement::setIndiceSportivite(" << idx << ")";
}
// _________________________________________________________
void CAsservissement::setCdeMinCdeMax(int min, int max)
{
    qDebug() << "CAsservissement::setCdeMinCdeMax(" << min << ", " << max << ")";
}

