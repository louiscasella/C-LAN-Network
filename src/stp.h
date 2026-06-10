#ifndef STP_H
#define STP_H

#include "reseau.h"
#include "simulateur.h"

/* Remplit la file avec les BPDUs initiaux de tous les switchs */
void demarrer_stp(Reseau *r, FileEvenements *file);

/* Traite un BPDU recu. Appele depuis simulateur.c */
void traiter_bpdu(Reseau *r, FileEvenements *file,
                  int idx_switch, int port_arrivee,
                  BPDU bpdu, int temps_actuel);

/* A appeler apres lancer_simulation() pour bloquer les ports redondants */
void finaliser_stp(Reseau *r);

#endif /* STP_H */