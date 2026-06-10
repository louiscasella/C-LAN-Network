#ifndef SIMULATEUR_H
#define SIMULATEUR_H

#include "reseau.h"
#include "trame.h"

/* =========================================================
   BPDU - message echange entre switchs pour STP
   ========================================================= */
typedef struct {
    AdresseMAC mac_racine;    /* MAC du switch qu'on pense etre la racine */
    int        prio_racine;   /* priorite de cette racine                 */
    int        cout_racine;   /* cout accumule depuis la racine           */
    AdresseMAC mac_emetteur;  /* MAC du switch qui envoie ce BPDU         */
    int        prio_emetteur; /* priorite de l'emetteur                   */
} BPDU;

/* =========================================================
   TYPES D'EVENEMENTS
   ========================================================= */
typedef enum {
    EVT_ENVOYER_TRAME,
    EVT_RECEVOIR_TRAME,
    EVT_RECEVOIR_BPDU
} TypeEvenement;

/* =========================================================
   EVENEMENT
   On utilise une union : un evenement contient soit une Trame
   soit un BPDU, jamais les deux en meme temps.
   Ca divise la taille par ~60 pour les BPDUs.
   ========================================================= */
typedef struct {
    int           temps;
    TypeEvenement type;
    int           noeud_src;
    int           noeud_dst;
    int           port_arrivee; /* port d'arrivee chez noeud_dst (BPDUs) */
    union {
        Trame trame;
        BPDU  bpdu;
    } data;
} Evenement;

/* =========================================================
   FILE D'EVENEMENTS
   256 suffit largement pour STP (nombre fini de mises a jour).
   Pour la tempete broadcast on atteint la limite et on s'arrete.
   ========================================================= */
#define MAX_EVENEMENTS 512

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