#include <stdio.h>
#include <string.h>
#include "reseau.h"

// ─────────────────────────────────────────────
//  Constructeurs
// ─────────────────────────────────────────────

AdresseMAC creer_mac(uint8_t a, uint8_t b, uint8_t c,
                     uint8_t d, uint8_t e, uint8_t f) {
    AdresseMAC mac;
    mac.octets[0] = a; mac.octets[1] = b; mac.octets[2] = c;
    mac.octets[3] = d; mac.octets[4] = e; mac.octets[5] = f;
    return mac;
}

/* Les octets sont stockés du poids fort au poids faible :
   creer_ip(130, 79, 80, 21) -> 0x824F5015              */
AdresseIP creer_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return ((uint32_t)a << 24) | ((uint32_t)b << 16)
         | ((uint32_t)c <<  8) |  (uint32_t)d;
}

Station creer_station(AdresseMAC mac, AdresseIP ip) {
    Station s;
    s.mac = mac;
    s.ip  = ip;
    return s;
}

Switch creer_switch(AdresseMAC mac, int nb_ports, int priorite) {
    Switch sw;
    sw.mac      = mac;
    sw.nb_ports = nb_ports;
    sw.priorite = priorite;
    table_vider(&sw.table);
    return sw;
}

// ─────────────────────────────────────────────
//  Affichage
// ─────────────────────────────────────────────

void afficher_mac(AdresseMAC mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac.octets[0], mac.octets[1], mac.octets[2],
           mac.octets[3], mac.octets[4], mac.octets[5]);
}

/* Extraction des octets par décalage binaire */
void afficher_ip(AdresseIP ip) {
    printf("%d.%d.%d.%d",
           (ip >> 24) & 0xFF,
           (ip >> 16) & 0xFF,
           (ip >>  8) & 0xFF,
            ip        & 0xFF);
}

void afficher_station(Station s) {
    printf("Station {\n");
    printf("  MAC : "); afficher_mac(s.mac); printf("\n");
    printf("  IP  : "); afficher_ip(s.ip);   printf("\n");
    printf("}\n");
}

void afficher_table_commutation(TableCommutation t) {
    printf("  Table de commutation (%d entrée(s)) :\n", t.nb_entrees);
    if (t.nb_entrees == 0) {
        printf("    (vide)\n");
        return;
    }
    printf("    %-20s | Port\n", "Adresse MAC");
    printf("    ---------------------|------\n");
    for (int i = 0; i < TAILLE_TABLE_MAX; i++) {
        if (t.entrees[i].valide) {
            printf("    ");
            afficher_mac(t.entrees[i].mac);
            printf(" | %d\n", t.entrees[i].port);
        }
    }
}

void afficher_switch(Switch sw) {
    printf("Switch {\n");
    printf("  MAC      : "); afficher_mac(sw.mac); printf("\n");
    printf("  Ports    : %d\n", sw.nb_ports);
    printf("  Priorité : %d\n", sw.priorite);
    afficher_table_commutation(sw.table);
    printf("}\n");
}

void afficher_reseau(Reseau r) {
    printf("=== Réseau local (%d noeud(s), %d lien(s)) ===\n",
           r.nb_noeuds, r.nb_liens);
    for (int i = 0; i < r.nb_noeuds; i++) {
        printf("[%d] ", i);
        if (r.noeuds[i].type == TYPE_STATION)
            afficher_station(r.noeuds[i].equipement.station);
        else
            afficher_switch(r.noeuds[i].equipement.sw);
    }
    printf("--- Liens ---\n");
    for (int i = 0; i < r.nb_liens; i++) {
        printf("  %d <--(%d)--> %d\n",
               r.liens[i].source,
               r.liens[i].poids,
               r.liens[i].destination);
    }
}

// ─────────────────────────────────────────────
//  Table de commutation
// ─────────────────────────────────────────────

int mac_egales(AdresseMAC a, AdresseMAC b) {
    return memcmp(a.octets, b.octets, 6) == 0;
}

void table_vider(TableCommutation *t) {
    t->nb_entrees = 0;
    for (int i = 0; i < TAILLE_TABLE_MAX; i++)
        t->entrees[i].valide = 0;
}

void table_ajouter(TableCommutation *t, AdresseMAC mac, int port) {
    /* Mise à jour si l'adresse existe déjà */
    for (int i = 0; i < TAILLE_TABLE_MAX; i++) {
        if (t->entrees[i].valide && mac_egales(t->entrees[i].mac, mac)) {
            t->entrees[i].port = port;
            return;
        }
    }
    /* Sinon, cherche un slot libre */
    for (int i = 0; i < TAILLE_TABLE_MAX; i++) {
        if (!t->entrees[i].valide) {
            t->entrees[i].mac    = mac;
            t->entrees[i].port   = port;
            t->entrees[i].valide = 1;
            t->nb_entrees++;
            return;
        }
    }
    fprintf(stderr, "Erreur : table de commutation pleine\n");
}

/* Retourne le port associé à la MAC, ou -1 si inconnue */
int table_rechercher(TableCommutation *t, AdresseMAC mac) {
    for (int i = 0; i < TAILLE_TABLE_MAX; i++) {
        if (t->entrees[i].valide && mac_egales(t->entrees[i].mac, mac))
            return t->entrees[i].port;
    }
    return -1;
}