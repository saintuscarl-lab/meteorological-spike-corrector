setup() {
   if [ ! -f "./weather_qc" ]; then
      make > /dev/null
   fi
}

@test "Sans aucun argument" {
   run ./weather_qc
   [ "$status" -ne 0 ]
   [[ "$output" =~ "Usage:" ]]
}

@test "Option -c sans fichier" {
   run ./weather_qc -c
   [ "$status" -ne 0 ]
}

@test "Argument inconnue" {
   run ./weather_qc -c config.yaml -x
   [ "$status" -ne 0 ]
   [[ "$output" =~ "Usage:" ]]
}

@test "Forme longue" {
   run ./weather_qc --cfg config.yaml
   [ "$status" -eq 0 ]
}

@test "Mode Verbeux" {
   run ./weather_qc -c config.yaml -v
   [ "$status" -eq 0 ]
   [[ "$output" =~ "WEATHER QC PROCESSING" ]]
   [[ "$output" =~ "Spike detection" ]]
}

@test "Fichier config YAML introuvable" {
   run ./weather_qc -c fichier_config.yaml
   [ "$status" -ne 0 ]
}

@test "Nom de fichier de prevision invalide" {
   # Génération d'une structure YAML imbriquée conforme attendue par le parser
   cat << EOF > config_broken_name.yaml
forecast:
  input:
    forecast_in: "data/forecasts/fichier_meteo_brise.txt"
EOF

   run ./weather_qc -c config_broken_name.yaml
   rm -f config_broken_name.yaml
   [ "$status" -eq 3 ]
}