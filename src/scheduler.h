#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "reseau.h"
#include "trame.h"

/* ─────────────────────────────────────────────
   Événement : une trame qui arrive chez un nœud
   ───────────────────────────────────────────── */

typedef struct {
    Trame trame;        /* La trame transportée              */
    int   expediteur;   /* Indice du nœud qui envoie         */
    int   destinataire; /* Indice du nœud qui reçoit         */
    int   port_arrivee; /* Port d'entrée chez le destinataire*/
} Evenement;

/* ─────────────────────────────────────────────
   File d'événements (FIFO circulaire)
   ───────────────────────────────────────────── */

#define FILE_MAX 512

typedef struct {
    Evenement evenements[FILE_MAX]; /* Tableau des événements en attente */
    int       tete;                 /* Indice du prochain à traiter      */
    int       queue;                /* Indice où écrire le prochain      */
    int       taille;               /* Nombre d'événements en attente    */
} FileEvenements;

/* ── Fonctions de la file ── */

void file_init(FileEvenements *f);
void enfiler(FileEvenements *f, Evenement ev);
Evenement defiler(FileEvenements *f);
int  file_est_vide(FileEvenements *f);

/* ── Simulation ── */

/* Crée le premier événement : st_src envoie une trame à st_dst */
void lancer_envoi(Reseau *r, FileEvenements *f,
                  int idx_src, int idx_dst,
                  uint16_t type, uint8_t *donnees, int taille);

/* Traite un seul événement et peut en ajouter de nouveaux dans la file */
void traiter_evenement(Reseau *r, Evenement *ev, FileEvenements *f);

/* Boucle principale : traite tous les événements un par un */
void simuler(Reseau *r, FileEvenements *f);

#endif /* SCHEDULER_H */