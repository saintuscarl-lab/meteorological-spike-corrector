/**
 * @file test_neighbor_finder.c
 * @brief Tests unitaires du module de recherche de voisins temporels avec CUnit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../../include/qc_config.h"
#include "../../include/qc_timeseries.h"
#include "../../include/qc_neighbor_finder.h"

/**
 * @brief Test 1 : une liste au milieu d'une série continue.
 *        Doit trouver le voisin précédent et le voisin suivant.
 */
void test_recherche_voisins_milieu(void) {
   struct config config;
   struct previ_noeud liste1 = {0}, liste2 = {0}, liste3 = {0};
   struct previ_noeud *v_prec = NULL, *v_suiv = NULL;

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   int stn = 1440; 
   int elem = ELEM_ID_TT; 
   double date = 2461119.0;

   // Configuration des liste
   liste1.donnee.stn_id = stn; 
	liste1.donnee.elem_id = elem; 
	liste1.donnee.run_date = date; 
	liste1.donnee.lead_time = 10.0; 
	liste1.donnee.prdn_value = 273.0; 
	liste1.suivant = &liste2;

   liste2.donnee.stn_id = stn; 
	liste2.donnee.elem_id = elem; 
	liste2.donnee.run_date = date; 
	liste2.donnee.lead_time = 11.0; 
	liste2.donnee.prdn_value = 274.0; 
	liste2.suivant = &liste3;
   
	liste3.donnee.stn_id = stn; 
	liste3.donnee.elem_id = elem; 
	liste3.donnee.run_date = date; 
	liste3.donnee.lead_time = 12.0; 
	liste3.donnee.prdn_value = 275.0; 
	liste3.suivant = NULL;

   recherche_voisins(&liste1, &liste2, &config, &v_prec, &v_suiv);

   CU_ASSERT_PTR_EQUAL(v_prec, &liste1);
   CU_ASSERT_PTR_EQUAL(v_suiv, &liste3);
}

/**
 * @brief Test 2 : Premier point de la série.
 *        Le voisin précédent doit être NULL.
 */
void test_recherche_voisins_premier_point(void) {
   struct config config;
   struct previ_noeud liste1 = {0}, liste2 = {0};
   struct previ_noeud *v_prec = NULL, *v_suiv = NULL;

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   int stn = 1440; 
   int elem = ELEM_ID_TT; 
   double date = 2461119.0;

   // Configuration des liste
   liste1.donnee.stn_id = stn; 
	liste1.donnee.elem_id = elem; 
	liste1.donnee.run_date = date; 
	liste1.donnee.lead_time = 10.0; 
	liste1.donnee.prdn_value = 273.0; 
	liste1.suivant = &liste2;
   
	liste2.donnee.stn_id = stn; 
	liste2.donnee.elem_id = elem; 
	liste2.donnee.run_date = date; 
	liste2.donnee.lead_time = 11.0; 
	liste2.donnee.prdn_value = 274.0; 
	liste2.suivant = NULL;

   recherche_voisins(&liste1, &liste1, &config, &v_prec, &v_suiv);
   
   CU_ASSERT_PTR_EQUAL(v_prec, NULL);
   CU_ASSERT_PTR_EQUAL(v_suiv, &liste2);
}

/**
 * @brief Test 3: Dernier point de la série, le voisin 
 * suivant doit être NULL
 */
void test_recherche_voisins_dernier_point(void) {
   struct config config;
   struct previ_noeud liste1 = {0}, liste2 = {0};
   struct previ_noeud *v_prec = NULL, *v_suiv = NULL;

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   int stn = 1440; 
   int elem = ELEM_ID_TT; 
   double date = 2461119.0;

   // Configuration des liste
   liste1.donnee.stn_id = stn; 
	liste1.donnee.elem_id = elem; 
	liste1.donnee.run_date = date; 
	liste1.donnee.lead_time = 10.0; 
	liste1.donnee.prdn_value = 273.0; 
	liste1.suivant = &liste2;

   liste2.donnee.stn_id = stn; 
	liste2.donnee.elem_id = elem; 
	liste2.donnee.run_date = date; 
	liste2.donnee.lead_time = 11.0; 
	liste2.donnee.prdn_value = 274.0;
	liste2.suivant = NULL;

   recherche_voisins(&liste1, &liste2, &config, &v_prec, &v_suiv);

   CU_ASSERT_PTR_EQUAL(v_prec, &liste1);
   CU_ASSERT_PTR_EQUAL(v_suiv, NULL);
}

/**
 * @brief Test 4: Distance temporelle supérieure à max_distance 
 *        Le point trop éloigné doit être ignoré et renvoie NULL
 */
void test_recherche_voisins_distance_trop_grande(void) {
   struct config config;
   struct previ_noeud liste1 = {0}, liste2 = {0};
   struct previ_noeud *v_prec = NULL, *v_suiv = NULL;

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   int stn = 1440; 
   int elem = ELEM_ID_TT; 
   double date = 2461119.0;

   // Écart de 5 heures entre les deux prévisions 
   liste1.donnee.stn_id = stn; 
	liste1.donnee.elem_id = elem; 
	liste1.donnee.run_date = date; 
	liste1.donnee.lead_time = 10.0; 
	liste1.donnee.prdn_value = 273.0; 
	liste1.suivant = &liste2;

   liste2.donnee.stn_id = stn; 
	liste2.donnee.elem_id = elem; 
	liste2.donnee.run_date = date; 
	liste2.donnee.lead_time = 15.0; 
	liste2.donnee.prdn_value = 274.0; 
	liste2.suivant = NULL;

   recherche_voisins(&liste1, &liste2, &config, &v_prec, &v_suiv);

   // La distance (15 - 10 = 5) > 3, liste1 ne peut pas être un voisin valide de liste2
   CU_ASSERT_PTR_EQUAL(v_prec, NULL);
   CU_ASSERT_PTR_EQUAL(v_suiv, NULL);
}

/**
 * @brief Test 5: Sauter les points invalidés autrement dit doit 
 * ignorer le voisin direct s'il est NAN et lier le point valide suivant
 */
void test_recherche_voisins_valeur_nan(void) {
   struct config config;
   struct previ_noeud liste1 = {0}, liste2 = {0}, liste3 = {0};
   struct previ_noeud *v_prec = NULL, *v_suiv = NULL;

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);

   int stn = 1440; 
   int elem = ELEM_ID_TT; 
   double date = 2461119.0;

   liste1.donnee.stn_id = stn; 
	liste1.donnee.elem_id = elem; 
	liste1.donnee.run_date = date; 
	liste1.donnee.lead_time = 9.0;  
	liste1.donnee.prdn_value = 273.0; 
	liste1.suivant = &liste2;

   liste2.donnee.stn_id = stn; 
	liste2.donnee.elem_id = elem;
	liste2.donnee.run_date = date; 
	liste2.donnee.lead_time = 10.0; 
	liste2.donnee.prdn_value = NAN;   
	liste2.suivant = &liste3;

   liste3.donnee.stn_id = stn; 
	liste3.donnee.elem_id = elem; 
	liste3.donnee.run_date = date; 
	liste3.donnee.lead_time = 11.0; 
	liste3.donnee.prdn_value = 275.0; 
	liste3.suivant = NULL;

   recherche_voisins(&liste1, &liste3, &config, &v_prec, &v_suiv);
	// Doit sauter liste2 puisque NAN et attraper liste1 
   CU_ASSERT_PTR_EQUAL(v_prec, &liste1);
   CU_ASSERT_PTR_EQUAL(v_suiv, NULL);
}

int main(void) {
   if (CUE_SUCCESS != CU_initialize_registry()) {
      return CU_get_error();
   }

   CU_pSuite pSuite = CU_add_suite("Suite_Neighbor_Finder", NULL, NULL);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if ((NULL == CU_add_test(pSuite, "test_recherche_voisins_milieu", 
		test_recherche_voisins_milieu)) ||
      (NULL == CU_add_test(pSuite, "test_recherche_voisins_premier_point", 
		test_recherche_voisins_premier_point)) ||
      (NULL == CU_add_test(pSuite, "test_recherche_voisins_dernier_point", 
		test_recherche_voisins_dernier_point)) ||
      (NULL == CU_add_test(pSuite, "test_recherche_voisins_distance_trop_grande", 
		test_recherche_voisins_distance_trop_grande)) ||
      (NULL == CU_add_test(pSuite, "test_recherche_voisins_valeur_nan", 
		test_recherche_voisins_valeur_nan))) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}