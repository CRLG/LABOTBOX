/*! \file CCollisionEngine.h
    \brief Moteur de collisions geometriques maison (etape 3 migration SimuBot v2).

    Remplace la detection de collision de Box2D (supprime a l'etape 3). Tout est calcule
    en COORDONNEES TERRAIN (cm), jamais via QGraphicsItem::collidingItems() : cela
    reintroduirait le repere Qt-Scene comme source de verite (probleme des "4 referentiels
    qui derivent", cf. rapport_simubot.md section 3). La scene Qt ne fait que rendre ;
    la collision de la boucle de simulation vit ici.

    Deux familles d'obstacles :
      - Bordures du terrain : 4 demi-plans axis-aligned (traitement exact polygone vs demi-plan).
      - Elements de jeu     : polygones convexes (les caisses 15x5 / 5x15 sont des rectangles),
                              testes contre le robot par l'algorithme SAT (Separating Axis
                              Theorem) convexe-convexe. Le robot etant un polygone (octogone)
                              tourne de teta, le SAT gere correctement le contact "coin".

    Chaque obstacle est stocke par son CENTROIDE : sommets locaux relatifs au centroide,
    pose courante = (x, y) = centroide terrain + teta = rotation autour du centroide.
    Ce choix permet de faire TOURNER un element pousse autour de son propre centre (effet
    "bras de levier"), pas autour de l'origine du terrain.

    Resolution (resolve) :
      - Element FIXE (mobile=false) ou bordure : le robot est repousse (MTV applique au robot).
      - Element POUSSABLE (mobile=true) : l'element est deplace par le MTV ET tourne d'un angle
        proportionnel au bras de levier de la poussee (modele cinematique, pas une vraie
        dynamique masse/inertie, cf. compromis assume rapport_simubot.md section 8ter).
      - Element <-> element : passe de separation supplementaire (SAT, translation seule) pour
        eviter que les caisses poussees ne se superposent (effet de chaine).

    Sortie pour l'affichage : getObstacleDisplacement(id, dx, dy, dteta) donne le deplacement
    (translation du centroide + rotation) cumule depuis la position initiale.

    Aucune dependance Qt : geometrie pure (std::vector + math.h), testable offline.
*/

#ifndef CCOLLISIONENGINE_H
#define CCOLLISIONENGINE_H

#include <vector>

class CCollisionEngine
{
public:
    // Point / vecteur 2D en repere terrain (cm). Struct minimaliste pour rester Qt-free.
    struct Vec2 { float x; float y; };

    CCollisionEngine();

    // Bornes du terrain (cm). Demi-plans : le robot doit rester dans [x_min,x_max]x[y_min,y_max].
    void setTerrain(float x_min, float y_min, float x_max, float y_max);

    // Forme convexe du robot en repere LOCAL (cm), centree sur le point de rotation, front
    // vers +x. Reglee une seule fois (octogone GrosBotForme). Les aretes degenerees
    // (sommet duplique) sont ignorees a la construction des axes.
    void setRobotShape(const std::vector<Vec2>& local_vertices);

    // Ajoute un rectangle axis-aligned (caisse de noisette) en repere terrain.
    // x_min/y_min/x_max/y_max = coins terrain ; id = identifiant retourne par resolve().
    // mobile : true => element POUSSABLE (deplace + tourne sous la poussee, le robot continue) ;
    //          false => element FIXE (traite comme une bordure : le robot est repousse).
    // Renvoie l'index interne de l'obstacle.
    int addObstacleRect(float x_min, float y_min, float x_max, float y_max, int id, bool mobile = false);

    // Ajoute un polygone convexe quelconque en repere terrain (sens trigonometrique).
    int addObstacleConvex(const std::vector<Vec2>& vertices_terrain, int id, bool mobile = false);

    // --- Crochet annees futures : elements de jeu ronds (cf. discussion 2026-05-21) ---
    // Le moteur est un SAT convexe ; un cercle/ellipse etant convexe, on l'APPROXIME par un
    // polygone regulier a n cotes (12-24 suffisent visuellement). Aucune autre modif du moteur :
    // collision, poussee, rotation, separation et clamp fonctionnent tels quels. Augmenter n
    // alourdit le SAT (plus d'axes) -> garder n raisonnable. Cercle = ellipse avec rx==ry.
    int addObstacleEllipse(float cx, float cy, float rx, float ry, int id, bool mobile = false, int n = 16);
    int addObstacleCircle(float cx, float cy, float r, int id, bool mobile = false, int n = 16);

    // Vide la liste des obstacles (les bordures restent).
    void clearObstacles();

    // Remet tous les elements a leur position initiale (deplacement et rotation nuls). A
    // appeler sur un "raz" (re-init de la simulation) pour que les caisses reviennent en place.
    void resetObstaclePoses();

    // Active / desactive entierement la collision (mode VISU : traverser les obstacles).
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // Deplacement cumule d'un element mobile depuis sa position initiale, identifie par son id :
    // dx, dy = translation du centroide (terrain, cm) ; dteta = rotation (rad, sens trigo).
    // Renvoie false si l'id est inconnu. Sert a l'affichage (SimuBot translate + tourne le sprite).
    bool getObstacleDisplacement(int id, float& dx, float& dy, float& dteta) const;

    // Coeur : a partir d'une pose candidate du robot (x,y en terrain, teta en rad trigo),
    // repousse le robot hors des bordures/elements fixes et pousse/tourne les elements mobiles
    // (et les separe entre eux). Modifie x et y en place (le robot ne subit que des translations,
    // teta robot non contraint). Renvoie true si un contact a eu lieu ; *hitId (optionnel) recoit
    // l'id du dernier element touche par le robot (-1 = bordure uniquement, ou aucun).
    bool resolve(float& x, float& y, float teta, int* hitId);

private:
    // Obstacle convexe stocke par son centroide. Sommets locaux relatifs au centroide initial.
    struct Obstacle
    {
        std::vector<Vec2> local;   // sommets relatifs au centroide (sens trigo)
        float cx0;                 // centroide initial (terrain) : reference pour le deplacement
        float cy0;
        float x;                   // centroide courant (terrain) : translation
        float y;
        float teta;                // rotation courante autour du centroide (rad)
        int   id;                  // identifiant logique (index d'element de jeu)
        bool  mobile;              // element poussable (true) ou fixe facon bordure (false)
        bool  pickable;            // crochet : element prehensible (non implemente)
    };

    // Calcule les sommets monde d'un obstacle (local -> terrain via sa pose centroide).
    void obstacleWorldVertices(const Obstacle& obs, std::vector<Vec2>& out) const;

    // Calcule les sommets monde du robot pose en (x,y,teta).
    void robotWorldVertices(float x, float y, float teta, std::vector<Vec2>& out) const;

    // SAT entre deux polygones convexes (sommets monde). Si recouvrement, renseigne mtv
    // (vecteur a ajouter a polyA pour le separer de polyB) et renvoie true. Sinon false.
    static bool satOverlap(const std::vector<Vec2>& polyA,
                           const std::vector<Vec2>& polyB,
                           Vec2& mtv);

    // Repousse le robot a l'interieur des bordures (polygone vs demi-plans axis-aligned).
    // Renvoie true si un bord a ete touche. Modifie x,y.
    bool resolveBorders(float& x, float& y, float teta);

    // Borne la pose d'un element pousse pour qu'il reste sur le terrain (son AABB monde
    // ne sort pas de [x_min,x_max]x[y_min,y_max]). Evite qu'une caisse poussee quitte la table.
    void clampObstacleToField(Obstacle& obs) const;

    // Pousse + tourne un element mobile sous une poussee de MTV (force = -mtv appliquee au
    // point de contact estime). robotW = sommets monde du robot pour estimer le contact.
    // Renvoie la translation REELLEMENT appliquee au centroide (apres clamp terrain) : quand
    // l'element est coince contre une bordure, cette translation est plus faible que -mtv, et
    // resolve() transmet la part non absorbee au robot (l'element arrete alors le robot au lieu
    // de le laisser traverser). La rotation est mise a l'echelle de cette part realisee.
    Vec2 pushMobileObstacle(Obstacle& obs, const Vec2& mtv, const std::vector<Vec2>& robotW);

    // Point P appartient-il au polygone convexe poly (sommets monde) ? (tolerance incluse)
    static bool pointInConvex(const std::vector<Vec2>& poly, float px, float py);

    // Bornes terrain (cm).
    float m_x_min;
    float m_y_min;
    float m_x_max;
    float m_y_max;

    // Forme robot en repere local (cm).
    std::vector<Vec2> m_robot_local;

    // Elements de jeu.
    std::vector<Obstacle> m_obstacles;

    bool m_enabled;

    // Nombre maxi de passes de resolution (un push hors d'un obstacle peut en chevaucher
    // un autre, et la separation element<->element se propage en chaine). On itere jusqu'a
    // stabilisation. Robot lent + elements espaces => converge en quelques passes.
    static const int k_max_iterations = 8;

    // Modele de rotation sous poussee (cinematique, pas dynamique) :
    //  dteta = clamp(k_push_rot_gain * couple, +/- k_push_rot_max), couple = bras_de_levier x force.
    // Valeurs volontairement modestes : la poussee se faisant a chaque tick, meme une petite
    // rotation par tick produit une mise en rotation visible sans emballement.
    static constexpr float k_push_rot_gain = 0.004f; // rad par (cm*cm) de couple
    static constexpr float k_push_rot_max  = 0.06f;  // rad : rotation maxi par tick (~3.4 deg)

    // Seuil de recouvrement (cm) en dessous duquel on ignore une collision element<->element
    // (evite le jitter sur les caisses initialement jointives).
    static constexpr float k_elem_overlap_eps = 0.05f;
};

#endif // CCOLLISIONENGINE_H
