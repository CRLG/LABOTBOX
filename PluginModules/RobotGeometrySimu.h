/*! \file RobotGeometrySimu.h
 * \brief SOURCE UNIQUE des constantes mecaniques qui ferment la boucle Software-In-the-Loop.
 *
 * Etape 8quater de la migration SimuBot v2 (cf. rapport_simubot.md section 8quater).
 *
 * # POURQUOI CE FICHIER EXISTE
 * Trois modules utilisent l'entraxe et les pas codeurs, et ils sont les DEUX COTES de la meme
 * boucle : l'un EMET les pas codeurs, les autres les RELISENT pour reconstruire une position.
 *   - CKinematicEngine (PluginModules/SimuBot)  : emet   steps = distance / DISTANCE_PAR_PAS_CODEUR
 *   - CAsservissement_simu (PluginModules/Simulia, build Simulia) : relit distance = steps * k
 *   - CPoseReconstructeurStandalone (PluginModules/SimuBot, build LaBotBox) : idem, copie autonome
 * Si deux de ces jeux divergent, l'asserv percoit une distance ET une vitesse faussees du rapport
 * des deux k, et le robot part en vrille. C'EST DEJA ARRIVE (facteur ~20,4). Les valeurs etaient
 * jusqu'ici recopiees a la main dans les trois fichiers ; elles sont desormais definies ICI et
 * NULLE PART AILLEURS. Modifier la geometrie = modifier ce fichier, point.
 *
 * # PIEGE MAJEUR : CE NE SONT PAS LES VALEURS DU FIRMWARE, ET C'EST VOULU
 * Le firmware embarque (Soft_STM32/CM7/Sources/CAsservissement.cpp, 27/04/2025) utilise
 * 0.003189567 / 30.7. La boucle SIL n'utilise PAS le firmware : elle se ferme via
 * CAsservissement_simu, reste sur les valeurs GROSBOT historiques ci-dessous, et les deux cotes
 * doivent matcher ENTRE EUX, independamment du firmware. Une tentative d'alignement sur le
 * firmware a deja casse la boucle d'un facteur ~20,4. Ne PAS "corriger" ces valeurs pour les faire
 * coller au firmware : l'ecart firmware/simu est un sujet distinct, hors perimetre, qui devrait
 * etre traite des DEUX cotes a la fois et revalide par un deplacement droit + une rotation.
 *
 * # PIEGE CONNEXE A NE PAS CONFONDRE
 * Le firmware tourne avec te = 0.005 s, mais CAsservissement_simu garde te = 0.02 s (defaut de la
 * classe de base, non surcharge) et CSimulia ticke l'asserv toutes les 20 ms. Le step(0.02f, ...)
 * code en dur dans CSimuBot est donc CORRECT (dt = te = cadence). Ne pas l'"aligner" sur 0.005.
 *
 * # CRITERE DE NON-REGRESSION
 * Apres toute modification ici : un deplacement en ligne droite et une rotation sur place doivent
 * rester bijectifs (la pose reconstruite par l'asserv doit suivre la pose du moteur cinematique).
 */

#ifndef ROBOTGEOMETRYSIMU_H
#define ROBOTGEOMETRYSIMU_H

namespace RobotGeometrySimu
{
    //! Distance parcourue par la roue gauche pour un pas codeur [cm].
    //! Ecrit 0.00325568*20 (et non 0.0651136) pour conserver la trace de l'ancien couplage
    //! GROSBOT : le *20 est historique, il n'a pas de signification physique isolee.
    constexpr float DISTANCE_PAR_PAS_CODEUR_G = 0.00325568f*20;

    //! Distance parcourue par la roue droite pour un pas codeur [cm].
    constexpr float DISTANCE_PAR_PAS_CODEUR_D = 0.00325568f*20;

    //! Entraxe des deux roues motrices [cm].
    constexpr float VOIE_ROBOT = 31.6867261f;
}

#endif // ROBOTGEOMETRYSIMU_H
