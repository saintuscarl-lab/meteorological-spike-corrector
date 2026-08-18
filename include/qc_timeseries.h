// qc_timeseries.h
// Modules pour structurer la liste chainée

#ifndef QC_TIMESERIES_H
#define QC_TIMESERIES_H

#define ELEM_ID_TT          6
#define ELEM_ID_TD          7


/**
 * @struct donnee_previ
 * @brief Structure contenant les informations brutes d'une prévision
 */
struct donnee_previ {
   int fcst_sys_id;
   int stn_id;
   int loc_id;
   int stat_method_id;
   int elem_id;
   double run_date;
   double lead_time;
   double prdn_value;
};

/**
 * @struct previ_noeud
 * @brief Nœud d'une liste simplement chaînée de prévisions
 */
struct previ_noeud{
   struct donnee_previ donnee;
   struct previ_noeud *suivant;
};

/**
 * @brief Alloue la mémoire et initialise un nouveau nœud de prévision
 * @param donnee Les données de prévision à insérer dans le nœud
 * @return Un pointeur vers le nouveau nœud ou NULL en cas d'échec
 */
struct previ_noeud* creation_previ_chaine(struct donnee_previ donnee);

/**
 * @brief Insère une prévision dans la liste chaînée, triée par lead_time
 * @param sommet Pointeur vers le pointeur de la tête de liste
 * @param donnee Les données de prévision à insérer
 * @return OK_SUCCESS en cas de succès, ou un code d'erreur
 */
int ajout_previ_chaine(struct previ_noeud **sommet, struct donnee_previ donnee);

/**
 * @brief Libère toute la mémoire allouée pour la liste de prévisions
 * @param sommet Le premier nœud de la liste à libérer
 */
void liberer_liste_previ(struct previ_noeud *sommet);

/**
 * @struct donnee_predict
 * @brief Structure contenant les informations brutes d'un prédicteur
 */
struct donnee_predict {
   int fcst_sys_id;
   int abacus_id;
   int loc_id;
   double run_date;
   double lead_time;
   double prdr_value;
};

/**
 * @struct predict_noeud
 * @brief Nœud d'une liste simplement chaînée de prédicteurs
 */
struct predict_noeud {
   struct donnee_predict donnee;
   struct predict_noeud *suivant;
};

/**
 * @brief Alloue la mémoire et initialise un nouveau nœud de prédicteur
 * @param donnee Les données de prédicteur à insérer dans le nœud
 * @return Un pointeur vers le nouveau nœud, ou NULL en cas d'échec
 */
struct predict_noeud* creation_predict_chaine(struct donnee_predict donnee);

/**
 * @brief Insère un prédicteur dans la liste chaînée, trié par lead_time
 * @param sommet Pointeur vers le pointeur de la tête de liste
 * @param donnee Les données de prédicteur à insérer
 * @return OK_SUCCESS en cas de succès ou un code d'erreur
 */
int ajout_predict_chaine(struct predict_noeud **sommet, struct donnee_predict donnee);

/**
 * @brief Libère toute la mémoire allouée pour la liste de prédicteurs
 * @param sommet Le premier nœud de la liste à libérer
 */
void liberer_liste_predict(struct predict_noeud *sommet);


/**
 * @struct donnee_observ
 * @brief Structure contenant les informations brutes d'une observation
 */
struct donnee_observ {
   long long obs_id;       
   long long obs_dt;       
   double obs_date;
   int level;
   int obs_level;
   double obs_val;
   double obs_value;
   int stn_id;
   int loc_id;
   int elem_id;
   int qc_id;
};

/**
 * @struct observ_noeud
 * @brief Nœud d'une liste simplement chaînée d'observations
 */
struct observ_noeud {
   struct donnee_observ donnee;
   struct observ_noeud *suivant;
};

/**
 * @brief Alloue la mémoire et initialise un nouveau nœud d'observation
 * @param donnee Les données d'observation à insérer dans le nœud
 * @return Un pointeur vers le nouveau nœud, ou NULL en cas d'échec
 */
struct observ_noeud* creation_observ_chaine(struct donnee_observ donnee);

/**
 * @brief Insère une observation de façon triée (stn_id -> elem_id -> obs_date)
 * @param sommet Pointeur vers le pointeur de la tête de liste
 * @param donnee Les données d'observation à insérer
 * @return OK_SUCCESS en cas de succès ou un code d'erreur
 */
int ajout_observ_chaine(struct observ_noeud **sommet, struct donnee_observ donnee);

/**
 * @brief Libère toute la mémoire allouée pour la liste d'observations.
 * @param sommet Le premier nœud de la liste à libérer.
 */
void liberer_liste_observ(struct observ_noeud *sommet);

#endif
