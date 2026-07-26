/*! \file CRoseDesVents.h
 * \brief Rose des vents : indicateur de cap du robot superpose a la scene SimuBot.
 *
 * ETAPE 7 de la migration SimuBot v2. Remplace l'ancien "nez" du polygone robot, retire a
 * l'etape 5 (la forme du robot est desormais un octogone simple decrit dans Config/robot.json).
 *
 * # CONVENTION D'ANGLE (un seul point de conversion, ici)
 * L'item recoit des caps exprimes en DEGRES TRIGONOMETRIQUES : axe x vers la droite = 0 degre,
 * sens anti-horaire, y vers le HAUT. Ce n'est PAS la convention de la scene Qt (dont l'axe y
 * pointe vers le BAS) : la negation vers le repere ecran est faite exclusivement dans paint(),
 * et l'item ne subit JAMAIS de setRotation().
 *
 * Quel repere ces degres expriment-ils ? C'est le choix de l'APPELANT, la rose n'affiche qu'un
 * angle. CSimuBot lui fournit le cap du repere ASSERVISSEMENT (GrosBot->getTheta()), car SimuBot
 * sert a construire une strategie et que le code generé par BlockBotLab raisonne dans ce repere :
 * le 0 de la rose est donc le 0 de l'asserv, pas celui du terrain. Cf. CSimuBot::refreshRoseDesVents.
 *
 * # ORIENTATION DU CADRAN (setOrientationCadranDeg) -- indispensable, pas cosmetique
 * Afficher des VALEURS asserv sur un cadran dont le 0 serait cloue a la droite de l'ecran serait
 * FAUX des que le repere asserv n'est pas aligne sur le terrain. Le cadran (graduations, reperes
 * numeriques ET aiguilles) est donc pivote de l'ecart entre les deux reperes, c'est-a-dire de la
 * direction TERRAIN de l'axe 0 de l'asserv. Exemple reel (Coupe 2026, pose d'init identique
 * -1,57 rad en terrain pour les deux equipes) :
 *   - equipe jaune : asserv_init = -1,57 rad -> ecart = -90 - (-90) =    0 deg -> 0 de la rose a droite
 *   - equipe bleue : asserv_init = +1,57 rad -> ecart = -90 - (+90) = -180 deg -> 0 de la rose a gauche
 * Les glyphes des reperes numeriques restent TOUJOURS a l'endroit : seule leur POSITION tourne
 * (sinon a -180 deg les nombres seraient a l'envers).
 *
 * # ECHELLE
 * L'item est dimensionne en UNITES SCENE (cm terrain), volontairement SANS le flag
 * ItemIgnoresTransformations : il est donc rendu sous la transformation de la vue et grossit
 * exactement avec le zoom SimuBot (CSimuBot::zoom -> scale()). Comme aucun fitInView n'existe
 * dans SimuBot, la transformation de la vue ne depend que du slider de zoom : redimensionner
 * la fenetre ne change pas la taille de la rose, seulement la surface visible.
 *
 * # POSITIONNEMENT
 * Le placement (ancrage naturel au-dessus du terrain + repli dans le viewport) n'est PAS gere
 * ici mais par CSimuBot::refreshRoseDesVents(), qui seul connait la vue et son etat de zoom /
 * defilement. L'item se contente d'exposer son rayon.
 */

#ifndef CROSEDESVENTS_H
#define CROSEDESVENTS_H

#include <QGraphicsItem>
#include <QColor>

class CRoseDesVents : public QGraphicsItem
{
public:
    //! \param rayon_scene rayon du disque en unites scene (cm terrain)
    explicit CRoseDesVents(qreal rayon_scene = 22.0);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    //! Cap courant du robot, en degres trigo (repere fixe par l appelant). Ne declenche un update() que si la valeur
    //! change reellement : sans ce garde-fou, chaque repaint reemettrait QGraphicsScene::changed,
    //! donc un tour supplementaire de CSimuBot::viewChanged a chaque tick, robot immobile compris.
    void setCapCourantDeg(qreal cap_deg);

    //! Cap de depart du robot (pose d init), en degres trigo (repere fixe par l appelant). Meme garde-fou.
    void setCapInitDeg(qreal cap_deg);

    //! Direction TERRAIN (degres trigo) de l'axe 0 du repere dans lequel sont exprimes les caps.
    //! Fait pivoter tout le cadran pour qu'il soit lisible dans le terrain (cf. en-tete du .h).
    //! Vaut 0 si les caps fournis sont deja des caps terrain. Meme garde-fou que les caps.
    void setOrientationCadranDeg(qreal orientation_deg);

    //! Rayon du disque en unites scene : necessaire a CSimuBot pour calculer l'ancrage et le repli.
    qreal rayon(void) const { return m_rayon; }

private:
    //! Ramene un angle en degres sur (-180, 180], comme le normalizeAngleDeg de CSimuBot.
    static qreal normaliseDeg(qreal angle_deg);

    //! Cap du cadran -> angle ecran en radians. Applique l'orientation du cadran PUIS la negation
    //! trigo -> scene Qt (y vers le bas). UNIQUE point de conversion de la classe.
    qreal angleEcranRad(qreal cap_deg) const;

    //! Trace une aiguille depuis le centre vers le cap donne (degres trigo).
    void dessineAiguille(QPainter *painter, qreal cap_deg, qreal longueur,
                         qreal demi_largeur, const QColor& couleur) const;

    qreal m_rayon;              //!< rayon du disque, unites scene
    qreal m_cap_courant_deg;    //!< cap courant, degres trigo (repere fixe par l appelant)
    qreal m_cap_init_deg;       //!< cap de depart, degres trigo (repere fixe par l appelant)
    qreal m_orientation_cadran_deg; //!< direction terrain de l'axe 0 du repere des caps
};

#endif // CROSEDESVENTS_H
