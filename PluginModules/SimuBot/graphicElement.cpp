#include "graphicElement.h"
#include <QPainter>
#include <QStyleOption>

#include <math.h>

/*!
 * \brief normalizeAngleRad utilitaire de normalisation d'angle en radian
 * \param angleRaw
 * \return
 */
static qreal normalizeAngleRad(qreal angleRaw)
{
    while (angleRaw < -Pi)
        angleRaw += TwoPi;
    while (angleRaw > Pi)
        angleRaw -= TwoPi;
    return angleRaw;
}

Coord::Coord(){x=0;y=0;teta=0;ortho=true;}
Coord::Coord(float _x, float _y, float _teta, bool _ortho):x(_x),y(_y),teta(_teta),ortho(_ortho){}
void Coord::init(float _x, float _y, float _teta, bool _ortho){x=_x;y=_y;teta=_teta;ortho=_ortho;}

/*!
 * \brief GraphicElement::GraphicElement constructeur TODO: rajouter le X,Y d'init
 * \param angle_init
 * \param R
 * \param G
 * \param B
 */
GraphicElement::GraphicElement(float R, float G, float B)
    : angle(0), color(R, G, B)
{
    //setRotation(qrand() % (360 * 16));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setOpacity(1.0);
    // Robot par défaut
    QPolygonF polygon;
    polygon << QPointF(10,-15) << QPointF(-10,-15)<< QPointF(-15,-10)<< QPointF(-15,10) << QPointF(-10,15);
    polygon << QPointF(10,15) << QPointF(15,10) << QPointF(10,0) << QPointF(15,-10) << QPointF(10,-15);
    setPolygon(polygon);
    setFillRule(Qt::WindingFill);
    x_reel=0.0; y_reel=0.0; angle=0.0;
    x_offset=0.0;y_offset=0.0;angle_offset=0.0;
    x_asserv_init=0.0; y_asserv_init=0.0; angle_asserv_init=0.0;
    isRelativToBot=true;
    sensOrtho=1;
    vitesse=0.0;
    isConvergenceXY=true;
    isConvergenceTeta=true;
    is_init_target=false;

}

/*!
 * \brief GraphicElement::GraphicElement constructeur TODO: rajouter le X,Y d'init
 * \param angle_init
 * \param R
 * \param G
 * \param B
 */
GraphicElement::GraphicElement(QPolygonF forme, float R, float G, float B)
    : angle(0), color(R, G, B)
{
    //setRotation(qrand() % (360 * 16));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setOpacity(1.0);
    setPolygon(forme);
    setFillRule(Qt::WindingFill);
    x_reel=0.0; y_reel=0.0; angle=0.0;
    x_offset=0.0;y_offset=0.0;angle_offset=0.0;
    x_asserv_init=0.0; y_asserv_init=0.0; angle_asserv_init=0.0;
    isRelativToBot=true;
    sensOrtho=1;

    //pour l'aaservissement interne
    vitesse=0.0;
    isConvergenceXY=true;
    isConvergenceTeta=true;
    is_init_target=false;
}

/*!
 * \brief GraphicElement::GraphicElement constructeur par défaut
 * \param angle_init
 * \param R
 * \param G
 * \param B
 */
GraphicElement::GraphicElement()
    : angle(0), color(255, 255, 125)
{
    //setRotation(qrand() % (360 * 16));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //Robot par défaut
    QPolygonF polygon;
    polygon << QPointF(10,-15) << QPointF(-10,-15)<< QPointF(-15,-10)<< QPointF(-15,10) << QPointF(-10,15);
    polygon << QPointF(10,15) << QPointF(15,10) << QPointF(10,0) << QPointF(15,-10) << QPointF(10,-15);
    setPolygon(polygon);
    setFillRule(Qt::WindingFill);
    isRelativToBot=true;
    sensOrtho=1;
    vitesse=0.0;
    isConvergenceXY=true;
    isConvergenceTeta=true;
    is_init_target=false;
}

/*!
 * \brief GraphicElement::mousePressEvent gestion du click de la souris sur l'élément, pour l'instant pas d'action spécifique
 * \param event
 */
void GraphicElement::mousePressEvent(QGraphicsSceneMouseEvent *event){
    //qDebug() << "Robot clicked.";

    //on ne fait rien

    //on rend la main au gestionnaire d'événement
    QGraphicsPolygonItem::mousePressEvent( event );
}

void GraphicElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit center(x(),y(),0.0);
    emit isDoubleClicked();
    QGraphicsPolygonItem::mouseDoubleClickEvent(event);
}

/*!
 * \brief GraphicElement::mousePressEvent gestion du click de la souris sur l'élément
 * \param event
 */
void GraphicElement::wheelEvent(QGraphicsSceneWheelEvent * event){
    //qDebug() << "Robot scrolled of" << event->delta();
    //init de la variable qui récupère le compteur de la molette
    qreal da=0;

    //détection du sens de déplacement de la molette
    if (event->delta()>0)
        da=1.0;
    else
        da=-1.0;

    //incrément ou décrément de l'angle de l'objet
    if (this->flags() & QGraphicsItem::ItemIsMovable)
        this->setRotation(rotation()+da);

    //on récupère l'angle final de rotation (normalisé) pour l'affichage
    //angle=normalizeAngleRad(angle-((da/360.0)*TwoPi));

    //on rend la main au gestionnaire d'événement
    QGraphicsPolygonItem::wheelEvent( event );
    this->update();
}

/*!
 * \brief GraphicElement::getTheta retourne l'angle de l'élément
 * \return
 */
qreal GraphicElement::getTheta(void){
    if(isRelativToBot)
        angle=TwoPi*((360-rotation())-(angle_offset - angle_asserv_init))/360;
    else
        angle=TwoPi*(360-rotation())/360;
    return normalizeAngleRad(angle);
}

/*!
 * \brief GraphicElement::getX retourne l'abcisse de l'élément
 * \return
 */
qreal GraphicElement::getX(void){
    if(isRelativToBot)
        //x_reel=sensOrtho*((x()-x_offset)*cos(TwoPi*angle_offset/360))+(((-y())-y_offset)*sin(TwoPi*angle_offset/360));
        x_reel=((x()-x_offset)*cos(TwoPi*(angle_offset-angle_asserv_init)/360))+(((-y())-y_offset)*sin(TwoPi*(angle_offset-angle_asserv_init)/360));
    else
        x_reel=x();
    return x_reel;
}

/*!
 * \brief GraphicElement::getX retourne l'abcisse de l'élément toujours relative au terrain
 * \return
 */
qreal GraphicElement::getX_terrain(void){
    return x();
}

/*!
 * \brief GraphicElement::getY retourne l'ordonnée de l'élément
 * \return
 */
qreal GraphicElement::getY(void){
    if (isRelativToBot)
        y_reel=-((x()-x_offset)*sin(TwoPi*(angle_offset-angle_asserv_init)/360))+(((-y())-y_offset)*cos(TwoPi*(angle_offset-angle_asserv_init)/360));
    else
        y_reel=-y();
    return y_reel;
}

/*!
 * \brief GraphicElement::getY retourne l'ordonnée de l'élément toujours relative au terrain
 * \return
 */
qreal GraphicElement::getY_terrain(void){
    return (-y());
}

void GraphicElement::setAsservInit(qreal x_init, qreal y_init, qreal angle_init)
{
    x_asserv_init=x_init;
    y_asserv_init=y_init;
    angle_asserv_init=angle_init;
}

void GraphicElement::setSpeed(double newVitesse)
{
    vitesse=newVitesse;
    is_init_target=false;
}

/*!
 * \brief GraphicElement::replace repositionne l'élément aux coordonnées indiquées et à la vitesse donnée de façon
 * linéaire si la vitesse interne est non nulle sinon le déplacement est instantané.
 * \param x_new
 * \param y_new
 */
void GraphicElement::replace(qreal x_new, qreal y_new){
    //si la vitesse interne est nulle alors
    //le robot se déplace instantanément
    if (vitesse==0.0)
        setPos(x_new,y_new);
    else
    {
        //la vitesse du robot est non nulle on est dans un mode
        //particulier avec un asservissement  interne bypassé par
        //un déplacement à intervalle constant
        if(!is_init_target)
        {
            moveInit(x_new,y_new);
            moveAtSpeed();
        }
    }
}

void GraphicElement::raz(qreal x_new, qreal y_new, qreal angle_new)
{
    replace(x_new,-y_new); //on replace l'élément aux coordonnées ramenées à un repère orthonormé
    setRotation(360-(angle_new)); //on tourne de angle_new ramené au repere orthonorme

    //on enregistre la position sur le terrain de simulation via un offset
    //sur ce point l'asservissement se croira en (0,0,0)
    x_offset=x_new;
    y_offset=y_new;
    angle_offset=angle_new;

    x_reel=0;y_reel=0;angle=0;
}

void GraphicElement::raz(Coord cTerrain, Coord cAsserv)
{
    //Positionnement terrain
    replace(cTerrain.x,-cTerrain.y); //on replace l'élément aux coordonnées ramenées à un repère orthonormé
    setRotation(360-cTerrain.teta); //on tourne de angle_new ramené au sens trigo

    //positionnement physique - on donne les offset du placement terrain
    x_offset=cTerrain.x;
    y_offset=cTerrain.y;
    angle_offset=cTerrain.teta;

    x_asserv_init=cAsserv.x;
    y_asserv_init=cAsserv.y;
    angle_asserv_init=cAsserv.teta;

    x_reel=0;y_reel=0;angle=0;
}

void GraphicElement::display_XY(qreal x_reel_new, qreal y_reel_new)
{
    qreal x_view=x();
    qreal y_view=y();

    qreal x_target=x_reel_new;
    qreal y_target=y_reel_new;


    //hors range pour Y, on affiche que X en gardant Y
    if(y_reel_new>=5000)
    {
        y_target=getY();

    }
    //hors range pour X, on affiche que Y en gardant X
    if(x_reel_new>=5000)
    {
        x_target=getX();
    }

    //qDebug() << "consigne:" << x_target <<","<<y_target;
    //qDebug() << "angle" << angle_offset;
    if(isRelativToBot){
        x_view=x_target*cos(-TwoPi*(angle_offset-angle_asserv_init)/360)+y_target*sin(-TwoPi*(angle_offset-angle_asserv_init)/360)+x_offset;
        y_view=x_target*sin(-TwoPi*(angle_offset-angle_asserv_init)/360)-y_target*cos(-TwoPi*(angle_offset-angle_asserv_init)/360)-y_offset;
    }
    else{
        x_view=x_target;
        y_view=-y_target;
    }

    replace(x_view,y_view);

}

void GraphicElement::display_theta(qreal theta_reel_new)
{
    if(isRelativToBot)
        this->setRotation(-(180*(theta_reel_new)/Pi)-(angle_offset-angle_asserv_init));
    else
        this->setRotation(-(180*(theta_reel_new)/Pi));
}

void GraphicElement::moveAtSpeed(void)
{
    if(is_init_target)
    {
        count_step_target++;

        if(count_step_target<nb_step_target)
        {
            double dX=count_step_target*dX_target;
            double dY=count_step_target*dY_target;
            //qDebug()<<"Next step to move ("<<x_internal_hold+dX<<","<<(m_target*(x_internal_hold*dX)+p_target)<<")";
            if(fabs(x_internal_target-x_internal_hold)<0.01) //x constant
                setPos(x_internal_target,y_internal_hold+dY);
            else if(fabs(y_internal_target-y_internal_hold)<0.01) //y constant
                setPos(x_internal_hold+dX,y_internal_target);
            else
                setPos(x_internal_hold+dX,m_target*(x_internal_hold+dX)+p_target);
        }
        else
        {
            if(!isConvergenceXY)
            {
                isConvergenceXY=true; //on indique qu'on est arrivé
                is_init_target=false; //on désactive le déplacement à vitesse constante (on est arrivé au point voulu)
                //qDebug()<<"Arrived at ("<<x_internal_target<<","<<y_internal_target<<")\n\n";
                setPos(x_internal_target,y_internal_target);
            }
        }
    }
}

void GraphicElement::moveInit(float toX, float toY)
{
    setTargetXY(toX,toY);

    //Initialisation et calcul des éléments du déplacement à vitesse constante
    dY_target=0.0;
    dX_target=0.0;
    count_step_target=0;
    //distance à parcourir
    double distance_to_move=sqrt(pow((x_internal_target-x_internal_hold),2)+pow((y_internal_target-y_internal_hold),2));
    //nombre de pas de déplacement et distance de déplacement à chaque pas
    nb_step_target=(distance_to_move/vitesse)/0.025;

    //trajectoire rectiligine
    //y=m*x+p
    if(fabs(x_internal_target-x_internal_hold)<0.01) //x constant
    {
        m_target=0.0;
        p_target=0.0;
        dY_target=(y_internal_target-y_internal_hold)/nb_step_target;
    }
    else if(fabs(y_internal_target-y_internal_hold)<0.01) //y constant
    {
        m_target=0.0;
        p_target=y_internal_hold;
        dX_target=(x_internal_target-x_internal_hold)/nb_step_target;
    }
    else
    {
        m_target=(y_internal_target-y_internal_hold)/(x_internal_target-x_internal_hold);
        p_target=(y_internal_target-m_target*x_internal_target);
        dX_target=(x_internal_target-x_internal_hold)/nb_step_target;
    }

/*
    qDebug() << "New target ("<<x_internal_target<<","<<y_internal_target<<") from ("<<x_internal_hold<<","<<y_internal_hold<<")";
    qDebug() << "Internal parameters:";
    qDebug() << "Speed = "<<vitesse<<"cm/s";
    if((x_internal_target-x_internal_hold)<0.01) //x constant
        qDebug() << "X constant at ("<< x_internal_target-x_internal_hold <<") => dY = "<<dY_target<<" cm every 25ms";
    else if((y_internal_target-y_internal_hold)<0.01) //y constant
        qDebug() << "Y constant at ("<< y_internal_target-y_internal_hold <<") => dX = "<<dX_target<<" cm every 25ms";
    else
    {
        qDebug() << "dX = "<<dX_target<<" cm every 25ms";
        qDebug() << "dY = "<<m_target<<"*dX+"<<p_target;
    }
    qDebug() << "Time of moving estimated at "<<nb_step_target*0.025<<" secondes";
*/
}

double GraphicElement::getErrorDistance(double x_target, double y_target)
{
    double x_view=this->getX();
    double y_view=this->getY();

    return sqrt(pow((x_target-x_view),2)+pow((y_target-y_view),2));
}
double GraphicElement::getErrorAngle(double x_target, double y_target)
{
    double x_view=this->getX();
    double y_view=this->getY();
    double teta_view=this->getTheta();
    double deltaAngle=0.0;

    //on calcule de l'angle par rapport à la consigne de position
    if (x_view==x_target)
    {
        if (y_target>y_view)
            deltaAngle=Pi/2;
        else if (y_target==y_view)
            deltaAngle=0;
        else if (y_target<y_view)
            deltaAngle=-Pi/2;
    }
    else if (x_target>x_view)
        deltaAngle=atan((y_target-y_view)/(x_target-x_view));
    else if (x_target<x_view)
    {
        if (y_target>y_view)
            deltaAngle=atan((y_target-y_view)/(x_target-x_view))+Pi;
        else if (y_target==y_view)
            deltaAngle=Pi;
        else if (y_target<y_view)
            deltaAngle=atan((y_target-y_view)/(x_target-x_view))-Pi;
    }

    //on retourne l'erreur d'angle compensée par l'angle actuel du robot
    return (deltaAngle-teta_view);
}

void GraphicElement::setTargetXY(float toX,float toY)
{
    /*x_internal_hold=this->getX();
    y_internal_hold=this->getY();*/
    x_internal_hold=x();
    y_internal_hold=y();
    x_internal_target=toX;
    y_internal_target=toY;
    qDebug() << "[CSimuBot] New target: ("<< toX << "," << toY <<") from ("<< x_internal_hold<<","<<y_internal_hold<<")";

    isConvergenceXY=false;
    isConvergenceTeta=false;
    is_init_target=true;
}
void GraphicElement::setTargetTeta(float toTeta)
{
    teta_internal_hold=this->getX();
    teta_internal_target=toTeta;


    isConvergenceXY=false;
    isConvergenceTeta=false;
    is_init_target=true;
}

void GraphicElement::stepToTarget(float * forceG, float * forceD)
{
    //on a demandé un déplacement avec asservissement
    if(is_init_target)
    {
        //La convergence XY n'est pas encore atteinte
        if(!isConvergenceXY)
        {
            float posError=getErrorDistance(x_internal_target,y_internal_target);
            //TODO : le retour d'erreur doit être signé
            *forceG=4.0*posError;
            *forceD=4.0*posError;
            qDebug()<<"Consigne roue gauche et droite ("<<*forceG<<","<<*forceD<<")\n\n";
        }
    }
}


