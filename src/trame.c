#include <stdio.h>
#include <string.h>
#include "trame.h"

/* Adresse de broadcast globale */
const AdresseMAC MAC_BROADCAST = {{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }};

/* ─────────────────────────────────────────────
   Création d'une trame
   ───────────────────────────────────────────── */

Trame creer_trame(AdresseMAC destination, AdresseMAC source,
                  uint16_t type, uint8_t *donnees, int taille) {
    Trame t;

    /* Préambule : 7 octets à 0xAA */
    memset(t.preambule, 0xAA, 7);

    /* SFD : début de trame */
    t.sfd = 0xAB;

    /* Adresses */
    t.destination = destination;
    t.source      = source;

    /* Type de protocole */
    t.type = type;

    /* Données + bourrage si trop court */
    if (taille < TAILLE_DONNEES_MIN) {
        /* On copie les données puis on bourre avec des zéros */
        memcpy(t.donnees, donnees, taille);
        memset(t.donnees + taille, 0x00, TAILLE_DONNEES_MIN - taille);
        t.taille_donnees = TAILLE_DONNEES_MIN;
    } else {
        memcpy(t.donnees, donnees, taille);
        t.taille_donnees = taille;
    }

    /* FCS simplifié : XOR de tous les octets des adresses */
    uint8_t xor = 0;
    for (int i = 0; i < 6; i++) xor ^= destination.octets[i];
    for (int i = 0; i < 6; i++) xor ^= source.octets[i];
    t.fcs[0] = xor; t.fcs[1] = 0; t.fcs[2] = 0; t.fcs[3] = 0;

    return t;
}

/* ─────────────────────────────────────────────
   Affichage lisible
   ───────────────────────────────────────────── */

void afficher_trame(Trame t) {
    printf("┌─── Trame Ethernet ───────────────────┐\n");

    printf("│ Source      : ");
    afficher_mac(t.source);
    printf("\n");

    printf("│ Destination : ");
    afficher_mac(t.destination);
    if (est_broadcast(t.destination)) printf(" (BROADCAST)");
    printf("\n");

    printf("│ Type        : 0x%04X", t.type);
    if      (t.type == ETHER_TYPE_IPv4) printf(" (IPv4)");
    else if (t.type == ETHER_TYPE_ARP)  printf(" (ARP)");
    else if (t.type == ETHER_TYPE_IPv6) printf(" (IPv6)");
    printf("\n");

    printf("│ Données     : %d octet(s)\n", t.taille_donnees);
    printf("│ FCS         : %02x %02x %02x %02x\n",
           t.fcs[0], t.fcs[1], t.fcs[2], t.fcs[3]);

    printf("└──────────────────────────────────────┘\n");
}

/* ─────────────────────────────────────────────
   Affichage hexadécimal brut
   ───────────────────────────────────────────── */

void afficher_trame_hex(Trame t) {
    printf("=== Contenu brut de la trame (hex) ===\n");

    printf("Préambule : ");
    for (int i = 0; i < 7; i++) printf("%02x ", t.preambule[i]);
    printf("\n");

    printf("SFD       : %02x\n", t.sfd);

    printf("Dest MAC  : ");
    for (int i = 0; i < 6; i++) printf("%02x ", t.destination.octets[i]);
    printf("\n");

    printf("Src MAC   : ");
    for (int i = 0; i < 6; i++) printf("%02x ", t.source.octets[i]);
    printf("\n");

    printf("Type      : %02x %02x\n", (t.type >> 8) & 0xFF, t.type & 0xFF);

    printf("Données   : ");
    for (int i = 0; i < t.taille_donnees && i < 16; i++)
        printf("%02x ", t.donnees[i]);
    if (t.taille_donnees > 16) printf("...");
    printf("\n");

    printf("FCS       : %02x %02x %02x %02x\n",
           t.fcs[0], t.fcs[1], t.fcs[2], t.fcs[3]);
}

/* ─────────────────────────────────────────────
   Utilitaire broadcast
   ───────────────────────────────────────────── */

int est_broadcast(AdresseMAC mac) {
    for (int i = 0; i < 6; i++)
        if (mac.octets[i] != 0xFF) return 0;
    return 1;
}