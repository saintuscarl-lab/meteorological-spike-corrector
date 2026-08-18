/**
 * @file test_spike_detection.c
 * @brief Tests unitaires du module de détection et correction des spikes temporels
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "../../include/qc_config.h"
#include "../../include/qc_timeseries.h"
#include "../../include/qc_spike_detector.h"

#ifndef OK_SUCCESS
#define OK_SUCCESS 0
#endif

/**
 * @brief Test 1 : Lorsque le module est désactivé dans la configuration,
 *        aucune détection ne doit avoir lieu et les compteurs doivent rester à 0
*/
void test_module_desactive(void) {
   struct config config;
   struct rapport_spike stats = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);
   config.validation.temporal_spike_detection_enabled = false;
   config.temporal_spike.enabled = false;

   int status = detecter_spikes(NULL, NULL, NULL, &config, &stats);

   CU_ASSERT_EQUAL(status, OK_SUCCESS);
   CU_ASSERT_EQUAL(stats.cas_1, 0);
   CU_ASSERT_EQUAL(stats.cas_3, 0);
}

/**
 * @brief Test 2 : CAS 3 où un point n'a aucun voisin temporel 
 *        valide à proximité 
*/
void test_detection_cas_3(void) {
   struct config config;
   struct rapport_spike stats = {0};
   struct previ_noeud noeud_previ = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);
   config.temporal_spike.use_predictors = false; 
   config.temporal_spike.use_observation_validation = false;
   strcpy(config.input.forecast_in, "data/forecasts/2026032500_006.txt");

   noeud_previ.donnee.elem_id = ELEM_ID_TT;
   noeud_previ.donnee.lead_time = 2.0;
   noeud_previ.donnee.prdn_value = 285.0;
   noeud_previ.suivant = NULL;

   int status = detecter_spikes(&noeud_previ, NULL, NULL, &config, &stats);

   CU_ASSERT_EQUAL(status, OK_SUCCESS);
   CU_ASSERT_EQUAL(stats.cas_3, 1);
}

/**
 * @brief Test 3: CAS 2a où un point initial est validé et corrigé
 *        directement par rapport à une observation correspondante
*/
void test_detection_cas_2a(void) {
   struct config config;
   struct rapport_spike stats = {0};
   struct previ_noeud noeud_previ = {0};
   struct observ_noeud noeud_obs = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);
   config.temporal_spike.use_observation_validation = true;
   config.temporal_spike.adaptive_thresholds.distance_1h = 5.0; 
   strcpy(config.input.forecast_in, "data/forecasts/2026032500_006.txt");

   int stn = 1440;
   double date = 2461119.0;

   noeud_previ.donnee.stn_id = stn;
   noeud_previ.donnee.elem_id = ELEM_ID_TT;
   noeud_previ.donnee.run_date = date;
   noeud_previ.donnee.lead_time = 0.0;
   noeud_previ.donnee.prdn_value = 330.0;
   noeud_previ.suivant = NULL;

   noeud_obs.donnee.stn_id = stn;
   noeud_obs.donnee.elem_id = ELEM_ID_TT;
   noeud_obs.donnee.obs_date = date;
   noeud_obs.donnee.obs_value = 265.0;
   noeud_obs.suivant = NULL;

   int status = detecter_spikes(&noeud_previ, NULL, &noeud_obs, &config, &stats);

   CU_ASSERT_EQUAL(status, OK_SUCCESS);
   CU_ASSERT_EQUAL(stats.cas_2a, 1);
   CU_ASSERT_EQUAL(stats.corrections_observation, 1);
   CU_ASSERT_DOUBLE_EQUAL(noeud_previ.donnee.prdn_value, 265.0, 0.001);
}

/**
 * @brief Test 4 : CAS 1 où un point est entouré de deux voisins temporels valides.
*/
void test_detection_cas_1(void) {
   struct config config;
   struct rapport_spike stats = {0};
   struct previ_noeud prec = {0}, cible = {0}, suiv = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);
   config.temporal_spike.adaptive_thresholds.distance_1h = 5.0; 
   config.temporal_spike.use_predictors = false;
   strcpy(config.input.forecast_in, "data/forecasts/2026032500_012.txt");

   int stn = 1440; 
   int elem = ELEM_ID_TT; 
   double date = 2461119.0;
   
   prec.donnee.stn_id = stn; 
   prec.donnee.elem_id = elem; 
   prec.donnee.run_date = date;
   prec.donnee.lead_time = 10.0; 
   prec.donnee.prdn_value = 275.0; 
   prec.suivant = &cible;

   cible.donnee.stn_id = stn; 
   cible.donnee.elem_id = elem; 
   cible.donnee.run_date = date;
   cible.donnee.lead_time = 11.0; 
   cible.donnee.prdn_value = 340.0; 
   cible.suivant = &suiv; 

   suiv.donnee.stn_id = stn; 
   suiv.donnee.elem_id = elem; 
   suiv.donnee.run_date = date;
   suiv.donnee.lead_time = 12.0; 
   suiv.donnee.prdn_value = 276.0; 
   suiv.suivant = NULL;

   int status = detecter_spikes(&prec, NULL, NULL, &config, &stats);

   CU_ASSERT_EQUAL(status, OK_SUCCESS);
   CU_ASSERT_EQUAL(stats.cas_1, 1); 
}

/**
 * @brief Test 5 : CAS 2b où un point situé sur la frontière inférieure d'un fichier
 *        dont le voisin précédent provient de l'historique du fichier précédent 
*/
void test_detection_cas_2b(void) {
   struct config config;
   struct rapport_spike stats = {0};
   struct previ_noeud prec = {0}, cible = {0}, suiv = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);
   
   config.temporal_spike.adaptive_thresholds.distance_1h = 5.0;
   config.temporal_spike.adaptive_thresholds.distance_2h = 10.0; 
   config.temporal_spike.adaptive_thresholds.distance_3h_plus = 5.0; 
   config.temporal_spike.use_predictors = false;
   strcpy(config.input.forecast_in, "data/forecasts/2026032500_012.txt");

   int stn = 1704; 
   int elem = ELEM_ID_TT; 
   double date = 2461124.5;

   // Point du fichier précédent
   prec.donnee.stn_id = stn; 
   prec.donnee.elem_id = elem; 
   prec.donnee.run_date = date;
   prec.donnee.lead_time = 6.0; 
   prec.donnee.prdn_value = 271.0; 
   prec.suivant = &cible;

   // Point cible du fichier courant
   cible.donnee.stn_id = stn; 
   cible.donnee.elem_id = elem; 
   cible.donnee.run_date = date;
   cible.donnee.lead_time = 7.0; 
   cible.donnee.prdn_value = 330.0; 
   cible.suivant = &suiv;

   // Point suivant du fichier courant
   suiv.donnee.stn_id = stn; 
   suiv.donnee.elem_id = elem; 
   suiv.donnee.run_date = date;
   suiv.donnee.lead_time = 8.0; 
   suiv.donnee.prdn_value = 272.0; 
   suiv.suivant = NULL;

   int status = detecter_spikes(&prec, NULL, NULL, &config, &stats);

   CU_ASSERT_EQUAL(status, OK_SUCCESS);

   /* 
    * Regarde si Le code applique le seuil forcé strict de distance_3h_plus 
    * ce qui valide l'existence d'un spike isolé entre differents fichier (CAS 2b).
    */
   CU_ASSERT_EQUAL(stats.cas_2b, 1);
}

/**
 * @brief Test 6 : CAS 2c où un point n'a qu'un seul voisin valide
 */
void test_detection_cas_2c(void) {
   struct config config;
   struct rapport_spike stats = {0};
   struct previ_noeud base = {0}, prec = {0}, cible = {0};
   
   // CORRECTIF 1 : Utilisation du type exact struct predict_noeud défini dans le projet
   struct predict_noeud pred_prec = {0}, pred_cible = {0};

   CU_ASSERT_EQUAL(charger_configuration("config.yaml", &config), OK_SUCCESS);
   config.temporal_spike.adaptive_thresholds.distance_1h = 5.0;
   config.temporal_spike.adaptive_thresholds.distance_2h = 10.0;
   config.temporal_spike.adaptive_thresholds.distance_3h_plus = 5.0;
   config.temporal_spike.use_predictors = true; 
   strcpy(config.input.forecast_in, "data/forecasts/2026032500_006.txt");

   int stn = 1440; 
   int elem = ELEM_ID_TT; 
   double date = 2461119.0;

   // Point d'ancrage initial
   base.donnee.stn_id = stn; 
   base.donnee.elem_id = elem; 
   base.donnee.run_date = date;
   base.donnee.lead_time = 4.0; 
   base.donnee.prdn_value = 270.0; 
   base.suivant = &prec;

   // Voisin stable (T+5)
   prec.donnee.stn_id = stn; 
   prec.donnee.elem_id = elem; 
   prec.donnee.run_date = date;
   prec.donnee.lead_time = 5.0; 
   prec.donnee.prdn_value = 271.0; 
   prec.suivant = &cible;

   // Point final (T+6) -> N'a qu'un seul voisin (T+5) et s'en écarte énormemet (Spike de 69K)
   cible.donnee.stn_id = stn; 
   cible.donnee.elem_id = elem; 
   cible.donnee.run_date = date;
   cible.donnee.lead_time = 6.0; 
   cible.donnee.prdn_value = 340.0; 
   cible.suivant = NULL;

   // Configuration du prédicteur pour le point précédent (T+5)
   pred_prec.donnee.abacus_id = 6; /* Équivalent à ELEM_ID_TT dans la table des abaques */
   pred_prec.donnee.lead_time = 5.0;
   pred_prec.donnee.prdr_value = 271.0;
   pred_prec.suivant = &pred_cible;

   // Configuration du prédicteur pour le point cible (T+6)
   pred_cible.donnee.abacus_id = 6; 
   pred_cible.donnee.lead_time = 6.0;
   pred_cible.donnee.prdr_value = 272.0; 
   pred_cible.suivant = NULL;

   int status = detecter_spikes(&base, &pred_prec, NULL, &config, &stats);

	/* 
    * CAS 2c (1 seul voisin) :
    * - Écart réel énorme (69K) alors que le modèle prévoit une météo stable (1K).
    * - Logique XOR validée : Le modèle ne confirme pas cette rupture brutale.
    * - Conclusion : C'est un spike artificiel confirmé.
   */
   CU_ASSERT_EQUAL(status, OK_SUCCESS);
   CU_ASSERT_EQUAL(stats.cas_2c, 1);
}

int main(void) {
   if (CUE_SUCCESS != CU_initialize_registry()) {
      return CU_get_error();
   }

   CU_pSuite pSuite = CU_add_suite("Suite_Spike_Detection", NULL, NULL);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   if ((NULL == CU_add_test(pSuite, "test_module_desactive", test_module_desactive)) ||
       (NULL == CU_add_test(pSuite, "test_detection_cas_3", test_detection_cas_3)) ||
       (NULL == CU_add_test(pSuite, "test_detection_cas_2a", test_detection_cas_2a)) ||
       (NULL == CU_add_test(pSuite, "test_detection_cas_1", test_detection_cas_1)) ||
       (NULL == CU_add_test(pSuite, "test_detection_cas_2b", test_detection_cas_2b)) ||
       (NULL == CU_add_test(pSuite, "test_detection_cas_2c", test_detection_cas_2c))) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}