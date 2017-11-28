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
        angle=TwoPi*(((360-rotation())-angle_offset)+angle_asserv_init)/360;
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
        x_reel=((x()-x_offset)*cos(TwoPi*(angle_offset+angle_asserv_init)/360))+(((-y())-y_offset)*sin(TwoPi*(angle_offset+angle_asserv_init)/360));
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
        y_reel=-((x()-x_offset)*sin(TwoPi*(angle_offset+angle_asserv_init)/360))+(((-y())-y_offset)*cos(TwoPi*(angle_offset+angle_asserv_init)/360));
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

/*!
 * \brief GraphicElement::replace repositionne l'élément aux coordonnées indiquées et à la vitesse donnée de façon
 * linéaire. si la vitesse est nulle alors le déplacement est instantané.
 * \param x_new
 * \param y_new
 * \param speed
 */
void GraphicElement::replace(qreal x_new, qreal y_new, float speed){

    if (speed==0.0)
        setPos(x_new,y_new);
    else
    {
        qreal x_sauv=x();
        qreal y_sauv=y();
//        qreal x_temp=x_sauv;
//        qreal y_temp=y_sauv;
        //y=m*x+p
        double m=((y_new-y_sauv)/(x_new-x_sauv));
        double p=(y_new-m*x_new);
        double distance_restante=sqrt(pow((x_new-x_sauv),2)+pow((y_new-y_sauv),2));
        //double nb_pas=(distance_restante/speed)/0.025;
        double nb_pas=(distance_restante/speed)/0.025;
        double dX=(x_new-x_sauv)/nb_pas;
        for (int i=1;i<nb_pas;i++)
        {
            setPos(x_sauv+i*dX,(m*(x_sauv+i*dX)+p));
            QTest::qWait(25);
        }
        setPos(x_new,y_new);
    }
}

void GraphicElement::raz(qreal x_new, qreal y_new, qreal angle_new)
{
    replace(x_new,-y_new,0.0); //on replace l'élément aux coordonnées ramenées à un repère orthonormé
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
    replace(cTerrain.x,-cTerrain.y,0.0); //on replace l'élément aux coordonnées ramenées à un repère orthonormé
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
        x_view=x_target*cos(-TwoPi*(angle_offset+angle_asserv_init)/360)+y_target*sin(-TwoPi*(angle_offset+angle_asserv_init)/360)+x_offset;
        y_view=x_target*sin(-TwoPi*(angle_offset+angle_asserv_init)/360)-y_target*cos(-TwoPi*(angle_offset+angle_asserv_init)/360)-y_offset;
    }
    else{
        x_view=x_target;
        y_view=-y_target;
    }

    replace(x_view,y_view,0.0);

}

void GraphicElement::display_theta(qreal theta_reel_new)
{
    if(isRelativToBot)
        this->setRotation(-(180*(theta_reel_new)/Pi)-angle_offset+angle_asserv_init);
    else
        this->setRotation(-(180*(theta_reel_new)/Pi));
}



