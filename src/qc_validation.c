/**
 * @file qc_validation.c
 * @brief Implémentation des modules de contrôle de qualité et de validation.
*/

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../include/qc_validation.h"

#define EPSILON 1e-6

int validation_min_max(struct previ_noeud *sommet, const struct config *config) {
   
    // verifier si la validation min_max est activée dans la configuration
   if (!config->validation.min_max_enabled) {
      return 0;
   }

   struct previ_noeud *actuel = sommet;
   int violations_min_max = 0;
   

   while (actuel != NULL) {
      // Test pour la temperature (TT)
      if (actuel->donnee.elem_id == ELEM_ID_TT) {
         if (actuel->donnee.prdn_value < config->tt_td.tt.min || 
            actuel->donnee.prdn_value > config->tt_td.tt.max) {
            
            actuel->donnee.prdn_value = NAN; // On marque comme invalide (NA)
            violations_min_max++;
         }
      } 
      // Test pour le point de rosee (TD)
      else if (actuel->donnee.elem_id == ELEM_ID_TD) {
         if (actuel->donnee.prdn_value < config->tt_td.td.min || 
            actuel->donnee.prdn_value > config->tt_td.td.max) {
            
            actuel->donnee.prdn_value = NAN; // On marque comme invalide (NA)
            violations_min_max++;
         }
      }
      actuel = actuel->suivant;
   }
   return violations_min_max;
}

int validation_inter_variable(struct previ_noeud *sommet, const struct config *config) {
   // Vérifier si les drapeaux de validation inter-variable sont activés
   if (!config->validation.inter_variable_enabled || 
      !config->inter_variable.td_le_tt_enabled) {
      return 0;
   }

   struct previ_noeud *actuel = sommet;
  int violations_inter_variable = 0;

   while (actuel != NULL) {
      // On cherche un élément TD valide (non marqué NAN par la validation min_max)
      if (actuel->donnee.elem_id == ELEM_ID_TD && !isnan(actuel->donnee.prdn_value)) {
         
         // Parcourir la liste pour trouver le TT correspondant (même station, même lead_time)
         struct previ_noeud *cherche_tt = sommet;
         while (cherche_tt != NULL) {
            if (cherche_tt->donnee.elem_id == ELEM_ID_TT &&
               cherche_tt->donnee.stn_id == actuel->donnee.stn_id &&
               fabs(cherche_tt->donnee.lead_time - actuel->donnee.lead_time) < EPSILON &&
               !isnan(cherche_tt->donnee.prdn_value)) {
               
               // TD doit être <= TT
               if (actuel->donnee.prdn_value > cherche_tt->donnee.prdn_value) {
                  actuel->donnee.prdn_value = cherche_tt->donnee.prdn_value;
                  violations_inter_variable++;
               }
               break; 
            }
            cherche_tt = cherche_tt->suivant;
         }
      }
      actuel = actuel->suivant;
   }
   return violations_inter_variable;
}