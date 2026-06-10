#ifndef RESEAU_H
#define RESEAU_H

#include <stdint.h>

/* =========================================================
   ADRESSES
   ========================================================= */
typedef struct {
    uint8_t octets[6];
} AdresseMAC;

typedef struct {
    uint8_t octets[4];
} AdresseIP;

/* =========================================================
   STATION
   ========================================================= */
typedef struct {
    AdresseMAC mac;
    AdresseIP  ip;
} Station;

/* =========================================================
   TABLE DE COMMUTATION
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
   ETATS STP D'UN PORT
   ========================================================= */
typedef enum {
    PORT_INCONNU,   /* etat initial                            */
    PORT_RACINE,    /* chemin le plus court vers la racine     */
    PORT_DESIGNE,   /* port ouvert, transmet les trames        */
    PORT_BLOQUE     /* port ferme par STP pour casser un cycle */
} EtatPort;

typedef struct {
    EtatPort etat;
} PortSTP;

/* =========================================================
   SWITCH
   ========================================================= */
#define MAX_PORTS 16

typedef struct {
    AdresseMAC       mac;
    int              nb_ports;
    int              priorite;
    TableCommutation table;

    /* Infos STP */
    AdresseMAC racine_mac;       /* MAC de la racine connue          */
    int        racine_priorite;  /* priorite de la racine connue     */
    int        cout_racine;      /* cout de ce switch vers la racine */
    PortSTP    ports[MAX_PORTS]; /* etat de chaque port              */
} Switch;

/* =========================================================
   RESEAU
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

typedef struct {
    int noeud1;
    int noeud2;
    int poids;
} Lien;

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
void afficher_stp_reseau(Reseau *r);

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