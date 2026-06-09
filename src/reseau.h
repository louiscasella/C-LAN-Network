#ifndef RESEAU_H
#define RESEAU_H

#include <stdint.h>

// ─────────────────────────────────────────────
//  Types de base
// ─────────────────────────────────────────────

/* Adresse MAC : 6 octets dans une struct pour passage par valeur */
typedef struct {
    uint8_t octets[6];
} AdresseMAC;

/* Adresse IPv4 : 1 entier 32 bits, octet de poids fort = premier octet */
typedef uint32_t AdresseIP;

// ─────────────────────────────────────────────
//  Table de commutation d'un switch
// ─────────────────────────────────────────────

#define TAILLE_TABLE_MAX 256

typedef struct {
    AdresseMAC mac;
    int        port;
    int        valide;
} EntreeTable;

typedef struct {
    EntreeTable entrees[TAILLE_TABLE_MAX];
    int         nb_entrees;
} TableCommutation;

// ─────────────────────────────────────────────
//  Équipements
// ─────────────────────────────────────────────

typedef struct {
    AdresseMAC mac;
    AdresseIP  ip;
} Station;

typedef struct {
    AdresseMAC       mac;
    int              nb_ports;
    int              priorite;
    TableCommutation table;
} Switch;

typedef enum {
    TYPE_SWITCH  = 2,
    TYPE_STATION = 1
} TypeEquipement;

typedef struct {
    TypeEquipement type;
    union {
        Station station;
        Switch  sw;
    } equipement;
} Noeud;

// ─────────────────────────────────────────────
//  Réseau local (graphe étiqueté)
// ─────────────────────────────────────────────

#define NB_NOEUDS_MAX 64

typedef struct {
    int source;
    int destination;
    int poids;
} Lien;

typedef struct {
    Noeud noeuds[NB_NOEUDS_MAX];
    int   nb_noeuds;
    Lien  liens[NB_NOEUDS_MAX * NB_NOEUDS_MAX];
    int   nb_liens;
} Reseau;

// ─────────────────────────────────────────────
//  Fonctions d'affichage
// ─────────────────────────────────────────────

void afficher_mac(AdresseMAC mac);
void afficher_ip(AdresseIP ip);
void afficher_station(Station s);
void afficher_switch(Switch sw);
void afficher_table_commutation(TableCommutation t);
void afficher_reseau(Reseau r);

// ─────────────────────────────────────────────
//  Fonctions utilitaires
// ─────────────────────────────────────────────

AdresseMAC creer_mac(uint8_t a, uint8_t b, uint8_t c,
                     uint8_t d, uint8_t e, uint8_t f);
AdresseIP  creer_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
Station    creer_station(AdresseMAC mac, AdresseIP ip);
Switch     creer_switch(AdresseMAC mac, int nb_ports, int priorite);

void table_ajouter(TableCommutation *t, AdresseMAC mac, int port);
int  table_rechercher(TableCommutation *t, AdresseMAC mac);
void table_vider(TableCommutation *t);

int  mac_egales(AdresseMAC a, AdresseMAC b);

#endif /* RESEAU_H */

// ─────────────────────────────────────────────
//  Lecture fichier de configuration
// ─────────────────────────────────────────────

Reseau charger_reseau(const char *chemin);

// ─────────────────────────────────────────────
//  Utilitaires de navigation dans le graphe
// ─────────────────────────────────────────────

/* Retourne le numéro de port du lien entre sw_idx et neighbor_idx.
   Les ports sont numérotés dans l'ordre d'apparition des liens. */
int get_port(Reseau *r, int sw_idx, int neighbor_idx);

/* Retourne l'indice du voisin connecté au port donné d'un switch. */
int get_voisin(Reseau *r, int sw_idx, int port);