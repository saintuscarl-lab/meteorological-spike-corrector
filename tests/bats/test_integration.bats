#!/usr/bin/env bats

@test "Affichage console" {
   run ./weather_qc -c config.yaml
   [ "$status" -eq 0 ]
   
   [[ "$output" == *"WEATHER QC PROCESSING"* ]]
   [[ "$output" == *"Validation:"* ]]
   [[ "$output" == *"Spike detection (5 cases):"* ]]
   [[ "$output" == *"Corrections applied:"* ]]
}

@test "Validation du fichier corrige" { 
   run ./weather_qc -c config.yaml
   [ "$status" -eq 0 ]
   
   [ -f "data/output/2026032500_012.txt" ]
   run diff -iw data/output/2026032500_012.txt tests/fixtures/forecast_simple.txt
   [ "$status" -eq 0 ]
}

@test "Generateur des sorti NA" {
   mkdir -p data/forecasts_backup data/output_backup
   mv data/forecasts/* data/forecasts_backup/ 2>/dev/null || true
   mv data/output/* data/output_backup/ 2>/dev/null || true
   
   cp tests/fixtures/forecast_NA.txt data/forecasts/2026032500_012.txt

   run ./weather_qc -c config.yaml
   
   [ -f "data/output/2026032500_012.txt" ]
   run grep -q "NA" data/output/2026032500_012.txt
   [ "$status" -eq 0 ]

   rm -f data/output/*
   mv data/forecasts_backup/* data/forecasts/ 2>/dev/null || true
   mv data/output_backup/* data/output/ 2>/dev/null || true
   rm -rf data/forecasts_backup data/output_backup
}

@test "Dossier de sortie output est absent" {
   mv data/output data/output_backup
   
   run ./weather_qc -c config.yaml
   
   rm -rf data/output
   mv data/output_backup data/output
   [ "$status" -eq 5 ]
}

@test "Fuite de memoire (Valgrind)" {
   run valgrind --leak-check=yes --error-exitcode=1 ./weather_qc -c config.yaml
   [ "$status" -eq 0 ]
}