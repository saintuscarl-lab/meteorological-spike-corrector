/**
 * @file test_validation.c
 * @brief Tests unitaires du module de contrôle de qualité et de validation (Min/Max et Inter-variable).
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../../include/qc_config.h"
#include "../../include/qc_timeseries.h"
#include "../../include/qc_validation.h" 

/**
 * @brief Test 1 : Une valeur de température dans les bornes
 *        ne doit pas être modifiée ni comptabilisée comme violation
 */
void test_validation_valeur_valide(void) {
   struct config config;
   struct previ_noeud noeud = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   noeud.donnee.elem_id = ELEM_ID_TT; 
   noeud.donnee.prdn_value = 280.0;
   noeud.suivant = NULL;

   int violations = validation_min_max(&noeud, &config);

   CU_ASSERT_EQUAL(violations, 0);
   CU_ASSERT_FALSE(isnan(noeud.donnee.prdn_value));
   CU_ASSERT_DOUBLE_EQUAL(noeud.donnee.prdn_value, 280.0, 0.001);
}

/**
 * @brief Test 2 : Une valeur supérieure à la borne maximale
 *        doit être convertie en NAN et incrémenter le compteur de violations
 */
void test_validation_valeur_trop_elevee(void) {
   struct config config;
   struct previ_noeud noeud = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   noeud.donnee.elem_id = ELEM_ID_TT;
   noeud.donnee.prdn_value = 350.0;
   noeud.suivant = NULL;

   int violations = validation_min_max(&noeud, &config);

   CU_ASSERT_EQUAL(violations, 1);
   CU_ASSERT_TRUE(isnan(noeud.donnee.prdn_value));
}

/**
 * @brief Test 3 : Une valeur inférieure à la borne minimale
 *        doit être convertie en NAN et incrémenter le compteur de violations
 */
void test_validation_valeur_trop_basse(void) {
   struct config config;
   struct previ_noeud noeud = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   noeud.donnee.elem_id = ELEM_ID_TT;
   noeud.donnee.prdn_value = 150.0;
   noeud.suivant = NULL;

   int violations = validation_min_max(&noeud, &config);

   CU_ASSERT_EQUAL(violations, 1);
   CU_ASSERT_TRUE(isnan(noeud.donnee.prdn_value));
}

/**
 * @brief Test 4: Une valeur de TD supérieure à sa borne max
 *        doit être mise à NAN et comptée comme violation
 */
void test_validation_td_trop_elevee(void) {
   struct config config;
   struct previ_noeud noeud = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   noeud.donnee.elem_id = ELEM_ID_TD;
   noeud.donnee.prdn_value = 320.0; 
   noeud.suivant = NULL;

   int violations = validation_min_max(&noeud, &config);

   CU_ASSERT_EQUAL(violations, 1);
   CU_ASSERT_TRUE(isnan(noeud.donnee.prdn_value));
}

/**
 * @brief Test Cas 5: Force la désactivation des flags de validation ainsi
 *        Les fonctions doivent retourner 0 sans modifier les données.
 */
void test_validation_desactivee(void) {
   struct config config;
   struct previ_noeud noeud = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   config.validation.min_max_enabled = false;
   config.validation.inter_variable_enabled = false;

   // On place une valeur aberrante qui devrait normalement lever une alerte
   noeud.donnee.elem_id = ELEM_ID_TT;
   noeud.donnee.prdn_value = 350.0;
   noeud.suivant = NULL;

   int violations = validation_min_max(&noeud, &config);

   // Contrat : Retour immédiat à 0 et valeur intacte
   CU_ASSERT_EQUAL(violations, 0);
   CU_ASSERT_FALSE(isnan(noeud.donnee.prdn_value));
   CU_ASSERT_DOUBLE_EQUAL(noeud.donnee.prdn_value, 350.0, 0.001);
}

/**
 * @brief Test 6: Un point de rosée inférieur ou égal à la 
 * température ambiante est valide et ne change pas
 */
void test_validation_inter_variable_valide(void) {
   struct config config;
   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   struct previ_noeud noeud_tt = {0};
   struct previ_noeud noeud_td = {0};

   // Situation  TD (270K) <= TT (275K)
   noeud_td.donnee.elem_id = ELEM_ID_TD;
   noeud_td.donnee.stn_id = 1704;
   noeud_td.donnee.lead_time = 2.0;
   noeud_td.donnee.prdn_value = 270.0;
   noeud_td.suivant = &noeud_tt;

   noeud_tt.donnee.elem_id = ELEM_ID_TT;
   noeud_tt.donnee.stn_id = 1704;
   noeud_tt.donnee.lead_time = 2.0;
   noeud_tt.donnee.prdn_value = 275.0;
   noeud_tt.suivant = NULL;

   int corrections = validation_inter_variable(&noeud_td, &config);

   CU_ASSERT_EQUAL(corrections, 0);
   CU_ASSERT_DOUBLE_EQUAL(noeud_td.donnee.prdn_value, 270.0, 0.001);
}

/**
 * @brief Test 7: Si le point de rosée dépasse la température,
 *        il doit être rabaissé au niveau exact de la température (TT).
 */
void test_validation_inter_variable_invalide(void) {
   struct config config;
   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   struct previ_noeud noeud_tt = {0};
   struct previ_noeud noeud_td = {0};

   // Situation : TD (280K) > TT (272K)
   noeud_td.donnee.elem_id = ELEM_ID_TD;
   noeud_td.donnee.stn_id = 1704;
   noeud_td.donnee.lead_time = 5.0;
   noeud_td.donnee.prdn_value = 280.0;
   noeud_td.suivant = &noeud_tt;

   noeud_tt.donnee.elem_id = ELEM_ID_TT;
   noeud_tt.donnee.stn_id = 1704;
   noeud_tt.donnee.lead_time = 5.0;
   noeud_tt.donnee.prdn_value = 272.0;
   noeud_tt.suivant = NULL;

   int corrections = validation_inter_variable(&noeud_td, &config);

   // La fonction doit appliquer une correction et égaliser les variables
   CU_ASSERT_EQUAL(corrections, 1);
   CU_ASSERT_DOUBLE_EQUAL(noeud_td.donnee.prdn_value, 272.0, 0.001);
}

int main(void) {
   if (CUE_SUCCESS != CU_initialize_registry()) {
      return CU_get_error();
   }

   CU_pSuite pSuite = CU_add_suite("Suite_Validation_Meteo", NULL, NULL);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

  if ((NULL == CU_add_test(pSuite, "test_validation_valeur_valide", 
      test_validation_valeur_valide)) ||
      (NULL == CU_add_test(pSuite, "test_validation_valeur_trop_elevee", 
      test_validation_valeur_trop_elevee)) ||
      (NULL == CU_add_test(pSuite, "test_validation_valeur_trop_basse", 
      test_validation_valeur_trop_basse)) ||
      (NULL == CU_add_test(pSuite, "test_validation_td_trop_elevee", 
      test_validation_td_trop_elevee)) ||
      (NULL == CU_add_test(pSuite, "test_validation_desactivee", 
      test_validation_desactivee)) ||
      (NULL == CU_add_test(pSuite, "test_validation_inter_variable_valide", 
      test_validation_inter_variable_valide)) ||
      (NULL == CU_add_test(pSuite, "test_validation_inter_variable_invalide", 
      test_validation_inter_variable_invalide))) {
      CU_cleanup_registry();
      return CU_get_error();
    }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}