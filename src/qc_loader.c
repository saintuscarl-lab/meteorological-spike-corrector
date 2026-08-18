/**
 * @file qc_loader.c
 * @brief Permet un chargement de fichiers prévisions
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/qc_loader.h"
#include "../include/qc_config.h"

#define MAX_TAILLE_LIGNE 512

int lire_fichier_previ(const char *chemin_fichier, struct previ_noeud **sommet, int *lignes_lu) {
   
   if (lignes_lu != NULL) {
      *lignes_lu = 0;
   }
   
   FILE *fichier = fopen(chemin_fichier, "r");
   if (fichier == NULL) {
      fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier forecast %s\n", chemin_fichier);
      return ERR_FILE_NOT_FOUND;
   }

   char tampon_ligne[MAX_TAILLE_LIGNE];
   struct donnee_previ donnee_temporaire;
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
     

      // On decoupe la ligne en inspectant les 8 colonnes attendues
      int colonnes_extraites = sscanf(tampon_ligne, "%d %d %d %d %d %lf %lf %lf",
         &donnee_temporaire.fcst_sys_id,
         &donnee_temporaire.stn_id,
         &donnee_temporaire.loc_id,
         &donnee_temporaire.stat_method_id,
         &donnee_temporaire.elem_id,
         &donnee_temporaire.run_date,
         &donnee_temporaire.lead_time,
         &donnee_temporaire.prdn_value);

      if (colonnes_extraites == NBR_COLONNES_PREVI) {
         if (ajout_previ_chaine(sommet, donnee_temporaire) != OK_SUCCESS) {
            fprintf(stderr, "Erreur : Mémoire insuffisante lors du chargement.\n");
            fclose(fichier);
               
            liberer_liste_previ(*sommet);
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

 
