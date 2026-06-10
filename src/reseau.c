#include <stdio.h>
#include "reseau.h"

/* =========================================================
   FONCTIONS DE CREATION
   ========================================================= */

AdresseMAC creer_mac(uint8_t a, uint8_t b, uint8_t c,
                     uint8_t d, uint8_t e, uint8_t f)
{
    AdresseMAC mac;
    mac.octets[0] = a; mac.octets[1] = b; mac.octets[2] = c;
    mac.octets[3] = d; mac.octets[4] = e; mac.octets[5] = f;
    return mac;
}

AdresseIP creer_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    AdresseIP ip;
    ip.octets[0] = a; ip.octets[1] = b;
    ip.octets[2] = c; ip.octets[3] = d;
    return ip;
}

Station creer_station(AdresseMAC mac, AdresseIP ip)
{
    Station st;
    st.mac = mac;
    st.ip  = ip;
    return st;
}

Switch creer_switch(AdresseMAC mac, int nb_ports, int priorite)
{
    Switch sw;
    sw.mac      = mac;
    sw.nb_ports = nb_ports;
    sw.priorite = priorite;
    sw.table.nb_entrees = 0;
    return sw;
}

Reseau creer_reseau_vide()
{
    Reseau r;
    r.nb_noeuds = 0;
    r.nb_liens  = 0;
    return r;
}

int ajouter_station(Reseau *r, Station st)
{
    int i = r->nb_noeuds;
    r->noeuds[i].type         = TYPE_STATION;
    r->noeuds[i].equipement.s = st;
    r->nb_noeuds++;
    return i;
}

int ajouter_switch(Reseau *r, Switch sw)
{
    int i = r->nb_noeuds;
    r->noeuds[i].type          = TYPE_SWITCH;
    r->noeuds[i].equipement.sw = sw;
    r->nb_noeuds++;
    return i;
}

void ajouter_lien(Reseau *r, int i, int j, int poids)
{
    int l = r->nb_liens;
    r->liens[l].noeud1 = i;
    r->liens[l].noeud2 = j;
    r->liens[l].poids  = poids;
    r->nb_liens++;
}

int mac_egales(AdresseMAC a, AdresseMAC b)
{
    int i;
    for (i = 0; i < 6; i++)
        if (a.octets[i] != b.octets[i]) return 0;
    return 1;
}

/* =========================================================
   FONCTIONS D'AFFICHAGE
   ========================================================= */

void afficher_mac(AdresseMAC mac)
{
    printf("%02X:%02X:%02X:%02X:%02X:%02X",
           mac.octets[0], mac.octets[1], mac.octets[2],
           mac.octets[3], mac.octets[4], mac.octets[5]);
}

void afficher_ip(AdresseIP ip)
{
    printf("%d.%d.%d.%d",
           ip.octets[0], ip.octets[1], ip.octets[2], ip.octets[3]);
}

void afficher_table(TableCommutation *table)
{
    int i;
    if (table->nb_entrees == 0) {
        printf("    (table vide)\n");
        return;
    }
    printf("    %-20s  Port\n", "Adresse MAC");
    printf("    %-20s  ----\n", "-------------------");
    for (i = 0; i < table->nb_entrees; i++) {
        printf("    ");
        afficher_mac(table->entrees[i].mac);
        printf("  ->  %d\n", table->entrees[i].port);
    }
}

void afficher_station(Station *st)
{
    printf("  MAC : "); afficher_mac(st->mac); printf("\n");
    printf("  IP  : "); afficher_ip(st->ip);   printf("\n");
}

void afficher_switch(Switch *sw)
{
    printf("  MAC      : "); afficher_mac(sw->mac); printf("\n");
    printf("  Ports    : %d\n", sw->nb_ports);
    printf("  Priorite : %d\n", sw->priorite);
    printf("  Table de commutation :\n");
    afficher_table(&sw->table);
}

void afficher_reseau(Reseau *r)
{
    int i;
    printf("=== RESEAU (%d equipements, %d liens) ===\n\n",
           r->nb_noeuds, r->nb_liens);
    for (i = 0; i < r->nb_noeuds; i++) {
        Noeud *n = &r->noeuds[i];
        if (n->type == TYPE_SWITCH) {
            printf("[%d] SWITCH\n", i);
            afficher_switch(&n->equipement.sw);
        } else {
            printf("[%d] STATION\n", i);
            afficher_station(&n->equipement.s);
        }
        printf("\n");
    }
    printf("--- Liens ---\n");
    for (i = 0; i < r->nb_liens; i++)
        printf("  [%d] <--(poids %d)--> [%d]\n",
               r->liens[i].noeud1, r->liens[i].poids, r->liens[i].noeud2);
}
