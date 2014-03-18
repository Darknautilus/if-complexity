#include<stdio.h> 
#include<math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#define MAXMOT 256
#define MAXS 500

/**
 * Structure pour representer un cycle
 */
typedef struct t_cycle 
{
  int taille;   //la taille du cycle en construction
  double poids; //le coût du cycle
  int c[MAXS];  //liste des "taille" sommets
} t_cycle;     


/**
 * Charge le CSV des coordonnées des villes.
 * 
 * @param [in] f le fichier
 * @param [out] nb_villes le nombre de villes de l'instance
 * @param [out] dist le tableau des nb_villes*nb_villes distances
 * @param [out] absc le tableau des abscisses des villes
 * @param [out] ord le tableau des ordonnées des villes
 */
void lire_donnees(const char *f, unsigned int *nb_villes, double ***dist,  double **absc, double **ord)
{
  //double *absc; ///tableau des ordonnées
  //double *ord;  /// tableau des abscisses
  char ligne[MAXMOT];
  FILE * fin = fopen(f,"r");

  if(fin != NULL)
    {
      //On recupere le nombre de villes
      fgets(ligne, MAXMOT, fin);
      *nb_villes = atoi(ligne);
      (*dist) = (double**)malloc(*nb_villes * sizeof(double*));
      (*absc) = (double*)malloc(*nb_villes * sizeof(double));
      (*ord) = (double*)malloc(*nb_villes * sizeof(double));
      int i = 0;
      while (fgets(ligne, MAXMOT, fin) != NULL) 
	{      
	  char *p = strchr(ligne, ';');
	  ligne[strlen(ligne) - strlen(p)]='\0';
	  p = &p[1];
	  (*absc)[i] = atof(ligne);
	  (*ord)[i] = atof(p);
	  i = i + 1;
	}
    }
  else
    {
      printf("Erreur de lecture du fichier.\n");
      exit(2);
    }
  fclose(fin);
  int i,j;

  //Calclul des distances
  for(i = 0; i < *nb_villes; i++)
    {
      (*dist)[i] = (double*) malloc(*nb_villes * sizeof(double));
      for(j = 0; j < *nb_villes; j++)
	{
	  (*dist)[i][j] = sqrt( ((*absc)[i] - (*absc)[j])* ((*absc)[i] - (*absc)[j]) + ((*ord)[i] - (*ord)[j]) * ((*ord)[i] - (*ord)[j]) );
	}
    }
}


/**
 * Supprime la structure des distances
 *
 * @param [in] nb_villes le nombre de villes.
 * @param [in,out] distances le tableau à supprimer.
 * @param [in,out] abscisses un autre tableau à supprimer.
 * @param [in,out] ordonnees encore un autre tableau à supprimer.
 */
void supprimer_distances_et_coordonnees(const int nb_villes, double **distances, double *abscisses, double *ordonnees)
{
  int i;
  for(i = 0; i < nb_villes; i++)
    {
      free(distances[i]);
    }
 free(distances);
 free(abscisses);
 free(ordonnees);
}


/**
 * Export le cycle dans un fichier HTML pour pouvoir être visualisé
 * dans l'applet.
 *
 * @param [in] cycle le cycle à afficher
 */
void afficher_cycle_html(const t_cycle cycle, double *posX, double *posY)
{
  FILE * fout = fopen("DisplayTsp2.html","w");
  if(fout != NULL)
    {
      int i;
      fprintf(fout, "<html>\n <applet codebase=\".\" code=\"DisplayTsp.class\" width=500 height=500>\n");
      fprintf(fout, "<param name = Problem value = \"custom\">\n");
      fprintf(fout, "<param name = Problem CitiesPosX value = \"");
      for(i = 0; i < cycle.taille; i++)
	fprintf(fout,"%f;",posX[i]);
      fprintf(fout, "\">\n");
      fprintf(fout, "<param name = Problem CitiesPosY value = \"");
      for(i = 0; i < cycle.taille; i++)
	fprintf(fout,"%f;",posY[i]);
      fprintf(fout, "\">\n");
      fprintf(fout, "<param name = Parcours value = \"");
      fprintf(fout,"%d",cycle.c[0]);
      for(i = 1; i < cycle.taille; i++)
	fprintf(fout,"-%d",cycle.c[i]);
      fprintf(fout,"\">\n</applet>\n </html>\n");
    }
  fclose(fout);
}

/**
 * Affiche le tableau des distances.
 *
 * @param [in] nb le nombre de villes
 * @param [in] distances le tableau
 */
void afficher_distances(const int nb, double **distances)
{
  unsigned int i ;
  unsigned int j ;
  for(i = 0  ; i < nb; i++)
    {   
      for(j = 0 ; j < nb ; j++)
	printf("%f ", distances[i][j]);
      printf("\n");
    }
  printf("\n");
}


/**
 * Fonction de comparaison pour le trie des arêtes par leur poids.
 *
 * @param [in] v1 pointeur vers un triplet (i,j,poids)
 * @param [in] v2 pointeur verts un triplet (i,j,poids)
 * @return vrai si poid v1 < poids v2
 */
int comparer(const void *v1, const void *v2)
{
  double **px1 = (double **) v1;
  double **px2 = (double **) v2;

  double *x1 = *px1;
  double *x2 = *px2;
  if(x1[2] - x2[2] < 0)
    return -1;
  else 
    {
      if(x1[2] - x2[2] == 0)
	return 0;
      else
	return 1;
    }
}

/**
 * Construit un tableau de n*(n-1)/2 arêtes trié selon leur poids.
 *
 * @note utile pour le Kruskal
 *
 * @param [in] n le nombre de villes
 * @param [in] d tableau des n x n distances.
 * @return tableau d'arétes triées T[i][j] = poids_ij 
 */
double **trier_aretes(const int n, double **d)
{
  assert(d);
  
  int nb_aretes = n * (n - 1) / 2;
  double **T = (double **)malloc(nb_aretes * sizeof(double *));
  int i, j;
  int a = 0;

  //On initialise la structure d'arêtes
  for(i = 0; i < n-1; i++)
    {
      for(j = i+1; j < n; j++)
	{
	  T[a] = (double *)malloc(3 * sizeof(double));
	  T[a][0] = i;
	  T[a][1] = j;
	  T[a][2] = d[i][j];
	  a++;
	}
    }
  
  //Appel au quicksort avec la bonne fonction de comparaison
  qsort(T, a, sizeof(T[0]), comparer);
  

  //Decommenter pour vérifier le tri
  /*
  for(i = 0; i < a; i++)
    printf("%f ", T[i][2]);
  printf("\n");
  */
  return T;
}


/**
 * Supprime le tableau des arêtes.
 *
 * @param [in] nb_villes le nombre de villes
 * @param [in,out] T le tableau à supprimer
 */
void supprimer_aretes(const int nb_villes, double **T)
{
  assert(T);

  int nb_aretes = nb_villes*(nb_villes - 1 ) / 2;
  unsigned int i;
  for( i = 0; i < nb_aretes ; ++i)
    free( T[i] );

  free(T);
}

// cycle et meilleur_cycle doivent être initialisés avant l'appel à la fonction
void PVC_EXACT_NAIF(t_cycle *cycle, t_cycle *meilleur_cycle, const unsigned int nb_villes, const double **distances)
{
	if(cycle->taille == 0)
	{
		cycle->c[0] = 0;
		cycle->taille++;
		/*
		int i;

		for(i=0;i<nb_villes;i++)
		{
			cycle->c[i] = -1;
		}*/
	}
	if(meilleur_cycle->poids == 0 || cycle->poids < meilleur_cycle->poids)
	{
		if(cycle->taille < nb_villes)
		{
			int i;
			for(i=1;i<nb_villes;i++)
			{
				int exists = 0;
				int j;
				for(j=0;j<cycle->taille;j++)
				{
					if(cycle->c[j] == i)
					{
						exists = 1;
						break;
					}
				}
				if(!exists)
				{
					cycle->c[cycle->taille] = i;
					double poids = distances[cycle->c[cycle->taille-1]][i];
					cycle->poids += poids;
					cycle->taille++;
					PVC_EXACT_NAIF(cycle,meilleur_cycle,nb_villes,distances);
					cycle->taille--;
					cycle->poids -= poids;
				}
			}
		}
		else
		{
			double poids = distances[cycle->c[0]][cycle->c[cycle->taille-1]];
			cycle->poids += poids;
			if(meilleur_cycle->poids == 0 || cycle->poids < meilleur_cycle->poids)
			{
				meilleur_cycle->taille = cycle->taille;
				meilleur_cycle->poids = cycle->poids;
				printf("%d | %f\n",cycle->taille,cycle->poids);
				int i;
				for(i=0;i<cycle->taille;i++)
				{
					meilleur_cycle->c[i] = cycle->c[i];
				}
			}
			cycle->poids -= poids;
		}
	}
}

/**
 * Fonction main.
 */
int main (int argc, char *argv[])
{
  double **distances;
  double *abscisses;
  double *ordonnees;
  unsigned int nb_villes;
  
  //Initialisation du timer pour mesurer des temps (compiler avec -lrt) 
  struct timespec myTimerStart;
  clock_gettime(CLOCK_REALTIME, &myTimerStart);

  //Exemple de mesure du temps
  lire_donnees("defi250.csv", &nb_villes, &distances, &abscisses, &ordonnees);

	t_cycle meilleur_cycle;
	t_cycle cycle;
	meilleur_cycle.taille = cycle.taille = 0;
	meilleur_cycle.poids = cycle.poids = 0;
	PVC_EXACT_NAIF(&cycle,&meilleur_cycle,15,distances);

  //Récupération du timer et affichage
  struct timespec current;
  clock_gettime(CLOCK_REALTIME, &current); //Linux gettime
  double elapsed_in_ms =    (( current.tv_sec - myTimerStart.tv_sec) *1000 +
          ( current.tv_nsec - myTimerStart.tv_nsec)/1000000.0);
  printf("Temps passe (ms) : %lf\n", elapsed_in_ms);


	printf("%d\n",meilleur_cycle.taille);

  //Affichage des distances
  //afficher_distances(nb_villes,distances);

  //naif
  afficher_cycle_html(meilleur_cycle, abscisses, ordonnees);
  
  double ** Aretes =  trier_aretes(nb_villes, distances);
  /// <-- Kruskal Here
  supprimer_aretes(nb_villes, Aretes);

  supprimer_distances_et_coordonnees(nb_villes, distances, abscisses, ordonnees);
  return 0;
}
