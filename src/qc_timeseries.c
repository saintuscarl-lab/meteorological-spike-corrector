#include <stdio.h>
#include <stdlib.h>
#include "../include/qc_timeseries.h"
#include "../include/qc_config.h"

/*
*                    Module prévisions
*/ 

struct previ_noeud* creation_previ_chaine(struct donnee_previ donnee){
   struct previ_noeud *nouveau_noeud = malloc(sizeof(struct previ_noeud));
   if(nouveau_noeud == NULL) {
         fprintf(stderr, "Erreur d'allocation de mémoire\n");
         return NULL;
   }
   nouveau_noeud->donnee = donnee;
   nouveau_noeud->suivant = NULL;
   return nouveau_noeud;
}

int ajout_previ_chaine(struct previ_noeud **sommet, struct donnee_previ donnee) {
   struct previ_noeud *nouveau_noeud = creation_previ_chaine(donnee);
   if (nouveau_noeud == NULL){
      return ERR_MALLOC_FAILED; 
   }
   if (*sommet == NULL || (*sommet)->donnee.lead_time > donnee.lead_time) {
         nouveau_noeud->suivant = *sommet;
      *sommet = nouveau_noeud;
      return OK_SUCCESS;
   }
   struct previ_noeud *actuel = *sommet;
   while (actuel->suivant != NULL && actuel->suivant->donnee.lead_time <= donnee.lead_time) {
      actuel = actuel->suivant;
   }

   nouveau_noeud->suivant = actuel->suivant;
   actuel->suivant = nouveau_noeud;
   return OK_SUCCESS;
}

void liberer_liste_previ(struct previ_noeud *sommet) {
   struct previ_noeud *temp; 
   while (sommet != NULL) {
         temp = sommet;
         sommet = sommet->suivant;
         free(temp);
   }
}

/*
*                    Module Prédicateur
*/ 
struct predict_noeud* creation_predict_chaine(struct donnee_predict donnee){
   struct predict_noeud *nouveau_noeud = malloc(sizeof(struct predict_noeud));
   if(nouveau_noeud == NULL) {
         fprintf(stderr, "Erreur d'allocation de mémoire\n");
         return NULL;
   }
   nouveau_noeud->donnee = donnee;
   nouveau_noeud->suivant = NULL;
   return nouveau_noeud;
}

int ajout_predict_chaine(struct predict_noeud **sommet, struct donnee_predict donnee) {
   struct predict_noeud *nouveau_noeud = creation_predict_chaine(donnee);
   
   if (nouveau_noeud == NULL){
      return ERR_MALLOC_FAILED;
   } 

   // La liste vide ou le lead_time est plus petit que la tête
   if (*sommet == NULL || (*sommet)->donnee.lead_time > donnee.lead_time) {
      nouveau_noeud->suivant = *sommet;
      *sommet = nouveau_noeud;
      return OK_SUCCESS;
   }
   struct predict_noeud *actuel = *sommet;
   while (actuel->suivant != NULL && actuel->suivant->donnee.lead_time <= donnee.lead_time) {
      actuel = actuel->suivant;
   }

   nouveau_noeud->suivant = actuel->suivant;
   actuel->suivant = nouveau_noeud;
   return OK_SUCCESS;
}

void liberer_liste_predict(struct predict_noeud *sommet) {
   struct predict_noeud *temp; 
   while (sommet != NULL) {
         temp = sommet;
         sommet = sommet->suivant;
         free(temp);
   }
}


/*
*                    Module observations
*/ 

/**
 * @brief Aide A déterminer l'ordre de tri des differents observations en
 *        Compare par stn_id, puis elem_id, et enfin obs_date.
 * @return 1 si 'courante' doit être placée APRÈS 'nouvelle', 0 sinon.
 */
static int aide_trie_obs(struct donnee_observ courante, struct donnee_observ nouvelle) {
   if (courante.stn_id != nouvelle.stn_id) {
      return courante.stn_id > nouvelle.stn_id;
   }
   if (courante.elem_id != nouvelle.elem_id) {
      return courante.elem_id > nouvelle.elem_id;
   }
   return courante.obs_date > nouvelle.obs_date;
}

struct observ_noeud* creation_observ_chaine(struct donnee_observ donnee){
   struct observ_noeud *nouveau_noeud = malloc(sizeof(struct observ_noeud));
   if(nouveau_noeud == NULL) {
         fprintf(stderr, "Erreur d'allocation de mémoire\n");
         return NULL;
    }
    
   nouveau_noeud->donnee = donnee;
   nouveau_noeud->suivant = NULL;
   return nouveau_noeud;
}

int ajout_observ_chaine(struct observ_noeud **sommet, struct donnee_observ donnee){
   struct observ_noeud *nouveau_noeud = creation_observ_chaine(donnee);
   if (nouveau_noeud == NULL){
      return ERR_MALLOC_FAILED;
   }

   // Insertion en tête si la liste est vide ou si le tri l'impose
   if (*sommet == NULL || aide_trie_obs((*sommet)->donnee, donnee)) {
      nouveau_noeud->suivant = *sommet;
      *sommet = nouveau_noeud;
      return OK_SUCCESS;
   }
   struct observ_noeud *actuel = *sommet;
   while (actuel->suivant != NULL && !aide_trie_obs(actuel->suivant->donnee, donnee)) {
      actuel = actuel->suivant;
   }

   nouveau_noeud->suivant = actuel->suivant;
   actuel->suivant = nouveau_noeud;
   return OK_SUCCESS;
}

void liberer_liste_observ(struct observ_noeud *sommet) {
   struct observ_noeud *temp; 
   while (sommet != NULL) {
      temp = sommet;
      sommet = sommet->suivant;
      free(temp);
   }
}