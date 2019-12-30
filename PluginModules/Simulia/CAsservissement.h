/*! \file CAsservissement.h
    \brief Classe qui contient l'asservissement vitesse/position du robot
*/

#ifndef _ASSERVISSEMENT_H_
#define _ASSERVISSEMENT_H_


// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CAsservissement
{
public :
    //! Constructeur / destructeur
    CAsservissement();
    ~CAsservissement();

    //! Réinitilise tous les paramètres et valeurs
    void Init(void);

    //! Bornage de l'angle pour les mouvement distance/angle
    float BornageAngle(float angle);

    // Prototype des fonctions
    void CommandeMouvementXY(float x, float y);
    void CommandeMouvementDistanceAngle(float distance, float angle);
    void CommandeManuelle(float cde_G, float cde_D);
    void CommandeMouvementXY_A(float x, float y);
    void CommandeMouvementXY_B(float x, float y);
    void CommandeMouvementXY_TETA(float x, float y, float teta);
    void CommandeVitesseMouvement(float vit_avance, float vit_angle);
    void CalculsMouvementsRobots(void);
    void setPosition_XYTeta(float x, float y, float teta);
    void setIndiceSportivite(float idx);
    void setCdeMinCdeMax(int min, int max);
};


#endif


