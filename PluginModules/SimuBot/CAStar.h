/** \brief CAStar.h
 * Extrait de https://fr.wikipedia.org/wiki/Algorithme_A*
 *
 * A* commence a un noeud choisi. Il applique a ce noeud un « cout »
 * (habituellement zero pour le noeud initial). A* estime ensuite la
 * distance qui separe ce noeud du but a atteindre. La somme du cout et
 * de l'evaluation represente le cout heuristique assigne au chemin
 * menant a ce noeud. Le noeud est alors ajoute a une file d'attente
 * prioritaire, couramment appelee open list.
 *
 * L'algorithme retire le premier noeud de la file d'attente prioritaire (du
 * au fonctionnement d'une file d'attente, le noeud a l'heuristique la plus
 * basse est retire en premier). Si la file d'attente est vide, il n'y a aucun
 * chemin du noeud initial au noeud d'arrivee, ce qui interrompt
 * l'algorithme. Si le noeud retenu est le noeud d'arrivee, A* reconstruit le
 * chemin complet et s'arrête. Pour cette reconstruction on se sert d'une
 * partie des informations sauvees dans la liste communement appele
 * closed list decrite plus bas.
 *
 * Si le noeud n'est pas le noeud d'arrivee, de nouveaux noeuds sont crees
 * pour tous les noeuds contigus admissibles ; la maniere exacte de faire depend du probleme a traiter. Pour
 * chaque noeud successif, A* calcule son cout et le stocke avec le noeud. Ce cout est calcule a partir de la
 * somme du cout de son ancêtre et du cout de l'operation pour atteindre ce nouveau noeud.
 *
 * L'algorithme maintient egalement la liste de noeuds qui ont ete verifies, couramment appelee closed list. Si
 * un noeud nouvellement produit est deja dans cette liste avec un cout egal ou inferieur, aucune operation n'est
 * faite sur ce noeud ni sur son homologue se trouvant dans la liste.
 *
 * Apres, l'evaluation de la distance du nouveau noeud au noeud d'arrivee est ajoutee au cout pour former
 * l'heuristique du noeud. Ce noeud est alors ajoute a la liste d'attente prioritaire, a moins qu'un noeud identique
 * dans cette liste ne possede deja une heuristique inferieure ou egale.
 *
 * Une fois les trois etapes ci-dessus realisees pour chaque nouveau noeud contigu, le noeud original pris de la
 * file d'attente prioritaire est ajoute a la liste des noeuds verifies. Le prochain noeud est alors retire de la file
 * d'attente prioritaire et le processus recommence.
 *
 */


#ifndef _ASTAR_H_
#define _ASTAR_H_
#include <iostream>
#include <iomanip>
#include <queue>
#include <string>
#include <math.h>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <cmath>
#include <utility>
#include <stdexcept>

#define PRECISION 5

/**< taille horizontale du terrain avec un pas de "PRECISION" en cm */
#define n 300/PRECISION
/**< taille verticale du terrain avec un pas de "PRECISION" en cm */
#define m 200/PRECISION
/**< nombre de directions possibles a partir d'un point
    8 on peut aller en diagonale, 4 juste les directions orthogonales */
#define dir 8
/**< Euclidien=0, Manhattan=1, Chebyshev=2 */
#define method_dist 0

#define IMAX 500

typedef std::pair<double, double> Point;

/**
 *  Classe definissant un noeud dans l'algorithme A*
 */
class node
{
private:
    //position courante du noeud
    int xPos;
    int yPos;
    //Distance totale deja parcourue pour atteindre le noeud
    int level;
    //priorite=distance+distance restante estimee, du coup un petit nombre equivaut a une priorite
    //elevee, logique on cherche le chemin le plus court ;-)
    int priority;

public:
    node(int xp, int yp, int d, int p); //initialise position, distance et priorite initiales
    //accesseurs
    int getxPos() const;
    int getyPos() const;
    int getLevel() const;
    int getPriority() const;
    //mise a jour de la priorite
    void updatePriority(const int & xDest, const int & yDest);
    void nextLevel(const int & i);
    const int & estimate(const int & xDest, const int & yDest) const;

    bool operator<(const node &) const;
};

class AStar
{
public:

    AStar();

    int map[n][m];
    int closed_nodes_map[n][m]; // map of closed (tried-out) nodes
    int open_nodes_map[n][m]; // map of open (not-yet-tried) nodes

    int dir_map[n][m]; // map of directions

    /*
    //dir==4
    static int dx[dir]={1, 0, -1, 0};
    static int dy[dir]={0, 1, 0, -1};
    */

    //dir==8
    int dx[dir];
    int dy[dir];
    int nb_points;
    int i_path_dir[IMAX];
    int i_x_dir[IMAX];
    int i_y_dir[IMAX];

    void initMap(int xRobot,int yRobot);
    void addBot2Map(int xRobot,int yRobot);
    int pathFind( const int & xStart, const int & yStart, const int & xFinish, const int & yFinish );
    int pathBuild(int x0, int y0);

    //Racine carre tres rapide (tire/pompe d'une pres RCA)
    static float r_sqrt( float in )
    {
        long estimator;
        float out,inhalfs;
        const float threehalfs=1.5F;
        inhalfs=in*0.5F;
        estimator=*(long*)&in; // get bits from float to int
        estimator=0x5f375a86-(estimator>>1); // strange estimator...
        out =*(float*)&estimator; // get bits back from int to float
        out*=(threehalfs-(inhalfs*out*out)); // Newton iteration
        out*=(threehalfs-(inhalfs*out*out)); // Newton iteration
        //out*=(threehalfs-(inhalfs*out*out)); // Newton iteration
        //out*=(threehalfs-(inhalfs*out*out)); // Newton iteration
        return out*in;
    }

private:
    int adapt(int coord);
    int expand(int coord);
    double PerpendicularDistance(const Point &pt, const Point &lineStart, const Point &lineEnd);
    void RamerDouglasPeucker(const std::vector<Point> &pointList, double epsilon, std::vector<Point> &out);
};
#endif
