#ifndef CONFIG_H
#define CONFIG_H

#include "reseau.h"

/* Charge un réseau depuis un fichier .lan
   Retourne 1 si succès, 0 si erreur. */
int charger_reseau(const char *nom_fichier, Reseau *r);

#endif /* CONFIG_H */
