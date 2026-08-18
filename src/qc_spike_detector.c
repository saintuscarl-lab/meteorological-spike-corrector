/**
 * @file qc_spike_detector.c
 * @brief Implémentation l'algorithme traitement des differencts cas de spikes 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/qc_spike_detector.h"
#include "../include/qc_neighbor_finder.h"
#include "../include/qc_config.h"

#define EPSILON 1e-6

/**
 * @brief Associe une distance temporelle à son seuil adaptatif
 */
static double seuil_correspondant(double distance, const struct config *config) {
   double dist_abs = fabs(distance);

   if (dist_abs <= 1.0 + EPSILON) {
      return config->temporal_spike.adaptive_thresholds.distance_1h; 
   } else if (dist_abs <= 2.0 + EPSILON) {
      return config->temporal_spike.adaptive_thresholds.distance_2h; 
   } else {
      return config->temporal_spike.adaptive_thresholds.distance_3h_plus; 
   }
}

/**
 * @brief Recherche une valeur d'observation correspondante dans la liste triée
 */
static double chercher_obs_value(struct observ_noeud *sommet, int stn_id, int elem_id, double run_date) {
   struct observ_noeud *actuel = sommet;

   while (actuel != NULL) {
      if (actuel->donnee.stn_id == stn_id &&
          actuel->donnee.elem_id == elem_id &&
          fabs(actuel->donnee.obs_date - run_date) < EPSILON) {
         return actuel->donnee.obs_value; 
      }
      actuel = actuel->suivant;
   }
   return NAN; 
}

/**
 * @brief Recherche une valeur de prédicteur de secours correspondante
 */
static double chercher_prdr_value(struct predict_noeud *sommet, int elem_id, int loc_id, double lead_time) {
   // determiner l'abacus_id requis : 6 pour TT, 152 pour TD
   int abacus_cible = (elem_id == ELEM_ID_TT) ? 6 : 152;
   struct predict_noeud *actuel = sommet;

   while (actuel != NULL) {
      if (actuel->donnee.abacus_id == abacus_cible &&
          actuel->donnee.loc_id == loc_id &&
          fabs(actuel->donnee.lead_time - lead_time) < EPSILON) {
         return actuel->donnee.prdr_value; 
      }
      actuel = actuel->suivant; 
   }
   return NAN; 
}

int detecter_spikes(struct previ_noeud *sommet_previ, 
   struct predict_noeud *sommet_predict, struct observ_noeud *sommet_observ, 
   const struct config *config, struct rapport_spike *stats) {
   
   if (!config->validation.temporal_spike_detection_enabled || !config->temporal_spike.enabled) {
      return OK_SUCCESS;
   }

   // Déterminer les bornes de lead_time du fichier courant à partir de son nom
   int max_lead = 0;
   int min_lead = 0;
   const char *nom_fichier = strrchr(config->input.forecast_in, '_');
   
   if (nom_fichier != NULL && strlen(nom_fichier) >= 4) {
      max_lead = atoi(nom_fichier + 1); 
      if (max_lead == config->temporal_spike.file_interval) {
         min_lead = 0;
      } else {
         min_lead = max_lead - config->temporal_spike.file_interval + 1;
      }
   } else {
      fprintf(stderr, "Erreur : Format de nom de fichier invalide.\n");
      return ERR_INVALID_FORMAT;
   }

   struct previ_noeud *cible = sommet_previ;

   while (cible != NULL) {
      // Filtrage par rapport à la plage horaire du fichier courant
      if (cible->donnee.lead_time < min_lead || cible->donnee.lead_time > max_lead) {
         cible = cible->suivant;
         continue;
      }
      
      // Sauter les données déjà invalidées par le filtre Min/Max
      if (isnan(cible->donnee.prdn_value)) {
         cible = cible->suivant; 
         continue;
      }

      bool noeud_traite_par_obs = false;

     
      // CAS 2a : T+00 avec observation disponible 
      if (fabs(cible->donnee.lead_time) < EPSILON && config->temporal_spike.use_observation_validation) {
         double val_obs = chercher_obs_value(sommet_observ, cible->donnee.stn_id, 
            cible->donnee.elem_id, cible->donnee.run_date);
         
         if (!isnan(val_obs)) {
            noeud_traite_par_obs = true; 
            double diff_obs = fabs(cible->donnee.prdn_value - val_obs);
            double threshold_obs = config->temporal_spike.adaptive_thresholds.distance_1h; 

            if (diff_obs >= threshold_obs) {
               stats->cas_2a++;
               cible->donnee.prdn_value = val_obs; 
               stats->corrections_observation++;
            }
         }
      }

      // Si le nœud est un T+00 validé par une observation, on passe au suivant
      if (noeud_traite_par_obs) {
         cible = cible->suivant;
         continue;
      }

      // Extraction des voisins pour les autres cas de figures
      struct previ_noeud *voisin_prec = NULL;
      struct previ_noeud *voisin_suiv = NULL;
      recherche_voisins(sommet_previ, cible, config, &voisin_prec, &voisin_suiv);

     
      // CAS 1 & CAS 2b : Deux voisins valides
      if (voisin_prec != NULL && voisin_suiv != NULL) {
         double diff_prev = fabs(cible->donnee.prdn_value - voisin_prec->donnee.prdn_value);
         double diff_next = fabs(cible->donnee.prdn_value - voisin_suiv->donnee.prdn_value);
         double diff_neighbors = fabs(voisin_prec->donnee.prdn_value - voisin_suiv->donnee.prdn_value);

         double dist_next = fabs(cible->donnee.lead_time - voisin_suiv->donnee.lead_time);
         double threshold_next = seuil_correspondant(dist_next, config);
         double threshold_prev;

         // CAS 2b : Si le voisin précédent vient du fichier précédent, 
         // forcer le seuil du palier 3h+
         if (config->temporal_spike.use_previous_files && voisin_prec->donnee.lead_time < min_lead) {
            threshold_prev = seuil_correspondant(3.0, config); 
         } else {
            double dist_prev = fabs(cible->donnee.lead_time - voisin_prec->donnee.lead_time);
            threshold_prev = seuil_correspondant(dist_prev, config);
         }
         
         double min_threshold = (threshold_prev < threshold_next) ? threshold_prev : threshold_next;
         double avg_diff = (diff_prev + diff_next) / 2.0;
         
         // Logique de decision 
         if (diff_neighbors < min_threshold && avg_diff >= (min_threshold * 0.85)) {
            // Si le voisin précédent est en dehors du fichier courant, c'est le cas 2b
            if (config->temporal_spike.use_previous_files && voisin_prec->donnee.lead_time < min_lead) {
               stats->cas_2b++;
            } else {
               stats->cas_1++;
            }

            if (config->temporal_spike.use_predictors) {
               double val_prdr = chercher_prdr_value(sommet_predict, 
                  cible->donnee.elem_id, cible->donnee.loc_id, cible->donnee.lead_time);
               
               if (!isnan(val_prdr)) {
                  cible->donnee.prdn_value = val_prdr;
                  stats->corrections_predicteur++;
               }
            }
         }
      }
      
      // CAS 2c : Un seul voisin valide 
      else if ((voisin_prec != NULL && voisin_suiv == NULL) || (voisin_prec == NULL && voisin_suiv != NULL)) {
         struct previ_noeud *voisin_unique = (voisin_prec != NULL) ? voisin_prec : voisin_suiv;
         double dist = fabs(cible->donnee.lead_time - voisin_unique->donnee.lead_time);
         double threshold = seuil_correspondant(dist, config);

         double diff_fcst = fabs(cible->donnee.prdn_value - voisin_unique->donnee.prdn_value);
         bool ecart_prev = (diff_fcst >= threshold);

         double prdr_cible = chercher_prdr_value(sommet_predict, cible->donnee.elem_id, 
            cible->donnee.loc_id, cible->donnee.lead_time);
         double prdr_voisin = chercher_prdr_value(sommet_predict, voisin_unique->donnee.elem_id, 
            voisin_unique->donnee.loc_id, voisin_unique->donnee.lead_time);

         if (!isnan(prdr_cible) && !isnan(prdr_voisin)) {
            double diff_prdr_calculee = fabs(prdr_cible - prdr_voisin);
            bool ecart_predict = (diff_prdr_calculee >= threshold);

            if (ecart_prev != ecart_predict) {
               stats->cas_2c++;
               if (config->temporal_spike.use_predictors) {
                  cible->donnee.prdn_value = prdr_cible; 
                  stats->corrections_predicteur++;
               }
            }
         }
      }
 

      // CAS 3 : Aucun voisin (Valeur isolée)
      else {
         stats->cas_3++;
         if (config->temporal_spike.use_predictors) {
            double val_prdr = chercher_prdr_value(sommet_predict, 
               cible->donnee.elem_id, cible->donnee.loc_id, cible->donnee.lead_time);
            if (!isnan(val_prdr)) {
               cible->donnee.prdn_value = val_prdr; 
               stats->corrections_predicteur++;
            }
         }
      }
  
      cible = cible->suivant;
   }
   return OK_SUCCESS;
}