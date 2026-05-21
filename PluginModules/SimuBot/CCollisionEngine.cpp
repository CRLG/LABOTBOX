/*! \file CCollisionEngine.cpp
    \brief Implementation du moteur de collisions geometriques maison (SAT + bordures).

    Reference SAT : pour deux polygones convexes, ils sont disjoints si et seulement s'il
    existe un axe de separation parmi les normales des aretes des deux polygones. S'ils se
    chevauchent sur tous ces axes, le plus petit recouvrement donne le vecteur de penetration
    minimal (MTV) qui les separe.
*/

#include "CCollisionEngine.h"
#include <math.h>
#include <float.h>

// Seuil d'arete degenere (sommet duplique) : une arete plus courte ne produit pas d'axe.
static const float k_eps_edge = 1.0e-4f;

CCollisionEngine::CCollisionEngine()
    : m_x_min(0.0f),
      m_y_min(0.0f),
      m_x_max(300.0f),   // terrain 2026 : 300 x 200 cm
      m_y_max(200.0f),
      m_enabled(true)
{
}

void CCollisionEngine::setTerrain(float x_min, float y_min, float x_max, float y_max)
{
    m_x_min = x_min;
    m_y_min = y_min;
    m_x_max = x_max;
    m_y_max = y_max;
}

void CCollisionEngine::setRobotShape(const std::vector<Vec2>& local_vertices)
{
    m_robot_local = local_vertices;
}

int CCollisionEngine::addObstacleRect(float x_min, float y_min, float x_max, float y_max, int id, bool mobile)
{
    // Rectangle axis-aligned -> stocke par son centroide. Sommets locaux (sens trigo) relatifs
    // au centroide, pour pouvoir tourner l'element autour de son propre centre.
    const float cx = 0.5f * (x_min + x_max);
    const float cy = 0.5f * (y_min + y_max);
    Obstacle obs;
    obs.local.push_back({x_min - cx, y_min - cy});
    obs.local.push_back({x_max - cx, y_min - cy});
    obs.local.push_back({x_max - cx, y_max - cy});
    obs.local.push_back({x_min - cx, y_max - cy});
    obs.cx0 = cx; obs.cy0 = cy;
    obs.x = cx; obs.y = cy;
    obs.teta = 0.0f;
    obs.id = id;
    obs.mobile = mobile;
    obs.pickable = false;
    m_obstacles.push_back(obs);
    return (int)m_obstacles.size() - 1;
}

int CCollisionEngine::addObstacleConvex(const std::vector<Vec2>& vertices_terrain, int id, bool mobile)
{
    // Centroide = moyenne des sommets (convexe). Sommets stockes relativement au centroide.
    float cx = 0.0f, cy = 0.0f;
    const size_t n = vertices_terrain.size();
    if (n > 0)
    {
        for (size_t i = 0; i < n; ++i) { cx += vertices_terrain[i].x; cy += vertices_terrain[i].y; }
        cx /= (float)n; cy /= (float)n;
    }
    Obstacle obs;
    obs.local.resize(n);
    for (size_t i = 0; i < n; ++i) { obs.local[i].x = vertices_terrain[i].x - cx; obs.local[i].y = vertices_terrain[i].y - cy; }
    obs.cx0 = cx; obs.cy0 = cy;
    obs.x = cx; obs.y = cy;
    obs.teta = 0.0f;
    obs.id = id;
    obs.mobile = mobile;
    obs.pickable = false;
    m_obstacles.push_back(obs);
    return (int)m_obstacles.size() - 1;
}

int CCollisionEngine::addObstacleEllipse(float cx, float cy, float rx, float ry, int id, bool mobile, int n)
{
    // Approximation polygonale convexe : n sommets regulierement repartis sur l'ellipse,
    // en sens trigonometrique (angle croissant). Delegue a addObstacleConvex (centroide,
    // stockage relatif, etc.). Convexe par construction -> SAT valide.
    if (n < 3) n = 3;
    const float k_two_pi = 6.28318530717958647692f;
    std::vector<Vec2> verts;
    verts.reserve((size_t)n);
    for (int i = 0; i < n; ++i)
    {
        const float a = k_two_pi * (float)i / (float)n;
        verts.push_back({ cx + rx * cosf(a), cy + ry * sinf(a) });
    }
    return addObstacleConvex(verts, id, mobile);
}

int CCollisionEngine::addObstacleCircle(float cx, float cy, float r, int id, bool mobile, int n)
{
    // Cercle = ellipse a rayons egaux.
    return addObstacleEllipse(cx, cy, r, r, id, mobile, n);
}

void CCollisionEngine::clearObstacles()
{
    m_obstacles.clear();
}

void CCollisionEngine::resetObstaclePoses()
{
    for (size_t k = 0; k < m_obstacles.size(); ++k)
    {
        m_obstacles[k].x = m_obstacles[k].cx0;
        m_obstacles[k].y = m_obstacles[k].cy0;
        m_obstacles[k].teta = 0.0f;
    }
}

bool CCollisionEngine::getObstacleDisplacement(int id, float& dx, float& dy, float& dteta) const
{
    for (size_t k = 0; k < m_obstacles.size(); ++k)
    {
        if (m_obstacles[k].id == id)
        {
            dx    = m_obstacles[k].x - m_obstacles[k].cx0;
            dy    = m_obstacles[k].y - m_obstacles[k].cy0;
            dteta = m_obstacles[k].teta;
            return true;
        }
    }
    return false;
}

void CCollisionEngine::obstacleWorldVertices(const Obstacle& obs, std::vector<Vec2>& out) const
{
    const float c = cosf(obs.teta);
    const float s = sinf(obs.teta);
    out.resize(obs.local.size());
    for (size_t i = 0; i < obs.local.size(); ++i)
    {
        const float lx = obs.local[i].x;
        const float ly = obs.local[i].y;
        out[i].x = obs.x + lx * c - ly * s;
        out[i].y = obs.y + lx * s + ly * c;
    }
}

void CCollisionEngine::robotWorldVertices(float x, float y, float teta, std::vector<Vec2>& out) const
{
    const float c = cosf(teta);
    const float s = sinf(teta);
    out.resize(m_robot_local.size());
    for (size_t i = 0; i < m_robot_local.size(); ++i)
    {
        const float lx = m_robot_local[i].x;
        const float ly = m_robot_local[i].y;
        out[i].x = x + lx * c - ly * s;
        out[i].y = y + lx * s + ly * c;
    }
}

// Projette un polygone sur un axe unitaire et renvoie l'intervalle [mn, mx].
static inline void projectPolygon(const std::vector<CCollisionEngine::Vec2>& poly,
                                  float ax, float ay, float& mn, float& mx)
{
    mn = FLT_MAX;
    mx = -FLT_MAX;
    for (size_t i = 0; i < poly.size(); ++i)
    {
        const float p = poly[i].x * ax + poly[i].y * ay;
        if (p < mn) mn = p;
        if (p > mx) mx = p;
    }
}

// Centroide d'un polygone (moyenne des sommets) - suffit pour orienter le MTV.
static inline void polygonCenter(const std::vector<CCollisionEngine::Vec2>& poly,
                                 float& cx, float& cy)
{
    cx = 0.0f; cy = 0.0f;
    if (poly.empty()) return;
    for (size_t i = 0; i < poly.size(); ++i) { cx += poly[i].x; cy += poly[i].y; }
    cx /= (float)poly.size();
    cy /= (float)poly.size();
}

// Teste un jeu d'axes (normales des aretes d'un polygone) et met a jour le recouvrement mini.
// Renvoie false des qu'un axe separe (overlap <= 0). axesFrom = polygone fournissant les aretes.
static bool testAxes(const std::vector<CCollisionEngine::Vec2>& axesFrom,
                     const std::vector<CCollisionEngine::Vec2>& polyA,
                     const std::vector<CCollisionEngine::Vec2>& polyB,
                     float& minOverlap, float& mtvAx, float& mtvAy)
{
    const size_t n = axesFrom.size();
    for (size_t i = 0; i < n; ++i)
    {
        const CCollisionEngine::Vec2& p1 = axesFrom[i];
        const CCollisionEngine::Vec2& p2 = axesFrom[(i + 1) % n];
        // Normale a l'arete (p1->p2).
        float ax = -(p2.y - p1.y);
        float ay =  (p2.x - p1.x);
        const float len = sqrtf(ax * ax + ay * ay);
        if (len < k_eps_edge) continue; // arete degeneree (sommet duplique) : pas d'axe
        ax /= len;
        ay /= len;

        float minA, maxA, minB, maxB;
        projectPolygon(polyA, ax, ay, minA, maxA);
        projectPolygon(polyB, ax, ay, minB, maxB);

        const float overlap = (maxA < maxB ? maxA : maxB) - (minA > minB ? minA : minB);
        if (overlap <= 0.0f) return false; // axe separateur trouve -> pas de collision

        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            mtvAx = ax;
            mtvAy = ay;
        }
    }
    return true;
}

bool CCollisionEngine::satOverlap(const std::vector<Vec2>& polyA,
                                  const std::vector<Vec2>& polyB,
                                  Vec2& mtv)
{
    if (polyA.size() < 3 || polyB.size() < 3) return false;

    float minOverlap = FLT_MAX;
    float mtvAx = 0.0f, mtvAy = 0.0f;

    // Tous les axes : normales des aretes de A puis de B.
    if (!testAxes(polyA, polyA, polyB, minOverlap, mtvAx, mtvAy)) return false;
    if (!testAxes(polyB, polyA, polyB, minOverlap, mtvAx, mtvAy)) return false;

    // Oriente le MTV pour qu'il pousse A hors de B (de B vers A).
    float cAx, cAy, cBx, cBy;
    polygonCenter(polyA, cAx, cAy);
    polygonCenter(polyB, cBx, cBy);
    const float dx = cAx - cBx;
    const float dy = cAy - cBy;
    if (dx * mtvAx + dy * mtvAy < 0.0f) { mtvAx = -mtvAx; mtvAy = -mtvAy; }

    mtv.x = mtvAx * minOverlap;
    mtv.y = mtvAy * minOverlap;
    return true;
}

bool CCollisionEngine::pointInConvex(const std::vector<Vec2>& poly, float px, float py)
{
    // Convexe : le point est interieur si tous les produits vectoriels (arete x (P - sommet))
    // ont le meme signe. On tolere un leger debordement (>= -eps) pour les contacts au ras.
    const size_t n = poly.size();
    if (n < 3) return false;
    bool hasPos = false, hasNeg = false;
    for (size_t i = 0; i < n; ++i)
    {
        const Vec2& a = poly[i];
        const Vec2& b = poly[(i + 1) % n];
        const float cross = (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x);
        if (cross >  1.0e-3f) hasPos = true;
        if (cross < -1.0e-3f) hasNeg = true;
        if (hasPos && hasNeg) return false;
    }
    return true;
}

void CCollisionEngine::pushMobileObstacle(Obstacle& obs, const Vec2& mtv, const std::vector<Vec2>& robotW)
{
    // Centre de l'element AVANT deplacement (pour le bras de levier).
    const float cx = obs.x;
    const float cy = obs.y;

    // Estimation du point de contact : sommets de l'element situes a l'interieur du robot,
    // moyennes. A defaut (contact arete-arete), sommet de l'element le plus avance vers le
    // robot (projection maxi sur la direction du MTV, qui pointe de l'element vers le robot).
    std::vector<Vec2> obsW;
    obstacleWorldVertices(obs, obsW);
    const float nlen = sqrtf(mtv.x * mtv.x + mtv.y * mtv.y);
    const float nx = (nlen > 1.0e-6f) ? mtv.x / nlen : 0.0f;
    const float ny = (nlen > 1.0e-6f) ? mtv.y / nlen : 0.0f;

    float Px = 0.0f, Py = 0.0f;
    int nin = 0;
    for (size_t i = 0; i < obsW.size(); ++i)
        if (pointInConvex(robotW, obsW[i].x, obsW[i].y)) { Px += obsW[i].x; Py += obsW[i].y; ++nin; }

    if (nin > 0) { Px /= (float)nin; Py /= (float)nin; }
    else
    {
        float best = -FLT_MAX;
        for (size_t i = 0; i < obsW.size(); ++i)
        {
            const float proj = obsW[i].x * nx + obsW[i].y * ny;
            if (proj > best) { best = proj; Px = obsW[i].x; Py = obsW[i].y; }
        }
    }

    // Translation : l'element s'ecarte du robot de -MTV (separation).
    obs.x -= mtv.x;
    obs.y -= mtv.y;

    // Rotation : couple = bras_de_levier x force, force = -MTV (sens de la poussee sur l'element).
    // Modele cinematique borne (pas une vraie dynamique), suffisant pour l'effet "bras de levier".
    const float rx = Px - cx;
    const float ry = Py - cy;
    const float fx = -mtv.x;
    const float fy = -mtv.y;
    const float couple = rx * fy - ry * fx;     // produit vectoriel scalaire (z)
    float dteta = k_push_rot_gain * couple;
    if (dteta >  k_push_rot_max) dteta =  k_push_rot_max;
    if (dteta < -k_push_rot_max) dteta = -k_push_rot_max;
    obs.teta += dteta;

    clampObstacleToField(obs);
}

bool CCollisionEngine::resolveBorders(float& x, float& y, float teta)
{
    std::vector<Vec2> rv;
    robotWorldVertices(x, y, teta, rv);
    if (rv.empty()) return false;

    float minVx = FLT_MAX, maxVx = -FLT_MAX, minVy = FLT_MAX, maxVy = -FLT_MAX;
    for (size_t i = 0; i < rv.size(); ++i)
    {
        if (rv[i].x < minVx) minVx = rv[i].x;
        if (rv[i].x > maxVx) maxVx = rv[i].x;
        if (rv[i].y < minVy) minVy = rv[i].y;
        if (rv[i].y > maxVy) maxVy = rv[i].y;
    }

    bool touched = false;
    // Demi-plan gauche / droit (exact pour un polygone vs bord axis-aligned).
    if (minVx < m_x_min) { x += (m_x_min - minVx); touched = true; }
    else if (maxVx > m_x_max) { x += (m_x_max - maxVx); touched = true; }
    // Demi-plan bas / haut.
    if (minVy < m_y_min) { y += (m_y_min - minVy); touched = true; }
    else if (maxVy > m_y_max) { y += (m_y_max - maxVy); touched = true; }

    return touched;
}

void CCollisionEngine::clampObstacleToField(Obstacle& obs) const
{
    std::vector<Vec2> w;
    obstacleWorldVertices(obs, w);
    if (w.empty()) return;

    float minVx = FLT_MAX, maxVx = -FLT_MAX, minVy = FLT_MAX, maxVy = -FLT_MAX;
    for (size_t i = 0; i < w.size(); ++i)
    {
        if (w[i].x < minVx) minVx = w[i].x;
        if (w[i].x > maxVx) maxVx = w[i].x;
        if (w[i].y < minVy) minVy = w[i].y;
        if (w[i].y > maxVy) maxVy = w[i].y;
    }
    // Corrige la translation du centroide pour ramener l'AABB de l'element dans le terrain.
    if (minVx < m_x_min) obs.x += (m_x_min - minVx);
    else if (maxVx > m_x_max) obs.x += (m_x_max - maxVx);
    if (minVy < m_y_min) obs.y += (m_y_min - minVy);
    else if (maxVy > m_y_max) obs.y += (m_y_max - maxVy);
}

bool CCollisionEngine::resolve(float& x, float& y, float teta, int* hitId)
{
    if (hitId) *hitId = -1;
    if (!m_enabled) return false;
    if (m_robot_local.size() < 3) return false; // pas de forme robot -> rien a resoudre

    bool anyContact = false;
    std::vector<Vec2> robotW;
    std::vector<Vec2> obsW, obsW2;

    for (int iter = 0; iter < k_max_iterations; ++iter)
    {
        bool movedThisIter = false;

        // 1. Bordures d'abord (les obstacles ne doivent pas pousser le robot hors terrain).
        if (resolveBorders(x, y, teta))
        {
            anyContact = true;
            movedThisIter = true;
        }

        // 2. Robot vs elements : SAT robot vs chaque obstacle.
        robotWorldVertices(x, y, teta, robotW);
        for (size_t k = 0; k < m_obstacles.size(); ++k)
        {
            obstacleWorldVertices(m_obstacles[k], obsW);
            Vec2 mtv;
            if (!satOverlap(robotW, obsW, mtv)) continue;

            if (hitId) *hitId = m_obstacles[k].id;
            anyContact = true;
            movedThisIter = true;

            if (m_obstacles[k].mobile)
            {
                // Element POUSSABLE : on deplace + tourne l'element ; le robot continue.
                pushMobileObstacle(m_obstacles[k], mtv, robotW);
            }
            else
            {
                // Element FIXE (traite comme une bordure) : on repousse le robot de +MTV.
                x += mtv.x;
                y += mtv.y;
                robotWorldVertices(x, y, teta, robotW);
            }
        }

        // 3. Separation element <-> element : evite que les caisses poussees se superposent.
        //    Translation seule (pas de rotation ici) pour rester robuste ; la chaine se propage
        //    sur les iterations suivantes. On ne traite une paire que si au moins un est mobile.
        for (size_t i = 0; i < m_obstacles.size(); ++i)
        {
            for (size_t j = i + 1; j < m_obstacles.size(); ++j)
            {
                if (!m_obstacles[i].mobile && !m_obstacles[j].mobile) continue;
                obstacleWorldVertices(m_obstacles[i], obsW);
                obstacleWorldVertices(m_obstacles[j], obsW2);
                Vec2 mtv; // a ajouter a i pour le separer de j
                if (!satOverlap(obsW, obsW2, mtv)) continue;
                if (sqrtf(mtv.x * mtv.x + mtv.y * mtv.y) < k_elem_overlap_eps) continue;

                if (m_obstacles[i].mobile && m_obstacles[j].mobile)
                {
                    // Les deux bougent : on partage la separation.
                    m_obstacles[i].x += 0.5f * mtv.x; m_obstacles[i].y += 0.5f * mtv.y;
                    m_obstacles[j].x -= 0.5f * mtv.x; m_obstacles[j].y -= 0.5f * mtv.y;
                    clampObstacleToField(m_obstacles[i]);
                    clampObstacleToField(m_obstacles[j]);
                }
                else if (m_obstacles[i].mobile)
                {
                    m_obstacles[i].x += mtv.x; m_obstacles[i].y += mtv.y;
                    clampObstacleToField(m_obstacles[i]);
                }
                else
                {
                    m_obstacles[j].x -= mtv.x; m_obstacles[j].y -= mtv.y;
                    clampObstacleToField(m_obstacles[j]);
                }
                movedThisIter = true;
            }
        }

        if (!movedThisIter) break; // stabilise : plus aucun chevauchement
    }

    return anyContact;
}
