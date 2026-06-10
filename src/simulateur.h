#ifndef SIMULATEUR_H
#define SIMULATEUR_H

#include "reseau.h"
#include "trame.h"

/* =========================================================
   TYPES D'EVENEMENTS
   ========================================================= */
typedef enum {
    EVT_ENVOYER_TRAME,
    EVT_RECEVOIR_TRAME
} TypeEvenement;

/* =========================================================
   EVENEMENT
   ========================================================= */
typedef struct {
    int           temps;
    TypeEvenement type;
    int           noeud_src;
    int           noeud_dst;
    Trame         trame;
} Evenement;

/* =========================================================
   FILE D'EVENEMENTS
   ========================================================= */
#define MAX_EVENEMENTS 1024

typedef struct {
    Evenement evenements[MAX_EVENEMENTS];
    int       nb_evenements;
} FileEvenements;

/* =========================================================
   PROTOTYPES
   ========================================================= */

void      init_file(FileEvenements *file);
void      ajouter_evenement(FileEvenements *file, Evenement evt);
Evenement creer_evt_envoi(int temps, int src, int dst, Trame trame);

/* limite_evenements : 0 = pas de limite */
void lancer_simulation(Reseau *r, FileEvenements *file, int limite_evenements);

#endif /* SIMULATEUR_H */
