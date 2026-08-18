/**
 * @file qc_validation.h
 * @brief Prototypes des fonctions de validation des seuils min/max et
 *        de la cohérence inter-variable pour les prévisions.
*/

#ifndef QC_VALIDATION_H
#define QC_VALIDATION_H

#include "qc_config.h"
#include "qc_timeseries.h"

/**
 * @brief Valide les prévisions par rapport aux seuils min/max définis
 *        Les valeurs hors bornes sont converties en NAN
 * @param sommet Pointeur vers la tête de la liste chaînée des prévisions
 * @param config Pointeur vers la structure de configuration globale
 * @return Le nombre total de violations min/max détectées
*/
int validation_min_max(struct previ_noeud *sommet, const struct config *config);

/**
 * @brief Assure la cohérence inter-variable en vérifiant que TD <= TT
 *        Si TD > TT, TD est remplacé par la valeur de TT
 * @param sommet Pointeur vers la tête de la liste chaînée des prévisions
 * @param config Pointeur vers la structure de configuration globale
 * @return Le nombre de corrections inter-variables appliquées
*/
int validation_inter_variable(struct previ_noeud *sommet, const struct config *config);

#endif
