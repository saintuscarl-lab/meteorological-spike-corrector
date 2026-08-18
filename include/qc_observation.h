/**
 * @file qc_observation
 * @brief Prototypes du module de chargement et d'extraction des données 
 *        à partir des fichiers d'observation
*/
#ifndef QC_OBSERVATION_H
#define QC_OBSERVATION_H

#include "qc_timeseries.h"

#define NBR_COLONNES_OBSERV     11

/**
 * @brief Lit un fichier d'observation au format texte 
 * @param chemin_fichier Le chemin d'accès vers le fichier à lire
 * @param sommet Pointeur vers le pointeur de la tête de la liste chaînée
 * @param lignes_lu Pointeur vers l'entier qui recevra le nombre de lignes lues
 * @return OK_SUCCESS en cas de succès, ou un code d'erreur de l'énumération qc_error
 */
int lire_fichier_observ(const char *chemin_fichier, struct observ_noeud **sommet, int *lignes_lu);

#endif 
