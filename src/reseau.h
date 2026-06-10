#ifndef RESEAU_H
#define RESEAU_H

#include <stdint.h>  /* pour uint8_t, uint16_t, etc. */

/* =========================================================
   ADRESSES
   ========================================================= */

/* Adresse MAC : 6 octets (ex: AA:BB:CC:DD:EE:FF) */
typedef struct {
    uint8_t octets[6];
} AdresseMAC;

/* Adresse IPv4 : 4 octets (ex: 130.79.80.21) */
typedef struct {
    uint8_t octets[4];
} AdresseIP;

/* =========================================================
   STATION
   Un ordinateur simple connecté au réseau.
   ========================================================= */
typedef struct {
    AdresseMAC mac;
    AdresseIP  ip;
} Station;

/* =========================================================
   TABLE DE COMMUTATION D'UN SWITCH
   Le switch apprend sur quel port se trouve chaque adresse MAC.
   On stocke ça dans un tableau de lignes (mac -> numéro de port).
   ========================================================= */

#define MAX_ENTREES_TABLE 64

typedef struct {
    AdresseMAC mac;
    int        port;
} EntreeTable;

typedef struct {
    EntreeTable entrees[MAX_ENTREES_TABLE];
    int         nb_entrees;
} TableCommutation;

/* =========================================================
   SWITCH
   ========================================================= */
typedef struct {
    AdresseMAC       mac;
    int              nb_ports;
    int              priorite;
    TableCommutation table;
} Switch;

/* =========================================================
   TYPE D'EQUIPEMENT
   ========================================================= */
typedef enum {
    TYPE_STATION = 1,
    TYPE_SWITCH  = 2
} TypeEquipement;

typedef struct {
    TypeEquipement type;
    union {
        Station s;
        Switch  sw;
    } equipement;
} Noeud;

/* =========================================================
   LIEN
   ========================================================= */
typedef struct {
    int noeud1;
    int noeud2;
    int poids;
} Lien;

/* =========================================================
   RESEAU LOCAL
   ========================================================= */

#define MAX_NOEUDS 32
#define MAX_LIENS  64

typedef struct {
    Noeud noeuds[MAX_NOEUDS];
    int   nb_noeuds;
    Lien  liens[MAX_LIENS];
    int   nb_liens;
} Reseau;

/* =========================================================
   PROTOTYPES
   ========================================================= */

void afficher_mac(AdresseMAC mac);
void afficher_ip(AdresseIP ip);
void afficher_station(Station *st);
void afficher_switch(Switch *sw);
void afficher_reseau(Reseau *r);
void afficher_table(TableCommutation *table);

AdresseMAC creer_mac(uint8_t a, uint8_t b, uint8_t c,
                     uint8_t d, uint8_t e, uint8_t f);
AdresseIP  creer_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
Station    creer_station(AdresseMAC mac, AdresseIP ip);
Switch     creer_switch(AdresseMAC mac, int nb_ports, int priorite);
Reseau     creer_reseau_vide();
int        ajouter_station(Reseau *r, Station st);
int        ajouter_switch(Reseau *r, Switch sw);
void       ajouter_lien(Reseau *r, int i, int j, int poids);
int        mac_egales(AdresseMAC a, AdresseMAC b);

#endif /* RESEAU_H */
