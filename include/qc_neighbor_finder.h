/**
 * @file qc_neighbor_finder.h
 * @brief Prototypes du module de recherche des voisins temporels pour les
 *        séries chronologiques de prévisions.
 */

#ifndef QC_NEIGHBOR_FINDER_H
#define QC_NEIGHBOR_FINDER_H

#include "qc_config.h"
#include "qc_timeseries.h"

/**
 * @brief Recherche les voisins précédent et suivant les plus proches d'une
 *        prévision cible dans un rayon temporel maximal
 * @param sommet Pointeur vers la tête de la liste chaînée des prévisions
 * @param cible Pointeur vers le nœud de la prévision à analyser
 * @param config Pointeur vers la configuration globale 
 * @param precedent Pointeur de sortie pour recevoir le voisin précédent trouvé
 * @param suivant Pointeur de sortie pour recevoir le voisin suivant trouvé
 */
void recherche_voisins(struct previ_noeud *sommet, 
   struct previ_noeud *cible, 
   const struct config *config,
   struct previ_noeud **precedent, 
   struct previ_noeud **suivant);

#endif
