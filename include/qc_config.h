/**
 * @file qc_config.h
 * @brief Définitions des structures de configuration et des codes d'erreur
 *        pour le programme de contrôle de qualité météo.
*/

#ifndef QC_CONFIG_H
#define QC_CONFIG_H

#include <stdbool.h>

#define MAX_CARACTERE 512

/**
 * @enum qc_error
 * @brief Codes d'erreur pour l'ensemble du programme
*/
typedef enum {
   OK_SUCCESS = 0,
   ERR_CONFIG_READ = 1,
   ERR_FILE_NOT_FOUND = 2,
   ERR_INVALID_FORMAT = 3,
   ERR_MALLOC_FAILED = 4,
   ERR_WRITE_FAILED = 5,
   ERR_SYNTAX_YAML = 6,
   ERR_ARGUMENT = 7
} qc_error;


/**
 * @struct config_input
 * @brief Chemins d'accès des fichiers d'entrée et bases de données
*/
struct config_input{
   char forecast_in[MAX_CARACTERE]; 
   char obs_db_path[MAX_CARACTERE];
   char predictors_base_dir[MAX_CARACTERE];
   char forecast_base_dir[MAX_CARACTERE];
};

/**
 * @struct config_validation
 * @brief Indicateurs d'activation des différents modules de validation
*/
struct config_validation {
   bool min_max_enabled;
   bool temporal_spike_detection_enabled;
   bool inter_variable_enabled;
};

/**
 * @struct config_max_min
 * @brief Seuils minimum et maximum pour une variable météo
*/
struct config_max_min {
   double min;
   double max;
};

/**
 * @struct config_tt_td
 * @brief Configuration des seuils min/max pour TT et TD
*/
struct config_tt_td {
   struct config_max_min tt; // elem_id 6 (temperature)
   struct config_max_min td; // elem_id 7 (point de rosée)
};

/**
 * @struct config_seuils
 * @brief Seuils adaptatifs de variation selon la distance temporelle
*/
struct config_seuils {
   double distance_1h;
   double distance_2h;
   double distance_3h_plus;
};

/**
 * @struct config_spike
 * @brief Paramètres pour la détection des spikes temporels
*/
struct config_spike {
   bool enabled;
   int max_distance;
   int file_interval;
   struct config_seuils adaptive_thresholds;
   bool use_predictors;
   bool use_observation_validation;
   bool use_previous_files;
};

/**
 * @struct config_inter_variable
 * @brief Paramètres pour la validation des valeur inter-variable.
*/
struct config_inter_variable {
   bool td_le_tt_enabled;
};

// Structure principale qui englobe tout
/**
 * @struct config
 * @brief Structure principale regroupant toute la configuration
*/
struct config {
   struct config_input input;
   struct config_validation validation;
   struct config_tt_td tt_td;
   struct config_spike temporal_spike;
   struct config_inter_variable inter_variable;
};


/**
 * Charge le fichier YAML et remplit la structure de configuration
 * @param chemin_fichier Le chemin vers le fichier config.yaml
 * @param config Un pointeur vers la structure à remplir
 * @return OK_SUCCESS en cas de succès ou un code d'erreur
*/
int charger_configuration(const char *chemin_fichier, struct config *config);

#endif
