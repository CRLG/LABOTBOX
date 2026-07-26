/*! \file CRoseDesVents.cpp
 * \brief Implementation de la rose des vents (etape 7, migration SimuBot v2).
 */

#include "CRoseDesVents.h"

#include <QPainter>
#include <QFont>
#include <QtMath>

//! Seuil de variation en dessous duquel on ne redessine pas : evite de reemettre
//! QGraphicsScene::changed en boucle (cf. commentaire des setters dans le .h).
static const qreal SEUIL_REDESSIN_DEG = 0.1;

//! Couleurs de la rose. L'aiguille d'init reste volontairement tres discrete : c'est une
//! reference, pas l'information principale.
static const QColor COULEUR_FOND        = QColor(255, 255, 255, 170);
static const QColor COULEUR_CONTOUR     = QColor( 70,  70,  70, 200);
static const QColor COULEUR_GRADUATION  = QColor( 90,  90,  90, 200);
static const QColor COULEUR_AIG_INIT    = QColor(150, 150, 150, 200);
static const QColor COULEUR_AIG_COURANT = QColor(200,  40,  40, 230);
static const QColor COULEUR_TEXTE       = QColor( 40,  40,  40, 230);

/*!
 * \brief CRoseDesVents::CRoseDesVents
 * \param rayon_scene rayon du disque en unites scene (cm terrain)
 */
CRoseDesVents::CRoseDesVents(qreal rayon_scene)
    : m_rayon(rayon_scene)
    , m_cap_courant_deg(0.0)
    , m_cap_init_deg(0.0)
    , m_orientation_cadran_deg(0.0)
{
    // Au-dessus du terrain, du robot et des elements de jeu : la rose est un indicateur, elle
    // ne doit jamais passer derriere une caisse lorsqu'elle se replie sur le terrain.
    setZValue(100);
}

/*!
 * \brief CRoseDesVents::normaliseDeg ramene un angle sur (-180, 180]
 */
qreal CRoseDesVents::normaliseDeg(qreal angle_deg)
{
    while (angle_deg < -180.0) angle_deg += 360.0;
    while (angle_deg >  180.0) angle_deg -= 360.0;
    return angle_deg;
}

/*!
 * \brief CRoseDesVents::boundingRect
 *
 * Carre centre sur l'origine de l'item : la position de l'item EST le centre du disque, ce dont
 * CSimuBot::refreshRoseDesVents se sert pour son calcul d'ancrage et de repli. La marge couvre
 * l'epaisseur du contour et la ligne de texte tracee sous le disque.
 */
QRectF CRoseDesVents::boundingRect() const
{
    const qreal marge = m_rayon * 0.35;
    return QRectF(-m_rayon - marge, -m_rayon - marge,
                  2.0 * (m_rayon + marge), 2.0 * (m_rayon + marge));
}

/*!
 * \brief CRoseDesVents::setCapCourantDeg
 * \param cap_deg cap courant en degres trigo (repere fixe par l appelant)
 */
void CRoseDesVents::setCapCourantDeg(qreal cap_deg)
{
    const qreal cap = normaliseDeg(cap_deg);
    if (qAbs(cap - m_cap_courant_deg) < SEUIL_REDESSIN_DEG) return;
    m_cap_courant_deg = cap;
    update();
}

/*!
 * \brief CRoseDesVents::setCapInitDeg
 * \param cap_deg cap de depart en degres trigo (repere fixe par l appelant)
 */
void CRoseDesVents::setCapInitDeg(qreal cap_deg)
{
    const qreal cap = normaliseDeg(cap_deg);
    if (qAbs(cap - m_cap_init_deg) < SEUIL_REDESSIN_DEG) return;
    m_cap_init_deg = cap;
    update();
}

/*!
 * \brief CRoseDesVents::setOrientationCadranDeg
 * \param orientation_deg direction terrain (degres trigo) de l'axe 0 du repere des caps
 */
void CRoseDesVents::setOrientationCadranDeg(qreal orientation_deg)
{
    const qreal orientation = normaliseDeg(orientation_deg);
    if (qAbs(orientation - m_orientation_cadran_deg) < SEUIL_REDESSIN_DEG) return;
    m_orientation_cadran_deg = orientation;
    update();
}

/*!
 * \brief CRoseDesVents::angleEcranRad convertit un cap du cadran en angle ecran
 *
 * Deux transformations, dans cet ordre :
 *  1. orientation du cadran : le cap est exprime dans le repere des caps (asserv en general), on
 *     le ramene en repere terrain en ajoutant la direction terrain de l'axe 0 de ce repere ;
 *  2. repere trigo (y vers le haut) -> repere scene Qt (y vers le bas) : negation de l'angle.
 * C'est l'UNIQUE endroit ou ces conversions sont faites pour la rose.
 */
qreal CRoseDesVents::angleEcranRad(qreal cap_deg) const
{
    return qDegreesToRadians(-(m_orientation_cadran_deg + cap_deg));
}

/*!
 * \brief CRoseDesVents::dessineAiguille trace une aiguille triangulaire du centre vers le cap
 * \param cap_deg cap en degres trigo (la conversion vers le repere ecran est faite ici)
 * \param longueur longueur de l'aiguille, unites scene
 * \param demi_largeur demi-largeur de la base du triangle, unites scene
 * \param couleur couleur de remplissage
 */
void CRoseDesVents::dessineAiguille(QPainter *painter, qreal cap_deg, qreal longueur,
                                    qreal demi_largeur, const QColor& couleur) const
{
    const qreal a = angleEcranRad(cap_deg);
    const qreal ca = qCos(a);
    const qreal sa = qSin(a);

    // Pointe a l'avant, base de part et d'autre du centre (perpendiculaire au cap).
    const QPointF pointe(longueur * ca, longueur * sa);
    const QPointF baseG(-demi_largeur * sa, demi_largeur * ca);
    const QPointF baseD( demi_largeur * sa, -demi_largeur * ca);

    QPolygonF aiguille;
    aiguille << pointe << baseG << baseD;

    painter->setPen(Qt::NoPen);
    painter->setBrush(couleur);
    painter->drawPolygon(aiguille);
}

/*!
 * \brief CRoseDesVents::paint
 *
 * Tout est trace en unites scene autour de l'origine de l'item (= centre du disque). Aucune
 * rotation n'est appliquee a l'item lui-meme : seules les aiguilles tournent.
 */
void CRoseDesVents::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing, true);

    // --- disque de fond ---
    painter->setPen(QPen(COULEUR_CONTOUR, m_rayon * 0.035));
    painter->setBrush(COULEUR_FOND);
    painter->drawEllipse(QPointF(0.0, 0.0), m_rayon, m_rayon);

    // --- graduations tous les 30 degres, plus longues sur les 4 axes cardinaux ---
    for (int deg = 0; deg < 360; deg += 30)
    {
        const bool cardinal = ((deg % 90) == 0);
        const qreal r_int = cardinal ? m_rayon * 0.78 : m_rayon * 0.87;
        const qreal a = angleEcranRad(static_cast<qreal>(deg));
        const qreal ca = qCos(a);
        const qreal sa = qSin(a);

        painter->setPen(QPen(COULEUR_GRADUATION, m_rayon * (cardinal ? 0.045 : 0.025)));
        painter->drawLine(QPointF(r_int * ca, r_int * sa),
                          QPointF(m_rayon * ca, m_rayon * sa));
    }

    // --- reperes numeriques des axes cardinaux du repere des caps ---
    // Leur POSITION tourne avec le cadran, mais les glyphes restent a l'endroit (aucune rotation
    // n'est appliquee au painter) : a -180 deg d'orientation les nombres seraient sinon inverses.
    QFont police = painter->font();
    police.setPointSizeF(m_rayon * 0.17);   // dimensionne en unites scene : suit le zoom
    police.setBold(true);
    painter->setFont(police);
    painter->setPen(COULEUR_TEXTE);

    const QString degre = QString(QChar(0x00B0)); // signe degre, sans caractere non ASCII en dur
    // Les reperes cardinaux sont volontairement SANS le signe degre : a zoom 1 le disque ne fait
    // que 44 px, "180" deborde deja sur les graduations si on ajoute un glyphe.
    const qreal r_txt = m_rayon * 0.55;
    const qreal demi_boite = m_rayon * 0.28;
    for (int deg = 0; deg < 360; deg += 90)
    {
        const qreal a = angleEcranRad(static_cast<qreal>(deg));
        const QPointF centre_txt(r_txt * qCos(a), r_txt * qSin(a));
        const QRectF boite(centre_txt.x() - demi_boite, centre_txt.y() - demi_boite,
                           2.0 * demi_boite, 2.0 * demi_boite);
        painter->drawText(boite, Qt::AlignCenter,
                          QString::number(static_cast<int>(normaliseDeg(deg))));
    }

    // --- aiguille du cap de depart : discrete, sert de reference fixe ---
    dessineAiguille(painter, m_cap_init_deg, m_rayon * 0.92, m_rayon * 0.10, COULEUR_AIG_INIT);

    // --- aiguille du cap courant : par dessus, plus visible ---
    dessineAiguille(painter, m_cap_courant_deg, m_rayon * 0.80, m_rayon * 0.15, COULEUR_AIG_COURANT);

    // --- moyeu ---
    painter->setPen(Qt::NoPen);
    painter->setBrush(COULEUR_CONTOUR);
    painter->drawEllipse(QPointF(0.0, 0.0), m_rayon * 0.07, m_rayon * 0.07);

    // --- valeur numerique du cap courant, sous le disque ---
    police.setPointSizeF(m_rayon * 0.24);
    painter->setFont(police);
    painter->setPen(COULEUR_AIG_COURANT);
    const QRectF boite_valeur(-m_rayon, m_rayon * 0.98, 2.0 * m_rayon, m_rayon * 0.36);
    painter->drawText(boite_valeur, Qt::AlignCenter,
                      QString::number(m_cap_courant_deg, 'f', 1) + degre);
}
