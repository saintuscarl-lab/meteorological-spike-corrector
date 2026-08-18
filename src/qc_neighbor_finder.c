/**
 * @file qc_neighbor_finder.c
 * @brief Implémentation de l'algorithme de recherche de voisins temporels
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/qc_neighbor_finder.h"

void recherche_voisins(struct previ_noeud *sommet, struct previ_noeud *cible, 
   const struct config *config, struct previ_noeud **precedent, 
   struct previ_noeud **suivant) {
   
   // On commence par vider les pointeurs de sortie
   *precedent = NULL;
   *suivant = NULL;

   int max_dist = config->temporal_spike.max_distance;
   struct previ_noeud *actuel = sommet;

   while (actuel != NULL) {
      // Vérifier si le nœud actuel appartient à la même station et au même élément
      // s'il ne s'agit pas de la cible elle-même et si la valeur est valide
      if (actuel->donnee.stn_id == cible->donnee.stn_id &&
         actuel->donnee.elem_id == cible->donnee.elem_id &&
         actuel != cible && !isnan(actuel->donnee.prdn_value)) { 

         double distance_tempo = actuel->donnee.lead_time - cible->donnee.lead_time;
         
      // C'est un voisin precedent potentiel 
         if(distance_tempo < 0){
            if(fabs(distance_tempo)<=max_dist){
               if(*precedent == NULL || actuel->donnee.lead_time > (*precedent)->donnee.lead_time){
                  *precedent = actuel;
               }
            }
         }

         // C'est un voisin suivant potentiel
         if(distance_tempo > 0){
            if(fabs(distance_tempo) <= max_dist){
               if(*suivant == NULL || actuel->donnee.lead_time < (*suivant)->donnee.lead_time){
                  *suivant = actuel;
               }
            }
         }
      }
      actuel = actuel->suivant;
   }
}
