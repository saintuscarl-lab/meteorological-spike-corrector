#ifndef QC_OUTPUT_H
#define QC_OUTPUT_H

#include "qc_timeseries.h"

/**
 * Ecrire la liste des previsions corrigees dans le fichier de sortie
 * Remplace les valeurs NAN par la chaine NA.
 * @return OK_SUCCESS (0) en cas de succès, ou ERR_WRITE_FAILED en cas d'erreur.
 */
int fichier_corriger(const char *chemin_fichier, struct previ_noeud *sommet);

#endif
