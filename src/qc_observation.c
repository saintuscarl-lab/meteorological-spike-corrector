/**
 * @file qc_observation.c
 * @brief Permet un chargement de fichiers d'observations
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/qc_observation.h"
#include "../include/qc_config.h"

#define MAX_TAILLE_LIGNE        512


int lire_fichier_observ(const char *chemin_fichier, struct observ_noeud **sommet, int *lignes_lu) {
   
   if (lignes_lu != NULL) {
      *lignes_lu = 0;
   }
   
   FILE *fichier = fopen(chemin_fichier, "r");
   if (fichier == NULL) {
      fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier observations %s\n", chemin_fichier);
      return ERR_FILE_NOT_FOUND;
   }

   char tampon_ligne[MAX_TAILLE_LIGNE];
   struct donnee_observ donnee_temporaire;
   int compteur_lignes = 0;

   // On lit le fichier ligne par ligne jusqu'a la fin (NULL)
   while (fgets(tampon_ligne, sizeof(tampon_ligne), fichier) != NULL) {
      if ((tampon_ligne[0] == '-' && tampon_ligne[1] == '-') ||
         (tampon_ligne[0] >= 'a' && tampon_ligne[0] <= 'z')  || 
         (tampon_ligne[0] >= 'A' && tampon_ligne[0] <= 'Z')  || 
         tampon_ligne[0] == '\n' || tampon_ligne[0] == '\0'  || 
         tampon_ligne[0] == '\r'){
         continue;
      }
      
   // On decoupe la ligne en inspectant les 11 colonnes attendues
      int colonnes_extraites = sscanf(tampon_ligne, "%lld %lld %lf %d %d %lf %lf %d %d %d %d",
         &donnee_temporaire.obs_id,
         &donnee_temporaire.obs_dt,
         &donnee_temporaire.obs_date,
         &donnee_temporaire.level,
         &donnee_temporaire.obs_level,
         &donnee_temporaire.obs_val,
         &donnee_temporaire.obs_value,
         &donnee_temporaire.stn_id,
         &donnee_temporaire.loc_id,
         &donnee_temporaire.elem_id,
         &donnee_temporaire.qc_id);
         
      if (colonnes_extraites == NBR_COLONNES_OBSERV) {
         if (ajout_observ_chaine(sommet, donnee_temporaire) != OK_SUCCESS) {
            fprintf(stderr, "Erreur : Mémoire insuffisante lors du chargement.\n");
            fclose(fichier);
               
            liberer_liste_observ(*sommet);
            *sommet = NULL; 
         
            return ERR_MALLOC_FAILED; 
         }
         compteur_lignes++;         
      } 
   }
   fclose(fichier);
   
   if (lignes_lu != NULL) {
      *lignes_lu = compteur_lignes;
   }
   
   return OK_SUCCESS;
}
