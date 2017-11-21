#include "graphicEnvironnement.h"

/*!
 * \brief Environnement::Environnement
 */
GraphicEnvironnement::GraphicEnvironnement()
{
    //On donne au terrain les dimensions Eurobot
    //(augmentées des épaisseurs de bordure elles-mêmes ramenées à 2cm, on se fout des 2mm ext ;-) )
    //TODO: prendre en compte les supports balise
    this->setSceneRect(0, -200, 300, 200);

    //Pour l'instant il n'y a pas assez d'objets pour avoir besoin de les indexer (plus rapide)
    this->setItemIndexMethod(QGraphicsScene::NoIndex);
}

/*!
 * \brief fromDimensionReelle
 * \param dim
 * \return
 */
float GraphicEnvironnement::fromDimensionReelle(float dim)
{
    return 2*dim;
}

/*!
 * \brief Environnement::toDimensionReelle
 * \param dim
 * \return
 */
float GraphicEnvironnement::toDimensionReelle(float dim)
{
    return 0.5*dim;
}

/*!
 * \brief Environnement::fromCoordonneesReelles
 * \param coord_reelles
 * \param angle_reel
 * \return
 */
Coordonnees GraphicEnvironnement::fromCoordonneesReelles(QPointF coord_reelles, float angle_reel)
{
    Coordonnees coord_new;
    coord_new.X=(coord_reelles.x()-150);
    coord_new.Y=(-1.0*coord_reelles.y()-100);
    coord_new.theta=angle_reel;

    return coord_new;
}

/*!
 * \brief Environnement::toCoordonneesReelles
 * \param coord_view
 * \param angle_view
 * \return
 */
Coordonnees GraphicEnvironnement::toCoordonneesReelles(QPointF coord_view, float angle_view)
{
    Coordonnees coord_new;
    coord_new.X=(coord_view.x()+150);
    coord_new.Y=(-1.0*coord_view.y()+100);
    coord_new.theta=angle_view;

    return coord_new;
}
