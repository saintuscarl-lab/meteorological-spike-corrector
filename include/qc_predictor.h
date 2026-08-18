/**
 * @file qc_predictor.h
 * @brief Prototypes du module de chargement et d'extraction des données 
 *        à partir des fichiers de prédiction
*/
#ifndef QC_PREDICTOR_H
#define QC_PREDICTOR_H

#include "qc_timeseries.h"

#define NBR_COLONNES_PREDICT    6

/**
 * @brief Lit un fichier de prédicteur au format texte 
 * @param chemin_fichier Le chemin d'accès vers le fichier à lire
 * @param sommet Pointeur vers le pointeur de la tête de la liste chaînée
 * @param lignes_lu Pointeur vers l'entier qui recevra le nombre de lignes lues
 * @return OK_SUCCESS en cas de succès ou un code d'erreur
 */
int lire_fichier_predict(const char *chemin_fichier, struct predict_noeud **sommet, int *lignes_lu);

#endif 
