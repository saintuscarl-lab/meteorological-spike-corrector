/**
 * @file qc_spike_detector.h
 * @brief Prototypes et structures pour la détection et la correction des
 *        anomalies temporelles dans les prévisions
*/


#ifndef QC_SPIKE_DETECTOR_H
#define QC_SPIKE_DETECTOR_H

#include "qc_config.h"
#include "qc_timeseries.h"

// Structure pour regrouper les compteurs du rapport console
/**
 * @struct rapport_spike
 * @brief Structure regroupant les compteurs de rapport pour la console
 */
struct rapport_spike {
   int cas_1;
   int cas_2a;
   int cas_2b;
   int cas_2c;
   int cas_3;
   int corrections_predicteur;
   int corrections_observation;
};


/**
 * @brief Parcourt la liste des prévisions pour détecter et corriger les spikes
 *        selon les 5 scénarios de validation
 * @param sommet_previ Tête de la liste des prévisions
 * @param sommet_predict Tête de la liste des prédicteurs de secours
 * @param sommet_observ Tête de la liste des observations de validation
 * @param config Configuration globale de l'application
 * @param stats Structure de compteurs à incrémenter lors des détections
 * @return OK_SUCCESS en cas de succès ou un code d'erreur de énumération qc_error
 */
int detecter_spikes(struct previ_noeud *sommet_previ, struct predict_noeud *sommet_predict,
struct observ_noeud *sommet_observ, const struct config *config,
struct rapport_spike *stats);

#endif 