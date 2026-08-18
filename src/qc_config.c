/**
 * @file qc_config.c
 * @brief Implémentation du parser de configuration basé sur libyaml.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <yaml.h>
#include "../include/qc_config.h"

#define MAX_NOM_CHAR       64
#define RESET              0


/**
 * @brief Convertit une chaîne de caractères en booléen
 * @param valeur Chaîne "true" ou "false"
 * @return true si la chaîne vaut "true", false sinon
*/
static bool en_booleen(const char *valeur) {
   return strcasecmp(valeur, "true") == 0;
}

/**
 * @brief Effectue une copie  d'une chaîne avec une taille maximale
 * @param destination Espace mémoire cible
 * @param source Chaîne d'origine à copier
*/
static void copie_chaine(char *destination, const char *source) {
   strncpy(destination, source, MAX_CARACTERE - 1);
   destination[MAX_CARACTERE - 1] = '\0';
}

/**
 * @brief Extrait et enregistre les chemins de la section input
 * @param input Pointeur vers la sous-structure des entrées
 * @param cle Nom du paramètre YAML
 * @param valeur Valeur textuelle associée au paramètre
*/
static void traiter_section_input(struct config_input *input, const char *cle, 
   const char *valeur) {
  if (strcmp(cle, "forecast_in") == 0) {
      copie_chaine(input->forecast_in, valeur);
   } else if (strcmp(cle, "obs_db_path") == 0) {
      copie_chaine(input->obs_db_path, valeur);
   } else if (strcmp(cle, "predictors_base_dir") == 0) {
      copie_chaine(input->predictors_base_dir, valeur);
   } else if (strcmp(cle, "forecast_base_dir") == 0) {
      copie_chaine(input->forecast_base_dir, valeur);
   }
}

/**
 * @brief Extrait et enregistre les activations de la section validation
 * @param validation Pointeur vers la sous-structure de validation
 * @param cle Nom du paramètre YAML
 * @param valeur Valeur textuelle associée au paramètre
*/
static void traiter_section_validation(struct config_validation *validation, 
   const char *cle, const char *valeur) {
   bool val_bool = en_booleen(valeur);
   if (strcmp(cle, "min_max_enabled") == 0) {
      validation->min_max_enabled = val_bool;
   } else if (strcmp(cle, "temporal_spike_detection_enabled") == 0) {
      validation->temporal_spike_detection_enabled = val_bool;
   } else if (strcmp(cle, "inter_variable_enabled") == 0) {
      validation->inter_variable_enabled = val_bool;
   }
}

/**
 * @brief Extrait et enregistre les bornes min/max de la section min_max
 * @param tt_td Pointeur vers la sous-structure des bornes thermiques
 * @param sous_section Nom de la variable ciblée (TT ou TD)
 * @param cle Nom de la borne (min ou max)
 * @param valeur Valeur numérique textuelle
*/
static void traiter_section_min_max(struct config_tt_td *tt_td, const char 
   *sous_section, const char *cle, const char *valeur) {
   struct config_max_min *cible = NULL;

   // Détermine quelle sous-structure on doit modifier
   if (strcmp(sous_section, "TT") == 0) {
      cible = &tt_td->tt;
   } else if (strcmp(sous_section, "TD") == 0) {
      cible = &tt_td->td;
   }

   // Applique la logique de façon centralisée si la sous-section est valide
   if (cible != NULL) {
      double val_double = atof(valeur);
      if (strcmp(cle, "min") == 0) {
         cible->min = val_double;
      } else if (strcmp(cle, "max") == 0) {
         cible->max = val_double;
      }
   }
}

/**
 * @brief Extrait et enregistre les paramètres de la section temporal_spike
 * @param spike Pointeur vers la sous-structure de gestion des spikes.
 * @param sous_section Nom de la sous-section interne ("adaptive_thresholds" ou vide).
 * @param cle Nom du paramètre YAML.
 * @param valeur Valeur textuelle associée au paramètre.
*/
static void traiter_section_spike(struct config_spike *spike, const char 
   *sous_section, const char *cle, const char *valeur) {
   if (strcmp(sous_section, "adaptive_thresholds") == 0) {
      double val_double = atof(valeur);
      if (strcmp(cle, "distance_1h") == 0) {
         spike->adaptive_thresholds.distance_1h = val_double;
      }
      else if (strcmp(cle, "distance_2h") == 0) {
         spike->adaptive_thresholds.distance_2h = val_double;
      }
      else if (strcmp(cle, "distance_3h_plus") == 0) {
         spike->adaptive_thresholds.distance_3h_plus = val_double;
      }
   } else {
      if (strcmp(cle, "enabled") == 0) {
         spike->enabled = en_booleen(valeur);
      }
      else if (strcmp(cle, "max_distance") == 0) {
         spike->max_distance = atoi(valeur);
      }
      else if (strcmp(cle, "file_interval") == 0) {
         spike->file_interval = atoi(valeur);
      }
      else if (strcmp(cle, "use_predictors") == 0) {
         spike->use_predictors = en_booleen(valeur);
      }
      else if (strcmp(cle, "use_observation_validation") == 0) {
         spike->use_observation_validation = en_booleen(valeur);
      }
      else if (strcmp(cle, "use_previous_files") == 0) {
         spike->use_previous_files = en_booleen(valeur);
      }
   }
}



int charger_configuration(const char *chemin_fichier, struct config *config) {
   memset(config, 0, sizeof(struct config));
   
   // Lecture du fichier
   FILE *fichier = fopen(chemin_fichier, "r");
   if(fichier == NULL) {
      fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", chemin_fichier);
      return ERR_FILE_NOT_FOUND;
   }

   // Initialisation des outils de libyaml
   yaml_parser_t lecture;
   yaml_event_t element_en_lecture;

   if(!yaml_parser_initialize(&lecture)) {
      fprintf(stderr, "Erreur : Impossible d'initialiser le parser YAML\n");
      fclose(fichier);
      return ERR_SYNTAX_YAML;
   }

   yaml_parser_set_input_file(&lecture, fichier); // analyse de fichier

   // Variables d'état pour savoir où l'on se trouve dans le fichier
   char section_actuelle[MAX_NOM_CHAR] = "";
   char sous_section_actuelle[MAX_NOM_CHAR] = "";
   char cle_actuelle[MAX_NOM_CHAR] = "";
   bool fin_text = false; // Indique si on a atteint la fin du fichier
   bool est_une_cle = true;
   int statut = OK_SUCCESS;
  

   // La boucle s'exécute tant que la fin du fichier n'est pas atteinte.
   while (!fin_text) {
      // Vérification de la syntaxe du fichier YAML et lecture de l'élement suivant
      if (!yaml_parser_parse(&lecture, &element_en_lecture)) {
         fprintf(stderr, "Erreur de syntaxe dans le fichier YAML\n");
         statut = ERR_SYNTAX_YAML;
         fin_text = true; 
         break; 

      // Vérification si on a atteint la fin du fichier
      } else if (element_en_lecture.type == YAML_STREAM_END_EVENT) {
         fin_text = true;

      }
      // Pour savoir si on est dans une sous section 
      else if (element_en_lecture.type == YAML_MAPPING_START_EVENT) {
         if (strlen(cle_actuelle) > 0) {
            // Si c'est la racine "forecast", on la vide simplement pour ne pas décaler le reste
            if (strcmp(cle_actuelle, "forecast") == 0) {
               cle_actuelle[RESET] = '\0';
            }
            else if (strlen(section_actuelle) == 0) {
               strncpy(section_actuelle, cle_actuelle, MAX_NOM_CHAR - 1);
               section_actuelle[MAX_NOM_CHAR - 1] = '\0';
               cle_actuelle[RESET] = '\0';
            } else {
               strncpy(sous_section_actuelle, cle_actuelle, MAX_NOM_CHAR - 1);
               sous_section_actuelle[MAX_NOM_CHAR - 1] = '\0';
               cle_actuelle[RESET] = '\0';
            }
         }
         est_une_cle = true;

      // Sortie d'une sous section ou d'une section
      }else if (element_en_lecture.type == YAML_MAPPING_END_EVENT) {
         if (strlen(sous_section_actuelle) > 0) {
            sous_section_actuelle[RESET] = '\0';
         } else if (strlen(section_actuelle) > 0) {
            section_actuelle[RESET] = '\0';
         }
         est_une_cle = true;
      }
      // analyse de l'element recu
      else if (element_en_lecture.type == YAML_SCALAR_EVENT) {
         char *valeur = (char *)element_en_lecture.data.scalar.value;
         if (est_une_cle) {
            strncpy(cle_actuelle, valeur, MAX_NOM_CHAR - 1);
            cle_actuelle[MAX_NOM_CHAR - 1] = '\0';
            est_une_cle = false; // Le prochain scalaire sera la valeur de cette clé
         } else {
            if (strcmp(section_actuelle, "input") == 0) {
               traiter_section_input(&config->input, cle_actuelle, valeur);
            } 
            else if (strcmp(section_actuelle, "validation") == 0) {
               traiter_section_validation(&config->validation, cle_actuelle, valeur);
            } 
            else if (strcmp(section_actuelle, "min_max") == 0) {
               traiter_section_min_max(&config->tt_td, sous_section_actuelle, cle_actuelle, valeur);
            } 
            else if (strcmp(section_actuelle, "temporal_spike") == 0) {
               traiter_section_spike(&config->temporal_spike, sous_section_actuelle, cle_actuelle, valeur);
            } 
            else if (strcmp(section_actuelle, "inter_variable") == 0) {
               if (strcmp(cle_actuelle, "td_le_tt_enabled") == 0) {
                  config->inter_variable.td_le_tt_enabled = en_booleen(valeur);
               }
            }
            cle_actuelle[RESET] = '\0';
            est_une_cle = true; // La prochaine donnée sera une nouvelle clé
         }
      }
            
      // Nettoyage de la memoire de l'evenement actuel avant le prochain tour
      yaml_event_delete(&element_en_lecture);
   }

   // Nettoyage final
   yaml_parser_delete(&lecture);
   fclose(fichier);
   return statut;
}
