/*! \file CPhysicalEngine.h
    \brief moteur 2d pour l'application
*/

#ifndef CPHYSICALENGINE_H
#define CPHYSICALENGINE_H
#include "box2d/box2d.h"
#include <QPointF>
#include <QPolygonF>

#define X_TERRAIN 300.0f
#define Y_TERRAIN 200.0f

#define DISTANCE_PAS_CODEUR_G 0.0651136f  // valeur par défaut reprise de GROSBOT
#define DISTANCE_PAS_CODEUR_D 0.0651136f
#define VOIE_BOT 31.6867261f
#define DISTANCE_PAS_CODEUR_G_2 0.00325568f*20  // valeur par défaut reprise de GROSBOT
#define DISTANCE_PAS_CODEUR_D_2 0.00325568f*20
#define VOIE_BOT_2 31.6867261f

class CApplication;

class CPhysicalEngine
{
public :
    CPhysicalEngine();

    void createPhysicalWorld(CApplication *application, QPolygonF bot1, QPolygonF bot2);
    void Init(float x_init1, float y_init1, float teta_init1, float x_init2, float y_init2, float teta_init2, bool enable2);
    void step(float schedule_lap, float vect_deplacement_G, float vect_deplacement_D,float vect_deplacement_G_2, float vect_deplacement_D_2, float x_recal=500., float y_recal=500.);
    float x_pos;
    float y_pos;
    float teta_pos;
    float x_pos_2;
    float y_pos_2;
    float teta_pos_2;
    int m_deltaRoue_G_bot1;
    int m_deltaRoue_D_bot1;
    int m_deltaRoue_G_bot2;
    int m_deltaRoue_D_bot2;
    QPointF getElement(int num);
    void activateBot2(bool flag);

    float getElementRotation(int num);
private :
    CApplication *m_application;
    b2World *realWorld;

    b2Body* elementsJeu[40];

    b2Body* m_bot1;
    b2Vec2 m_old_bot1_pos_G;
    b2Vec2 m_old_bot1_pos_D;
    b2Vec2 m_bot1_pos_G;
    b2Vec2 m_bot1_pos_D;
    float m_old_bot1_teta;
    QPolygonF m_shape_bot1;
    float m_x_init1;
    float m_y_init1;
    float m_teta_init1;

    bool m_bot2_activated;
    b2Body* m_bot2;
    b2Vec2 m_old_bot2_pos_G;
    b2Vec2 m_old_bot2_pos_D;
    b2Vec2 m_bot2_pos_G;
    b2Vec2 m_bot2_pos_D;
    float m_old_bot2_teta;
    QPolygonF m_shape_bot2;
    float m_x_init2;
    float m_y_init2;
    float m_teta_init2;

    float _x1(float x);
    float _y1(float y);
    float _x2(float x);
    float _y2(float y);
    b2Body *setElement(float x, float y);
    b2Vec2 getBotLateralVelocity(int numBot);
};

#endif // CPHYSICALENGINE_H
