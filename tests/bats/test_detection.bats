#!/usr/bin/env bats

setup() {
   mkdir -p data/forecasts_backup data/output_backup
   cp -r data/forecasts/* data/forecasts_backup/ 2>/dev/null || true
   cp -r data/output/* data/output_backup/ 2>/dev/null || true
   rm -f data/forecasts/*
   rm -f data/output/*
}

teardown() {
   rm -rf data/forecasts/* data/output/*
   cp -r data/forecasts_backup/* data/forecasts/ 2>/dev/null || true
   cp -r data/output_backup/* data/output/ 2>/dev/null || true
   rm -rf data/forecasts_backup data/output_backup
}

@test "Détection CAS 1" {
   cp tests/fixtures/forecast_cas1.txt data/forecasts/2026032500_012.txt

   run ./weather_qc -c config.yaml

   [ "$status" -eq 0 ]
   [[ "$output" == *"Case 1 (2 neighbors): 1"* ]]
}

@test "Détection CAS 2a : T+00 avec observation" {
   cp tests/fixtures/forecast_cas2a.txt data/forecasts/2026032500_006.txt

   cp config.yaml test_config_2a.yaml
   sed -i 's/2026032500_012.txt/2026032500_006.txt/g' test_config_2a.yaml

   run ./weather_qc -c test_config_2a.yaml
   
   rm -f test_config_2a.yaml

   [ "$status" -eq 0 ]
   [[ "$output" == *"Case 2a (T+00): 1"* ]]
}

@test "Détection CAS 2c : Un seul voisin" {
   cp tests/fixtures/forecast_cas2c.txt data/forecasts/2026032500_012.txt

   run ./weather_qc -c config.yaml

   [ "$status" -eq 0 ]
   [[ "$output" == *"Case 2c (1 neighbor): 2"* ]]
}

@test "Détection CAS 3" {
   cp tests/fixtures/forecast_cas3.txt data/forecasts/2026032500_012.txt

   run ./weather_qc -c config.yaml

   [ "$status" -eq 0 ]
   [[ "$output" == *"Case 3 (no neighbor): 1"* ]]
}

@test "Détection CAS 2b" {
   cp tests/fixtures/forecast_cas2bp.txt data/forecasts/2026032500_006.txt
   cp tests/fixtures/forecast_cas2bc.txt data/forecasts/2026032500_012.txt

   run ./weather_qc -c config.yaml

   [ "$status" -eq 0 ]
   [[ "$output" == *"Case 2b (prev file): 1"* ]]
}
