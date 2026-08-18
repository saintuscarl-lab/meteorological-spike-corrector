/**
 * @file weather_qc.c
 * @brief Programme principal et orchestrateur du contrôle de qualité
 *        et de correction des prévisions météorologiques.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include "../include/qc_config.h"
#include "../include/qc_timeseries.h"
#include "../include/qc_loader.h"
#include "../include/qc_predictor.h"
#include "../include/qc_observation.h"
#include "../include/qc_validation.h"
#include "../include/qc_spike_detector.h"
#include "../include/qc_output.h"

#define MAX_NOM_CHEMIN    1024

/**
 * @brief Affiche les instructions d'utilisation du programme sur la sortie d'erreur
 * @param nom_programme Nom de l'exécutable.
 */
static void afficher_usage(const char *nom_programme) {
   fprintf(stderr, "Usage: %s {-c|--cfg} <config.yaml> [-v]\n\n", nom_programme);
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "  -c, --cfg <fichier>  Spécifier le fichier de configuration YAML\n");
   fprintf(stderr, "  -v                   Activer le mode verbeux\n");
}

/**
 * @brief Point d'entrée du programme weather_qc
 */
int main(int argc, char *argv[]) {
   char *chemin_config = NULL;
   bool mode_verbeux = false;

   // Configuration des options longues pour getopt_long
   static struct option options_longues[] = {
      {"cfg", required_argument, 0, 'c'},
      {0, 0, 0, 0}
   };

   int option;
   int indice_option = 0;

   // Analyse de la ligne de commande
   while ((option = getopt_long(argc, argv, "c:v", options_longues, &indice_option)) != -1) {
      switch (option) {
         case 'c':
            chemin_config = optarg;
            break;
         case 'v':
            mode_verbeux = true;
            break;
         default:
            afficher_usage(argv[0]);
            return ERR_ARGUMENT;
      }
   }

   // Validation de l'argument de configuration
   if (chemin_config == NULL) {
      fprintf(stderr, "Erreur : L'option -c ou --cfg est obligatoire.\n");
      afficher_usage(argv[0]);
      return ERR_ARGUMENT;
   }

   // Initialisation des structures et des compteurs
   struct config les_config;
   struct rapport_spike les_stats = {0, 0, 0, 0, 0, 0, 0};
   struct previ_noeud *liste_previ = NULL;
   struct predict_noeud *liste_predict = NULL;
   struct observ_noeud *liste_observ = NULL;

   int violations_min_max = 0;
   int violations_inter = 0;

   // Chargement de la configuration YAML
   int status_config = charger_configuration(chemin_config, &les_config);
   if (status_config != OK_SUCCESS) {
      fprintf(stderr, "Erreur lors du chargement de la configuration %s\n", chemin_config);
      return status_config;
   }

   // Extraction du nom du fichier pour construire le chemin de sortie
   char chemin_sortie[MAX_NOM_CHEMIN];
   const char *nom_fichier = strrchr(les_config.input.forecast_in, '/');
   if (nom_fichier == NULL) {
      nom_fichier = les_config.input.forecast_in;
   } else {
      nom_fichier++; 
   }

   // Assemblage du chemin de fichier de sortie cible
   snprintf(chemin_sortie, sizeof(chemin_sortie), "data/output/%s", nom_fichier);

   // Découpage des paramètres temporels à partir du nom standardisé du fichier
   int annee, mois, jour, heure, plage;
   if (sscanf(nom_fichier, "%4d%2d%2d%2d_%3d.txt", &annee, &mois, &jour, &heure, &plage) != 5) {
      fprintf(stderr, "Erreur : Format du nom de fichier de prévision invalide (%s)\n", nom_fichier);
      return ERR_INVALID_FORMAT;
   }

   // Calcul des bornes de lead_time pour le fichier en traitement
   int max_lead = plage;
   int min_lead = (max_lead == les_config.temporal_spike.file_interval) ? 0 : 
                  (max_lead - les_config.temporal_spike.file_interval + 1);

   // Gestion cas 2b : Chargement du fichier de prévisions précédent si nécessaire
   if (les_config.temporal_spike.enabled && les_config.temporal_spike.use_previous_files && min_lead > 0) {
      char chemin_prev_fcst[MAX_NOM_CHEMIN];
      size_t dossier_len = nom_fichier - les_config.input.forecast_in;

      strncpy(chemin_prev_fcst, les_config.input.forecast_in, dossier_len);
      chemin_prev_fcst[dossier_len] = '\0';

      snprintf(chemin_prev_fcst + dossier_len, sizeof(chemin_prev_fcst) - dossier_len,
         "%04d%02d%02d%02d_%03d.txt", annee, mois, jour, heure, max_lead - les_config.temporal_spike.file_interval);

      if (mode_verbeux) {
         printf("Chargement du fichier précédent pour CAS 2b : %s\n", chemin_prev_fcst);
      }
      // Passage de NULL pour le paramètre de comptage des lignes
      lire_fichier_previ(chemin_prev_fcst, &liste_previ, NULL); 
   }

   // Chargement du fichier de prévisions principal
   int res_lecture = lire_fichier_previ(les_config.input.forecast_in, &liste_previ, NULL);
   if (res_lecture != OK_SUCCESS) {
      liberer_liste_previ(liste_previ);
      return res_lecture;
   }

   // Gestion dynamique des chargement des prédicteurs et observations
   if (les_config.temporal_spike.enabled) {
      // Chargement de la plage horaire étendue des fichiers de prédicteurs
      int prem_prdr = (min_lead > 0) ? (min_lead - 1) : min_lead;
      char chemin_predict[MAX_NOM_CHEMIN];

      for (int h = prem_prdr; h <= max_lead; h++) {
         snprintf(chemin_predict, sizeof(chemin_predict), "%s%04d/%02d/%02d/%02d/db/hourly/%04d%02d%02d%02d_%03d-predictors.txt",
            les_config.input.predictors_base_dir, annee, mois, jour, heure, annee, mois, jour, heure, h);
         lire_fichier_predict(chemin_predict, &liste_predict, NULL);
      }

      // Chargement de la base de données des observations directes
      lire_fichier_observ(les_config.input.obs_db_path, &liste_observ, NULL);
   }

   // Exécution des modules de filtrage et de validation de base
   violations_min_max = validation_min_max(liste_previ, &les_config);
   violations_inter = validation_inter_variable(liste_previ, &les_config);

   // Exécution de l'algorithme d'élimination des spikes temporels
   int status_detection = detecter_spikes(liste_previ, liste_predict, liste_observ, &les_config, &les_stats);
   if (status_detection != OK_SUCCESS) {
      liberer_liste_previ(liste_previ);
      liberer_liste_predict(liste_predict);
      liberer_liste_observ(liste_observ);
      return status_detection;
   }

   // Écriture et sauvegarde des données corrigées dans le répertoire de sortie
   if (fichier_corriger(chemin_sortie, liste_previ) != OK_SUCCESS) {
      liberer_liste_previ(liste_previ);
      liberer_liste_predict(liste_predict);
      liberer_liste_observ(liste_observ);
      return ERR_WRITE_FAILED;
   }

   // Affichage final  sur la console 
   printf("WEATHER QC PROCESSING\n\n");
   printf("Config file: %s\n", chemin_config);
   printf("Input file: %s ", les_config.input.forecast_in);
   printf("Output file: %s ", chemin_sortie);
   printf("Processing forecasts...\n\n");

   printf("Validation:\n");
   printf("  Min/max violations: %d\n", violations_min_max);
   printf("  Inter-variable violations: %d\n\n", violations_inter);

   printf("Spike detection (5 cases):\n");
   printf("  Case 1 (2 neighbors): %d\n", les_stats.cas_1);
   printf("  Case 2a (T+00): %d\n", les_stats.cas_2a);
   printf("  Case 2b (prev file): %d\n", les_stats.cas_2b);
   printf("  Case 2c (1 neighbor): %d\n", les_stats.cas_2c);
   printf("  Case 3 (no neighbor): %d\n\n", les_stats.cas_3);

   printf("Corrections applied: %d\n", les_stats.corrections_predicteur + les_stats.corrections_observation);
   printf("  ✓ Corrected with predictor: %d\n", les_stats.corrections_predicteur);
   printf("  ✓ Corrected with observation: %d\n\n", les_stats.corrections_observation);

   printf("Output file: %s ", chemin_sortie);
   printf("Processing complete!\n");

   // Libération de la mémoire dynamique allouée
   liberer_liste_previ(liste_previ);
   liberer_liste_predict(liste_predict);
   liberer_liste_observ(liste_observ);

   return OK_SUCCESS;
}