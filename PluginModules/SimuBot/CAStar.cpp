#include "CAStar.h"

/*__________________________________________
    NODE
  __________________________________________*/

node::node(int xp, int yp, int d, int p)
{
    xPos=xp; yPos=yp; level=d; priority=p;
}

int node::getxPos() const{return xPos;}
int node::getyPos() const {return yPos;}
int node::getLevel() const {return level;}
int node::getPriority() const {return priority;}

void node::updatePriority(const int & xDest, const int & yDest)
{
    priority=level+estimate(xDest, yDest)*10; //A*
}

void node::nextLevel(const int & i) // i: direction
{
    //on donne une priorite accrue aux deplacements droits plutot
    //qu'en diagonales pour choisir le prochain noeud (bien adapte
    //au robot qui a ses capteur de distance a l'avant et a l'arriere)
    level+=(dir==8?(i%2==0?10:14):10);
}

/**
 * Fonction d'estimation de la distance restante separant le noeud de l'objectif
 * 3 choix possibles:
 * Euclidien (ou quadratique) => somme de carres
 * Manhattan ou Chebyshev => pas de multiplications donc caclcul normalement plus rapide
 */
const int & node::estimate(const int & xDest, const int & yDest) const
{
    static int xd, yd, d;
    xd=xDest-xPos;
    yd=yDest-yPos;
    switch (method_dist)
    {
        //Euclidien
        case 0: d=static_cast<int>(sqrt(xd*xd+yd*yd));
                //d=static_cast<int>(fsqrt(xd*xd+yd*yd));
                break;
        // Manhattan
        case 1: d=abs(xd)+abs(yd);
                break;
        // Chebyshev
        case 2: d=std::max(abs(xd), abs(yd));
                break;

        default: d=static_cast<int>(sqrt(xd*xd+yd*yd));
                break;
    }

    return(d);
}

/**
 * surcharge de l'operateur pour pouvoir ordonner les noeuds suivant leur priorite
 */
bool node::operator<(const node &b) const
{
    int a_prio=this->getPriority();
    int b_prio=b.getPriority();
    bool b_inf=(a_prio>b_prio);
    return  b_inf;
}

/*__________________________________________
    A*
  __________________________________________*/

AStar::AStar()
{
/*
//dir==4
dx[0]=1;dx[1]=0;dx[2]=-1;dx[3]=0;
dy[0]=0;dy[1]=1;dy[2]=0;dy[3]=-1;
*/

//dir==8
/*
-1 0 1    -1-1-1
-1 x 1     0 y 0
-1 0 1     1 1 1
*/
dx[0]=1;dx[1]=1;dx[2]=0;dx[3]=-1;dx[4]=-1;dx[5]=-1;dx[6]=0;dx[7]=1;
dy[0]=0;dy[1]=1;dy[2]=1;dy[3]=1;dy[4]=0;dy[5]=-1;dy[6]=-1;dy[7]=-1;
}

/**
 * Algorithme A*
 * Le chemin est accessible via un vector
 * @return Le nombre de points composant le chemin, 0 sinon
 */
int AStar::pathFind( const int & xStart, const int & yStart,
                    const int & xFinish, const int & yFinish )
{
    std::priority_queue<node> pq[2]; //double liste des neuds ouverts (chaque element ajoute sera
                                    //trie par priorite decroissante, priority_queue de la
                                    //lib standard)
    int pqi; //index de la liste
    node* n0;
    node* m0;
    int i, j, x, y, xdx, ydy;
    pqi=0;

    //on reserve de la memoire pour au moins 200 elements pour stocker le chemin
    path_dir.reserve(150);
    x_dir.reserve(150);
    y_dir.reserve(150);


    //on remet a zero la carte des noeuds
    for(y=0;y<m;y++)
        for(x=0;x<n;x++)
        {
            closed_nodes_map[x][y]=0;
            open_nodes_map[x][y]=0;
        }

    //creation du noeud de depart et on l'inclut dans la liste des noeuds ouverts
    n0=new node(xStart, yStart, 0, 0);
    n0->updatePriority(xFinish, yFinish);
    pq[pqi].push(*n0);
    open_nodes_map[x][y]=n0->getPriority(); //on l'indique dans la carte des noeuds ouverts

    //Recherche A*
    while(!pq[pqi].empty())
    {
        //On prend le noeud courant avec la plus haute priorite
        //de la liste des noeuds ouverts
        n0=new node( pq[pqi].top().getxPos(), pq[pqi].top().getyPos(),
        pq[pqi].top().getLevel(), pq[pqi].top().getPriority());
        x=n0->getxPos(); y=n0->getyPos();
        pq[pqi].pop(); // le noeud est traite on le retire de la liste des noeuds ouverts
        open_nodes_map[x][y]=0;
        //et on l'indique dans la carte des noeuds fermes
        closed_nodes_map[x][y]=1;

        //on interrompt l'algo si le point de destination est atteint
        //if((*n0).estimate(xFinish, yFinish) < distance mini) a prendre en cas de pb d'arrondi
        if(x==xFinish && y==yFinish)
        {
            //generation du chemin de la fin au debut
            //en suivant les directions
            int nb_points=0;
            while(!(x==xStart && y==yStart))
            {
                j=dir_map[x][y];
                int k=(j+dir/2)%dir; //direction symetrique pour avoir le chemin du point
                                    //de depart a l'arrivee
                path_dir.push_back(k);
                x+=dx[j];
                y+=dy[j];
                nb_points++;
            }
            //garbage collection
            delete n0;
            //on vide la liste des noeuds restants
            while(!pq[pqi].empty()) pq[pqi].pop();
            return nb_points;
        }

        //on genere tous les noeuds (enfants) contigus admissibles
        //(toutes les directions possibles)
        for(i=0;i<dir;i++)
        {
            xdx=x+dx[i]; ydy=y+dy[i];

            //si on n'est pas en dehors du terrain (mettre une marge)
			//si il n'y a pas d'obstacle (cartographie "map" en prenant en compte le perimetre robot)
			//si le noeud n'est pas encore totalement verifie
            if(!(xdx<0 || xdx>n-1 || ydy<0 || ydy>m-1 || map[xdx][ydy]==1
            || closed_nodes_map[xdx][ydy]==1))
            {
                //generation d'un noeud enfant
                m0=new node( xdx, ydy, n0->getLevel(),
                n0->getPriority());
                m0->nextLevel(i);
                m0->updatePriority(xFinish, yFinish);
                //Si il n'est pas dans la liste des noeuds ouverts, on l'ajoute
                if(open_nodes_map[xdx][ydy]==0)
                {
                    open_nodes_map[xdx][ydy]=m0->getPriority();
                    pq[pqi].push(*m0);
                    //on sauvegarde la direction du noeud parent (pour pouvoir,
                    //en rebroussant chemin, reconstruire plus tard le chemin du depart
                    //a l'arrivee
                    dir_map[xdx][ydy]=(i+dir/2)%dir;
                }
                else if(open_nodes_map[xdx][ydy]>m0->getPriority())
                {
                    //Mise a jour des infos de priorite
                    open_nodes_map[xdx][ydy]=m0->getPriority();
                    //mise a jour des infos de direction du noeud parent
                    dir_map[xdx][ydy]=(i+dir/2)%dir;
                    //on remplace le noeud
					//en vidant la liste "ouverte" dans l'autre liste "ouverte"
					//a l'exception du noeud a remplecer qui sera ignore
					//le nouveau noeud sera choisi a sa place
                    while(!(pq[pqi].top().getxPos()==xdx && pq[pqi].top().getyPos()==ydy))
                    {
                        pq[1-pqi].push(pq[pqi].top());
                        pq[pqi].pop();
                    }

                    pq[pqi].pop(); //on retire le noeud recherche

					//si la taille de la iste "ouverte" ainsi obtenue est plus petite que la liste
					//"ouverte" source, on permute l'index et elle devient la nouvelle liste de reference
                    if(pq[pqi].size()>pq[1-pqi].size()) pqi=1-pqi;
                    while(!pq[pqi].empty())
                    {
                        pq[1-pqi].push(pq[pqi].top());
                        pq[pqi].pop();
                    }
                    pqi=1-pqi;

                    pq[pqi].push(*m0); //on ajoute le meilleur noeud a la place
                }
                else delete m0; // garbage collection
            }
        }
    delete n0; // garbage collection
    }

    return 0; //aucune route trouvee (echec de l'algorithme)
}

void AStar::initMap(int xRobot,int yRobot)
{
    //on cree une carte d'obstacle vide
    for(int y=0;y<m;y++)
        for(int x=0;x<n;x++)
            map[x][y]=0;
/*
    //pour les tests on met un obstacle en forme de '+' au milieu du terrain
    for(int x=n/8;x<n*7/8;x++)
        map[x][m/2]=1;
    for(int y=m/8;y<m*7/8;y++)
        map[n/2][y]=1;
*/
    //on construit la zone du robot adverse
    int r=40; //rayon robot
   	int botShape=2; //1 carre, 2 cercle
   	int bold=3; //epaisseur perimetre (cercle ou polygone)
	
    if(botShape==1)
    {
        //on remplit la carte avec la zone de l'autre robot
        for(int i=0;i<r;i++)
            for(int j=0;j<r;j++)
        {
            map[xRobot+i][yRobot+j]=1;
            map[xRobot+i][yRobot-j]=1;
            map[xRobot-i][yRobot+j]=1;
            map[xRobot-i][yRobot-j]=1;
        }
    }
    else
    {
		//Algorithme de trace de cercle d'Andres
        int nb_trace=0;
        while(nb_trace<bold)
        {
            int x_c=0;
            int y_c=r-nb_trace;
            int d=r-nb_trace-1;
            while (y_c>=x_c)
            {
                map[xRobot + x_c][yRobot + y_c]=1;
                map[xRobot + y_c][yRobot + x_c]=1;

                map[xRobot - x_c][yRobot + y_c]=1;
                map[xRobot - y_c][yRobot + x_c]=1;
                map[xRobot + x_c][yRobot - y_c]=1;
                map[xRobot + y_c][yRobot - x_c]=1;
                map[xRobot - x_c][yRobot - y_c]=1;
                map[xRobot - y_c][yRobot - x_c]=1;
                if(d >= (2*x_c))
                {
                    d-=2*x_c-1;
                    x_c++;
                }
                else if (d < (2*(r-y_c)))
                {
                    d+=2*y_c-1;
                    y_c--;
                }
                else
                {
                    d+=2*(y_c-x_c-1);
                    y_c--;
                    x_c++;
                }
            }
            nb_trace++;
        }

    }
}

/**
 * Permet de "lisser" la trajectoire qui est normalement en point à point sur la grille de 2cm
 * tant que la direction est inchangee on compte le nombre de pas, au changement de direction
 * on en deduit la nouvelle coordonnee en ajoutant le nombre de pas dans une direction donnee a l'ancienne coordonnee
 */
int AStar::pathBuild(int x0, int y0)
{
   if(path_dir.empty())
    return 0;
   else
   {
       std::vector<int>::reverse_iterator it;
       int trace_x=x0; int trace_y=y0;
       int dx_prec=0; int dy_prec=0;
       int k=0;
       int nb_points=0;
       //std::cout <<"Trajectoire:"<<std::endl;
       for(it=path_dir.rbegin(); it!=path_dir.rend(); ++it)
        {
            if((dx[*it]==dx_prec)&&(dy[*it]==dy_prec))
            {

                k++;
            }
            else
            {

                trace_x=trace_x+k*dx_prec;
                trace_y=trace_y+k*dy_prec;

                x_dir.push_back(trace_x);
                y_dir.push_back(trace_y);

                dx_prec=dx[*it];
                dy_prec=dy[*it];

                //std::cout <<"("<<trace_x<<","<<trace_y<<");";

                k=1;
                nb_points++;
            }
        }

        //std::cout << std::endl;
        return nb_points+1;
   }
}



