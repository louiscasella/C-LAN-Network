#ifndef TRAME_H
#define TRAME_H

#include <stdint.h>
#include "reseau.h"

/* ─────────────────────────────────────────────
   Trame Ethernet (structure fidèle au format réel)
   Taille totale : 26 octets fixes + données + bourrage
   ───────────────────────────────────────────── */

#define TAILLE_DONNEES_MIN  46
#define TAILLE_DONNEES_MAX  1500

/* Types Ethernet courants */
#define ETHER_TYPE_IPv4  0x0800
#define ETHER_TYPE_ARP   0x0806
#define ETHER_TYPE_IPv6  0x86DD

typedef struct {
    uint8_t    preambule[7];               /* 7 octets : 0xAA (synchronisation) */
    uint8_t    sfd;                        /* 1 octet  : 0xAB (début de trame)  */
    AdresseMAC destination;               /* 6 octets : adresse MAC dest        */
    AdresseMAC source;                    /* 6 octets : adresse MAC src         */
    uint16_t   type;                      /* 2 octets : protocole (IPv4, ARP…)  */
    uint8_t    donnees[TAILLE_DONNEES_MAX];/* 46–1500 octets : payload           */
    int        taille_donnees;            /* taille réelle du payload           */
    uint8_t    fcs[4];                    /* 4 octets : checksum (simplifié)    */
} Trame;

/* Adresse de broadcast : FF:FF:FF:FF:FF:FF */
extern const AdresseMAC MAC_BROADCAST;

/* ── Fonctions ── */

/* Crée une trame avec les champs remplis */
Trame creer_trame(AdresseMAC destination, AdresseMAC source,
                  uint16_t type, uint8_t *donnees, int taille);

/* Affichage lisible (source, destination, type…) */
void afficher_trame(Trame t);

/* Affichage brut octet par octet en hexadécimal */
void afficher_trame_hex(Trame t);

/* Renvoie 1 si la destination est le broadcast */
int est_broadcast(AdresseMAC mac);

#endif /* TRAME_H */