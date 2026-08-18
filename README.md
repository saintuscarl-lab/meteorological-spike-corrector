# Meteorological Data Quality Control & Spike Correction Engine (`weather_qc`)

Un pipeline modulaire en C99 d'assurance qualité et de traitement automatique de séries temporelles météorologiques. Le système ingère des prévisions fragmentées, identifie les anomalies physiques et corrige les artefacts temporels (*spikes*) via des seuils adaptatifs, des stations d'observation réelles et des prédicteurs.

![C Standard](https://img.shields.io/badge/C-C99-blue.svg)
![Coverage](https://img.shields.io/badge/Coverage-91.78%25-brightgreen.svg)
![Memory Safety](https://img.shields.io/badge/Valgrind-0%20Leaks-brightgreen.svg)
![Tests](https://img.shields.io/badge/Tests-CUnit%20%2B%20Bats%20Passing-success.svg)
![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)
![CI](https://github.com/saintuscarl@gmail.com/weather-qc-pipeline/actions/workflows/ci.yml/badge.svg)

---

## Problématique & architecture

Les données météorologiques brutes distribuées par blocs horaires de 6 heures présentent régulièrement des discontinuités ou des valeurs aberrantes (*spikes*) dues à des erreurs de transmission ou de capteurs.

Ce moteur analyse et assainit les séries temporelles via une chaîne de traitement complète :

- **Architecture 100 % dynamique :** Gestion rigoureuse de la mémoire sur le tas (*heap*) via des structures de données en listes simplement chaînées avec libération intégrale (zéro fuite mémoire sous Valgrind).
- **Modularité poussée :** Découpage strict en plus de 10 sous-systèmes indépendants (`src/` et `include/`) pour l'analyse syntaxique, le chargement, la recherche spatio-temporelle de voisins, le calcul d'anomalies et la sérialisation.
- **Configuration déclarative :** Intégration de `libyaml` pour l'ingestion dynamique des paramètres d'analyse, des chemins de fichiers et des seuils physiques.

---

## Moteur de détection & validation

### 1. Contrôles de cohérence physique

- **Validation Min/Max :** Élimination des températures (TT) et points de rosée (TD) hors des bornes physiques admises (ex. 203 K ≤ TT ≤ 333 K).
- **Validation inter-variable :** Maintien de la relation thermodynamique stricte TD ≤ TT pour chaque pas horaire et station.

### 2. Détection de spikes par seuils adaptatifs (5 cas)

L'algorithme adapte la tolérance d'écart thermique en fonction de la distance temporelle (ex. 30 K pour 1 h, 28 K pour 2 h, 27 K pour ≥ 3 h) :

- **Cas 1 (Deux voisins valides) :** Détection d'inflexion abrupte encadrée par deux pas temporels valides et correction par prédicteur horaire.
- **Cas 2a (Initialisation à T+00) :** Confrontation de la première prévision contre les stations d'observations réelles au sol.
- **Cas 2b (Raccordement de fichiers) :** Récupération de l'état terminal (T+06) du fichier de lot précédent pour évaluer le début du lot courant (T+07).
- **Cas 2c (Voisin unique & matrice de décision) :** Analyse différentielle croisée entre variation de prévision et variation de prédicteur pour distinguer un front météorologique réel d'un artefact de mesure.
- **Cas 3 (Valeur isolée) :** Remplacement automatique par prédicteur en l'absence de voisins dans le rayon temporel maximal.

---

## Structure du projet

```text
weather-qc-pipeline/
├── Makefile
├── README.md
├── config.yaml
├── include/
│   ├── qc_config.h
│   ├── qc_loader.h
│   ├── qc_neighbor_finder.h
│   ├── qc_observation.h
│   ├── qc_output.h
│   ├── qc_predictor.h
│   ├── qc_spike_detector.h
│   ├── qc_timeseries.h
│   └── qc_validation.h
├── src/
│   ├── weather_qc.c
│   ├── qc_config.c
│   ├── qc_loader.c
│   ├── qc_neighbor_finder.c
│   ├── qc_observation.c
│   ├── qc_output.c
│   ├── qc_predictor.c
│   ├── qc_spike_detector.c
│   ├── qc_timeseries.c
│   └── qc_validation.c
├── tests/
│   ├── unit/            # Suites de tests CUnit
│   └── bats/            # Tests d'intégration BATS
└── data/                # Échantillons de prévisions, prédicteurs et observations
```

---

## Compilation et exécution

### Prérequis

- Compilateur `gcc` ou `clang` supportant le standard C99.
- Bibliothèques de développement : `libyaml-dev`, `libcunit-dev`, `libm`.
- Outils d'assurance qualité : `bats-core`, `valgrind`, `gcov` / `lcov`.

```bash
# Compiler le binaire principal
make

# Exécuter le pipeline avec un fichier de configuration
./weather_qc -c config.yaml

# Exécuter en mode verbeux (traçabilité détaillée)
./weather_qc -c config.yaml -v
```

---

## Assurance qualité et métriques

Le projet applique une suite de tests à deux niveaux pour garantir l'intégrité des calculs météorologiques et la stabilité système :

```bash
# Exécution de l'ensemble des tests unitaires et fonctionnels
make test

# Profilage de mémoire avec Valgrind (détection de fuites)
make valgrind

# Génération du rapport de couverture de code
make couverture
```

### Résultats d'audit

| Contrôle | Résultat |
|---|---|
| Tests unitaires (CUnit) | 4 suites complètes, 21 assertions validées (0 régression) |
| Tests fonctionnels (BATS) | 17 scénarios d'intégration système validés |
| Couverture de code (gcov) | 91,78 % des branches et lignes testées |
| Intégrité mémoire (Valgrind) | 0 fuite mémoire (definitely/indirectly lost = 0 bytes) |

---

## Auteur

Carl Ed Saintus
