#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/qc_output.h"
#include "../include/qc_config.h"

int fichier_corriger(const char *chemin_fichier, struct previ_noeud *sommet) {
   FILE *fichier = fopen(chemin_fichier, "w");
   if (fichier == NULL) {
      fprintf(stderr, "Erreur : Impossible de creer le fichier de sortie %s\n", chemin_fichier);
      return ERR_WRITE_FAILED;
   }

   // Écriture des en-têtes textuels pour respecter scrupuleusement le format d'origine
   fprintf(fichier, "fcst_sys_id  stn_id  loc_id  stat_method_id  elem_id  run_date   lead_time  prdn_value\n");

   struct previ_noeud *actuel = sommet;

   // Parcours la liste pour re-ecrire les 8 colonnes de prevision
   while (actuel != NULL) {
      
      // revision sur le format d'alignement des colonnes pour match le fichier d'origine  
      fprintf(fichier, "%-13d%-8d%-8d%-16d%-9d%-11.1lf %-11.1lf ",
      actuel->donnee.fcst_sys_id,
      actuel->donnee.stn_id,
      actuel->donnee.loc_id,
      actuel->donnee.stat_method_id,
      actuel->donnee.elem_id,
      actuel->donnee.run_date,
      actuel->donnee.lead_time);

      // Traitement special pour la 8e colonne (prdn_value)
      if (isnan(actuel->donnee.prdn_value)) {
        fprintf(fichier, "NA\n"); // Traduction du NAN en texte "NA"
      } else {
        fprintf(fichier, "%.4lf\n", actuel->donnee.prdn_value);
      }

      actuel = actuel->suivant; // Avancement au noeud suivant
   }

   fclose(fichier);
   return OK_SUCCESS; 
}