/**
 * @file test_config.c
 * @brief Tests unitaires du module de configuration YAML
 */

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../../include/qc_config.h"

/**
 * @brief Test 1: Chargement du fichier config.yaml par défaut
 * Le fichier doit exister à la racine et retourner OK_SUCCESS
 */
void test_charger_config_valide(void) {
   struct config config;
   int status = charger_configuration("config.yaml", &config);
   
   // Le chargement doit renvoyer le code de succès
   CU_ASSERT_EQUAL(status, OK_SUCCESS);
   
   // Validation de l'extraction de paramètres clés du fichier d'origine
   CU_ASSERT_TRUE(config.validation.temporal_spike_detection_enabled);
   CU_ASSERT_TRUE(config.validation.min_max_enabled);
   CU_ASSERT_EQUAL(config.temporal_spike.max_distance, 3);
}

/**
 * @brief Test 2: Tentative de chargement d'un fichier inexistant
 * La fonction doit intercepter l'erreur et retourner ERR_FILE_NOT_FOUND
 */
void test_charger_config_inexistant(void) {
   struct config config;
   int status = charger_configuration("fichier_fantome_introuvable.yaml", &config);
   
   // Le statut de retour doit correspondre au code d'erreur
   CU_ASSERT_EQUAL(status, ERR_FILE_NOT_FOUND);
}

/**
 * @brief Test 3 : Fichier YAML avec une syntaxe corrompue
 */
void test_charger_config_syntaxe_invalide(void) {
   const char *nom_fichier_brise = "config_syntaxe_invalide.yaml";
   
   FILE *fichier = fopen(nom_fichier_brise, "w");
   CU_ASSERT_PTR_NOT_NULL(fichier);
   
   if (fichier != NULL) {
      fprintf(fichier, "forecast:\n  input:\n    [un_tableau_mal_ferme\n");
      fclose(fichier);
   }

   struct config config;
   int status = charger_configuration(nom_fichier_brise, &config);
   
   //  le module doit renvoyer ERR_SYNTAX_YAML
   CU_ASSERT_EQUAL(status, ERR_SYNTAX_YAML);

   remove(nom_fichier_brise);
}

int main(void) {

   if (CUE_SUCCESS != CU_initialize_registry()) {
      return CU_get_error();
   }

   CU_pSuite pSuite = CU_add_suite("Suite_Configuration", NULL, NULL);

   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }
   if ((NULL == CU_add_test(pSuite, "test_charger_config_valide", test_charger_config_valide)) ||
       (NULL == CU_add_test(pSuite, "test_charger_config_inexistant", test_charger_config_inexistant))||
       (NULL == CU_add_test(pSuite, "test_charger_config_syntaxe_invalide", test_charger_config_syntaxe_invalide))) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   
   CU_cleanup_registry();
   return CU_get_error();
}