/*! \file CPhysicalEngine.cpp
    \brief moteur 2d pour la simulation
*/
#include "CPhysicalEngine.h"
#include "CApplication.h"
#include "CDataManager.h"
#include <QRectF>
//___________________________________________________________________________
/*!
   \brief Constructeur

   \param --
   \return --
*/
CPhysicalEngine::CPhysicalEngine()
    :m_application(nullptr)
{

}

//___________________________________________________________________________
void CPhysicalEngine::createPhysicalWorld(CApplication *application, QPolygonF bot1, QPolygonF bot2)
{
    m_application = application;
    m_shape_bot1=bot1;
    m_shape_bot2=bot2;

    m_deltaRoue_G_bot1=0;
    m_deltaRoue_D_bot1=0;
    m_deltaRoue_G_bot2=0;
    m_deltaRoue_D_bot2=0;

    x_pos=0;
    y_pos=0;
    teta_pos=0;

    //comme pour l'asservissement on centre le repère de notre monde sur le centre
    //de notre robot
    //les offset x et y en paramètres nous permmet
    m_x_init1=0;
    m_y_init1=0;

    //coeff de friction des bordures pour eviter de glisser dessus
    float coeffFriction=3.0f;

    //par défaut box2d travaille dans le plan (X,Z) on leurre donc en créant
    //un vecteur gravité nul pour ramener le monde physique en (X,Y)
    b2Vec2 gravity(0.0f,0.0f);
    //initialisation du monde physique avec un vecteur gravité
    realWorld= new b2World(gravity);

    //création des bordures (ce sont les limites du monde de notre robot :-) )
    //comportement de la bordure dans le monde simulé
    b2BodyDef Definition_Bordure_gauche;
    Definition_Bordure_gauche.position.Set(-1,Y_TERRAIN/2);
    //ajout de la bordure au monde simulé
    b2Body* Corps_Bordure_gauche = realWorld->CreateBody(&Definition_Bordure_gauche);
    //caractéristiques physiques de la bordure
    b2PolygonShape Forme_Bordure_gauche;
    Forme_Bordure_gauche.SetAsBox(2.0f, Y_TERRAIN);
    b2FixtureDef Fixture_Bordure_gauche;
    Fixture_Bordure_gauche.shape = &Forme_Bordure_gauche;
    Fixture_Bordure_gauche.density = 1.0f;
    Fixture_Bordure_gauche.friction = coeffFriction;
    Corps_Bordure_gauche->CreateFixture(&Fixture_Bordure_gauche);

    //comportement de la bordure dans le monde simulé
    b2BodyDef Definition_Bordure_haute;
    Definition_Bordure_haute.position.Set(X_TERRAIN/2,Y_TERRAIN+1);
    //ajout de la bordure au monde simulé
    b2Body* Corps_Bordure_haute = realWorld->CreateBody(&Definition_Bordure_haute);
    //caractéristiques physiques de la bordure
    b2PolygonShape Forme_Bordure_haute;
    Forme_Bordure_haute.SetAsBox(X_TERRAIN+4,2.0f);
    b2FixtureDef Fixture_Bordure_haute;
    Fixture_Bordure_haute.shape = &Forme_Bordure_haute;
    Fixture_Bordure_haute.density = 1.0f;
    Fixture_Bordure_haute.friction =coeffFriction;
    Corps_Bordure_haute->CreateFixture(&Fixture_Bordure_haute);

    //comportement de la bordure dans le monde simulé
    b2BodyDef Definition_Bordure_droite;
    Definition_Bordure_droite.position.Set(X_TERRAIN,(Y_TERRAIN/2)+1);
    //ajout de la bordure au monde simulé
    b2Body* Corps_Bordure_droite = realWorld->CreateBody(&Definition_Bordure_droite);
    //caractéristiques physiques de la bordure
    b2PolygonShape Forme_Bordure_droite;
    Forme_Bordure_droite.SetAsBox(2.0f, Y_TERRAIN);
    b2FixtureDef Fixture_Bordure_droite;
    Fixture_Bordure_droite.shape = &Forme_Bordure_droite;
    Fixture_Bordure_droite.density = 1.0f;
    Fixture_Bordure_droite.friction = coeffFriction;
    Corps_Bordure_droite->CreateFixture(&Fixture_Bordure_droite);

    //comportement de la bordure dans le monde simulé
    b2BodyDef Definition_Bordure_basse;
    Definition_Bordure_basse.position.Set(X_TERRAIN/2,-1);
    //ajout de la bordure au monde simulé
    b2Body* Corps_Bordure_basse = realWorld->CreateBody(&Definition_Bordure_basse);
    //caractéristiques physiques de la bordure
    b2PolygonShape Forme_Bordure_basse;
    Forme_Bordure_basse.SetAsBox(X_TERRAIN+4,2.0f);
    b2FixtureDef Fixture_Bordure_basse;
    Fixture_Bordure_basse.shape = &Forme_Bordure_basse;
    Fixture_Bordure_basse.density = 1.0f;
    Fixture_Bordure_basse.friction = 1.0f;
    Corps_Bordure_basse->CreateFixture(&Fixture_Bordure_basse);

    m_old_bot1_pos_G.x=0.0f;
    m_old_bot1_pos_G.y=0.0f;
    m_old_bot1_pos_D.x=0.0f;
    m_old_bot1_pos_D.y=0.0f;
    m_bot1_pos_G.x=0.0f;
    m_bot1_pos_G.y=0.0f;
    m_bot1_pos_D.x=0.0f;
    m_bot1_pos_D.y=0.0f;

}

void CPhysicalEngine::Init(float x_init1, float y_init1, float teta_init1,float x_init2, float y_init2, float teta_init2,bool enable2)
{
    //reset de la mémoire des pas codeurs
    m_deltaRoue_G_bot1=0;
    m_deltaRoue_D_bot1=0;
    m_deltaRoue_G_bot2=0;
    m_deltaRoue_D_bot2=0;

    //realWorld->ClearForces();

    //on efface les anciens corps
    if(m_bot1)
        realWorld->DestroyBody(m_bot1);
    if(m_bot2)
        realWorld->DestroyBody(m_bot2);
    for(int i=0; i<24;i++)
    {
        if(elementsJeu[i])
            realWorld->DestroyBody(elementsJeu[i]);
    }

    //éléments de jeux
    elementsJeu[0]=setElement(30.0f,160.0f);
    elementsJeu[1]=setElement(45.0f,149.0f);
    elementsJeu[2]=setElement(67.0f,190.0f);
    elementsJeu[3]=setElement(95.0f,160.0f);

    elementsJeu[4]=setElement(30.0f,80.0f);
    elementsJeu[5]=setElement(45.0f,92.0f);
    elementsJeu[6]=setElement(110.0f,120.0f);
    elementsJeu[7]=setElement(127.0f,80.0f);

    elementsJeu[8]=setElement(100.5f,4.5f);
    elementsJeu[9]=setElement(106.5f,35.0f);
    elementsJeu[10]=setElement(133.5f,35.0f);
    elementsJeu[11]=setElement(139.5f,4.5f);

    elementsJeu[12]=setElement(270.0f,160.0f);
    elementsJeu[13]=setElement(255.0f,149.0f);
    elementsJeu[14]=setElement(230.0f,190.0f);
    elementsJeu[15]=setElement(205.0f,160.0f);

    elementsJeu[16]=setElement(270.0f,80.0f);
    elementsJeu[17]=setElement(255.0f,92.0f);
    elementsJeu[18]=setElement(190.0f,120.0f);
    elementsJeu[19]=setElement(173.0f,80.0f);

    elementsJeu[20]=setElement(199.5f,4.5f);
    elementsJeu[21]=setElement(193.5f,35.0f);
    elementsJeu[22]=setElement(166.5f,35.0f);
    elementsJeu[23]=setElement(160.5f,4.5f);

    //comme pour l'asservissement on centre le repère de notre monde sur le centre de notre robot
    //on utilisera donc les offset d'init x et y en paramètres
    m_x_init1=x_init1;
    m_y_init1=y_init1;
    m_x_init2=x_init2;
    m_y_init2=y_init2;
    m_teta_init1=teta_init1;
    m_teta_init2=teta_init2;

    //création du 1er robot
    //comportement du robot dans le monde simulé
    b2BodyDef Definition_Robot;
    Definition_Robot.type = b2_dynamicBody; //corps dynamique
    Definition_Robot.linearDamping = 8.0f; //frottement lineaire pour simuler une gravite Y
    Definition_Robot.angularDamping=2.0f; //idem pour l'angle
    Definition_Robot.position.Set(m_x_init1,m_y_init1); //position de depart
    Definition_Robot.angle=m_teta_init1;

    x_pos=0;
    y_pos=0;
    teta_pos=m_teta_init1;

    //ajout du robot au monde simulé
    m_bot1 = realWorld->CreateBody(&Definition_Robot);
    //caractéristiques physiques du robot
    b2PolygonShape Forme_Robot;
    int countShape=m_shape_bot1.count();
    if(countShape>8)
    {
        b2Vec2 botDefaultShape[8];
        botDefaultShape[0].x=10;botDefaultShape[0].y=16;
        botDefaultShape[1].x=-10;botDefaultShape[1].y=16;
        botDefaultShape[2].x=-15;botDefaultShape[2].y=10;
        botDefaultShape[3].x=-15;botDefaultShape[3].y=-10;
        botDefaultShape[4].x=-10;botDefaultShape[4].y=-16;
        botDefaultShape[5].x=10;botDefaultShape[5].y=-16;
        botDefaultShape[6].x=15;botDefaultShape[6].y=-10;
        botDefaultShape[7].x=15;botDefaultShape[7].y=10;
        Forme_Robot.Set(botDefaultShape,8);
    }
    else
    {
        b2Vec2 botShape[countShape];
        for(int i=0;i<countShape;i++)
        {
            botShape[i].x=m_shape_bot1.at(i).x();
            botShape[i].y=m_shape_bot1.at(i).y();
        }
        Forme_Robot.Set(botShape,countShape);
    }
    b2FixtureDef Fixture_Robot;
    Fixture_Robot.shape = &Forme_Robot;
    Fixture_Robot.density = 1.0f;
    Fixture_Robot.friction = 0.3f;
    m_bot1->CreateFixture(&Fixture_Robot);


    m_old_bot1_pos_G.x=0.0f;
    m_old_bot1_pos_G.y=0.0f;
    m_old_bot1_pos_D.x=0.0f;
    m_old_bot1_pos_D.y=0.0f;
    m_bot1_pos_G.x=0.0f;
    m_bot1_pos_G.y=0.0f;
    m_bot1_pos_D.x=0.0f;
    m_bot1_pos_D.y=0.0f;

    m_bot2_activated=enable2;

    if(m_bot2_activated)
    {
        //création du 2ème robot
        //comportement du robot dans le monde simulé
        b2BodyDef Definition_bot2;
        Definition_bot2.type = b2_dynamicBody; //corps dynamique
        Definition_bot2.linearDamping = 8.0f; //frottement lineaire pour simuler une gravite Y
        Definition_bot2.angularDamping=2.0f; //idem pour l'angle
        Definition_bot2.position.Set(m_x_init2,m_y_init2); //position de depart
        Definition_bot2.angle=m_teta_init2;
        x_pos_2=0;
        y_pos_2=0;
        teta_pos_2=m_teta_init2;
        //ajout du robot au monde simulé
        m_bot2 = realWorld->CreateBody(&Definition_bot2);
        //caractéristiques physiques du robot
        b2PolygonShape Forme_bot2;
        int countShape2=m_shape_bot2.count();
        if(countShape2>8)
        {
            b2Vec2 bot2DefaultShape[8];
            bot2DefaultShape[0].x=10;bot2DefaultShape[0].y=16;
            bot2DefaultShape[1].x=-10;bot2DefaultShape[1].y=16;
            bot2DefaultShape[2].x=-15;bot2DefaultShape[2].y=10;
            bot2DefaultShape[3].x=-15;bot2DefaultShape[3].y=-10;
            bot2DefaultShape[4].x=-10;bot2DefaultShape[4].y=-16;
            bot2DefaultShape[5].x=10;bot2DefaultShape[5].y=-16;
            bot2DefaultShape[6].x=15;bot2DefaultShape[6].y=-10;
            bot2DefaultShape[7].x=15;bot2DefaultShape[7].y=10;
            Forme_bot2.Set(bot2DefaultShape,8);
        }
        else
        {
            b2Vec2 bot2Shape[countShape2];
            for(int i=0;i<countShape2;i++)
            {
                bot2Shape[i].x=m_shape_bot2.at(i).x();
                bot2Shape[i].y=m_shape_bot2.at(i).y();
            }
            Forme_bot2.Set(bot2Shape,countShape2);
        }
        b2FixtureDef Fixture_bot2;
        Fixture_bot2.shape = &Forme_bot2;
        Fixture_bot2.density = 1.0f;
        Fixture_bot2.friction = 0.3f;
        m_bot2->CreateFixture(&Fixture_bot2);


        m_old_bot2_pos_G.x=0.0f;
        m_old_bot2_pos_G.y=0.0f;
        m_old_bot2_pos_D.x=0.0f;
        m_old_bot1_pos_D.y=0.0f;
        m_bot2_pos_G.x=0.0f;
        m_bot2_pos_G.y=0.0f;
        m_bot2_pos_D.x=0.0f;
        m_bot2_pos_D.y=0.0f;
    }
    //m_bot2->SetEnabled(enable2);
}

//ramène la coordonnée x dans le repère Terrain (orthogonal direct)
float CPhysicalEngine::_x1(float x) {return (x-m_x_init1);}
//ramène la coordonnée y dans le repère Terrain (orthogonal direct)
float CPhysicalEngine::_y1(float y) {return (y-m_y_init1);}
//ramène la coordonnée x dans le repère Terrain (orthogonal direct)
float CPhysicalEngine::_x2(float x) {return (x-m_x_init2);}
//ramène la coordonnée y dans le repère Terrain (orthogonal direct)
float CPhysicalEngine::_y2(float y) {return (y-m_y_init2);}

void CPhysicalEngine::step(float schedule_lap, float vect_deplacement_G, float vect_deplacement_D, float vect_deplacement_G_2, float vect_deplacement_D_2)
{
    m_deltaRoue_G_bot1=0;
    m_deltaRoue_D_bot1=0;
    m_deltaRoue_G_bot2=0;
    m_deltaRoue_D_bot2=0;

    m_old_bot1_pos_G.x=m_bot1->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT/2))).x;
    m_old_bot1_pos_G.y=m_bot1->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT/2))).y;
    m_old_bot1_pos_D.x=m_bot1->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT/2))).x;
    m_old_bot1_pos_D.y=m_bot1->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT/2))).y;
    m_old_bot1_teta=m_bot1->GetAngle();

    if(m_bot2_activated)
    {
        m_old_bot2_pos_G.x=m_bot2->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT_2/2))).x;
        m_old_bot2_pos_G.y=m_bot2->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT_2/2))).y;
        m_old_bot2_pos_D.x=m_bot2->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT_2/2))).x;
        m_old_bot2_pos_D.y=m_bot2->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT_2/2))).y;
        m_old_bot2_teta=m_bot2->GetAngle();
    }

    realWorld->ClearForces();

    m_bot1->ApplyLinearImpulse(m_bot1->GetWorldVector(b2Vec2(vect_deplacement_G,0)),
                             m_bot1->GetWorldPoint(b2Vec2(0.0f,12.0f)),
                             true);
    m_bot1->ApplyLinearImpulse(m_bot1->GetWorldVector(b2Vec2(vect_deplacement_D,0)),
                             m_bot1->GetWorldPoint(b2Vec2(0.0f,-12.0f)),
                             true);
    if(m_bot2_activated)
    {
        m_bot2->ApplyLinearImpulse(m_bot2->GetWorldVector(b2Vec2(vect_deplacement_G_2,0)),
                                 m_bot2->GetWorldPoint(b2Vec2(0.0f,8.0f)),
                                 true);
        m_bot2->ApplyLinearImpulse(m_bot2->GetWorldVector(b2Vec2(vect_deplacement_D_2,0)),
                                 m_bot2->GetWorldPoint(b2Vec2(0.0f,-8.0f)),
                                 true);
    }

    int32 velocityIterations =8;
    int32 positionIterations = 2;

    realWorld->Step(schedule_lap, velocityIterations, positionIterations);

    b2Vec2 position = m_bot1->GetPosition();
    float angle = m_bot1->GetAngle();

    x_pos=_x1(position.x);
    y_pos=_y1(position.y);
    teta_pos=angle;

    m_bot1_pos_G.x=m_bot1->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT/2))).x;
    m_bot1_pos_G.y=m_bot1->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT/2))).y;
    m_bot1_pos_D.x=m_bot1->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT/2))).x;
    m_bot1_pos_D.y=m_bot1->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT/2))).y;

    //delta codeur Gauche
    int sens_G=1;
    if ((cos(angle)*(m_bot1_pos_G.x-m_old_bot1_pos_G.x)) >=0)
        sens_G=1;
    else
        sens_G=-1;
    m_deltaRoue_G_bot1=(sens_G * sqrt(pow((m_bot1_pos_G.x-m_old_bot1_pos_G.x),2)+pow((m_bot1_pos_G.y-m_old_bot1_pos_G.y),2)))/(DISTANCE_PAS_CODEUR_G);

    //delta codeur Droit
    int sens_D=1;
    if ((cos(angle)*(m_bot1_pos_D.x-m_old_bot1_pos_D.x)) >=0)
        sens_D=1;
    else
        sens_D=-1;
    m_deltaRoue_D_bot1=(sens_D * sqrt(pow((m_bot1_pos_D.x-m_old_bot1_pos_D.x),2)+pow((m_bot1_pos_D.y-m_old_bot1_pos_D.y),2)))/(DISTANCE_PAS_CODEUR_D);

    if(m_bot2_activated)
    {
        b2Vec2 position2 = m_bot2->GetPosition();
        float angle2 = m_bot2->GetAngle();

        x_pos_2=_x2(position2.x);
        y_pos_2=_y2(position2.y);
        teta_pos_2=angle2;

        m_bot2_pos_G.x=m_bot2->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT_2/2))).x;
        m_bot2_pos_G.y=m_bot2->GetWorldPoint(b2Vec2(0.0f,(VOIE_BOT_2/2))).y;
        m_bot2_pos_D.x=m_bot2->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT_2/2))).x;
        m_bot2_pos_D.y=m_bot2->GetWorldPoint(b2Vec2(0.0f,-(VOIE_BOT_2/2))).y;

        //delta codeur Gauche
        int sens_G_2=1;
        if ((cos(angle)*(m_bot2_pos_G.x-m_old_bot2_pos_G.x)) >=0)
            sens_G_2=1;
        else
            sens_G_2=-1;
        m_deltaRoue_G_bot2=(sens_G_2 * sqrt(pow((m_bot2_pos_G.x-m_old_bot2_pos_G.x),2)+pow((m_bot2_pos_G.y-m_old_bot2_pos_G.y),2)))/(DISTANCE_PAS_CODEUR_G_2);

        //delta codeur Droit
        int sens_D_2=1;
        if ((cos(angle)*(m_bot2_pos_D.x-m_old_bot2_pos_D.x)) >=0)
            sens_D_2=1;
        else
            sens_D_2=-1;
        m_deltaRoue_D_bot2=(sens_D_2 * sqrt(pow((m_bot2_pos_D.x-m_old_bot2_pos_D.x),2)+pow((m_bot2_pos_D.y-m_old_bot2_pos_D.y),2)))/(DISTANCE_PAS_CODEUR_D_2);

    }
}

QPointF CPhysicalEngine::getElement(int num)
{
    //QPointF elementCoord(_x(elementsJeu[num]->GetPosition().x),_y(elementsJeu[num]->GetPosition().y));
    QPointF elementCoord(elementsJeu[num]->GetPosition().x,elementsJeu[num]->GetPosition().y);

    return elementCoord;

}


b2Body* CPhysicalEngine::setElement(float x, float y)
{
    b2Body* nouveauElement;
    b2BodyDef Definition_ElJeu;
    Definition_ElJeu.type = b2_dynamicBody; //corps dynamique
    Definition_ElJeu.linearDamping = 5.0f; //frottement lineaire pour simuler une gravite Y
    Definition_ElJeu.angularDamping=5.0f; //idem pour l'angle
    Definition_ElJeu.position.Set(x,y); //position de depart
    //ajout du robot au monde simulé
    nouveauElement = realWorld->CreateBody(&Definition_ElJeu);
    //caractéristiques physiques du robot
    b2CircleShape Forme_ElJeu;
    Forme_ElJeu.m_radius=3.6f;
    b2FixtureDef Fixture_ElJeu;
    Fixture_ElJeu.shape = &Forme_ElJeu;
    Fixture_ElJeu.density = 1.0f;
    Fixture_ElJeu.friction = 0.3f;
    nouveauElement->CreateFixture(&Fixture_ElJeu);

    return nouveauElement;
}

void CPhysicalEngine::activateBot2(bool flag)
{
    m_bot2_activated=flag;
    if(m_bot2)
        realWorld->DestroyBody(m_bot2);
    if(m_bot2_activated)
    {
        //création du 2ème robot
        //comportement du robot dans le monde simulé
        b2BodyDef Definition_bot2;
        Definition_bot2.type = b2_dynamicBody; //corps dynamique
        Definition_bot2.linearDamping = 8.0f; //frottement lineaire pour simuler une gravite Y
        Definition_bot2.angularDamping=2.0f; //idem pour l'angle
        Definition_bot2.position.Set(m_x_init2,m_y_init2); //position de depart
        Definition_bot2.angle=m_teta_init2;
        x_pos_2=0;
        y_pos_2=0;
        teta_pos_2=m_teta_init2;
        //ajout du robot au monde simulé
        m_bot2 = realWorld->CreateBody(&Definition_bot2);
        //caractéristiques physiques du robot
        b2PolygonShape Forme_bot2;
        int countShape2=m_shape_bot2.count();
        if(countShape2>8)
        {
            b2Vec2 bot2DefaultShape[8];
            bot2DefaultShape[0].x=10;bot2DefaultShape[0].y=16;
            bot2DefaultShape[1].x=-10;bot2DefaultShape[1].y=16;
            bot2DefaultShape[2].x=-15;bot2DefaultShape[2].y=10;
            bot2DefaultShape[3].x=-15;bot2DefaultShape[3].y=-10;
            bot2DefaultShape[4].x=-10;bot2DefaultShape[4].y=-16;
            bot2DefaultShape[5].x=10;bot2DefaultShape[5].y=-16;
            bot2DefaultShape[6].x=15;bot2DefaultShape[6].y=-10;
            bot2DefaultShape[7].x=15;bot2DefaultShape[7].y=10;
            Forme_bot2.Set(bot2DefaultShape,8);
        }
        else
        {
            b2Vec2 bot2Shape[countShape2];
            for(int i=0;i<countShape2;i++)
            {
                bot2Shape[i].x=m_shape_bot2.at(i).x();
                bot2Shape[i].y=m_shape_bot2.at(i).y();
            }
            Forme_bot2.Set(bot2Shape,countShape2);
        }
        b2FixtureDef Fixture_bot2;
        Fixture_bot2.shape = &Forme_bot2;
        Fixture_bot2.density = 1.0f;
        Fixture_bot2.friction = 0.3f;
        m_bot2->CreateFixture(&Fixture_bot2);


        m_old_bot2_pos_G.x=0.0f;
        m_old_bot2_pos_G.y=0.0f;
        m_old_bot2_pos_D.x=0.0f;
        m_old_bot1_pos_D.y=0.0f;
        m_bot2_pos_G.x=0.0f;
        m_bot2_pos_G.y=0.0f;
        m_bot2_pos_D.x=0.0f;
        m_bot2_pos_D.y=0.0f;
    }
}
