#include <stdio.h>
#include "reseau.h"

int main(void) {
    /* ── Création du réseau de l'exemple du sujet ── */
    Reseau r;
    r.nb_noeuds = 0;
    r.nb_liens  = 0;

    /* Switch sw1 */
    AdresseMAC mac_sw1 = creer_mac(0x01, 0x45, 0x23, 0xa6, 0xf7, 0xab);
    Switch sw1 = creer_switch(mac_sw1, 8, 1024);
    r.noeuds[0].type = TYPE_SWITCH;
    r.noeuds[0].equipement.sw = sw1;
    r.nb_noeuds++;

    /* Station st1 */
    AdresseMAC mac_st1 = creer_mac(0x54, 0xd6, 0xa6, 0x82, 0xc5, 0x23);
    AdresseIP  ip_st1  = creer_ip(130, 79, 80, 21);
    r.noeuds[1].type = TYPE_STATION;
    r.noeuds[1].equipement.station = creer_station(mac_st1, ip_st1);
    r.nb_noeuds++;

    /* Station st2 */
    AdresseMAC mac_st2 = creer_mac(0xc8, 0x69, 0x72, 0x5e, 0x43, 0xaf);
    AdresseIP  ip_st2  = creer_ip(130, 79, 80, 27);
    r.noeuds[2].type = TYPE_STATION;
    r.noeuds[2].equipement.station = creer_station(mac_st2, ip_st2);
    r.nb_noeuds++;

    /* Station st3 */
    AdresseMAC mac_st3 = creer_mac(0x77, 0xac, 0xd6, 0x82, 0x12, 0x23);
    AdresseIP  ip_st3  = creer_ip(130, 79, 80, 42);
    r.noeuds[3].type = TYPE_STATION;
    r.noeuds[3].equipement.station = creer_station(mac_st3, ip_st3);
    r.nb_noeuds++;

    /* Liens */
    r.liens[0] = (Lien){0, 1, 4};
    r.liens[1] = (Lien){0, 2, 19};
    r.liens[2] = (Lien){0, 3, 4};
    r.nb_liens = 3;

    /* ── Affichage ── */
    afficher_reseau(r);

    /* ── Test comparaison d'IPs (maintenant un simple == ) ── */
    printf("\n--- Test comparaison IP ---\n");
    AdresseIP ip_a = creer_ip(130, 79, 80, 21);
    AdresseIP ip_b = creer_ip(130, 79, 80, 27);
    printf("ip_st1 == ip_a : %s\n", ip_st1 == ip_a ? "vrai" : "faux");
    printf("ip_st1 == ip_b : %s\n", ip_st1 == ip_b ? "vrai" : "faux");

    /* ── Test table de commutation ── */
    printf("\n--- Test table de commutation de sw1 ---\n");
    Switch *psw = &r.noeuds[0].equipement.sw;
    table_ajouter(&psw->table, mac_st1, 1);
    table_ajouter(&psw->table, mac_st2, 2);
    table_ajouter(&psw->table, mac_st3, 3);
    afficher_table_commutation(psw->table);

    printf("\nRecherche st2 -> port %d\n",
           table_rechercher(&psw->table, mac_st2));
    printf("Recherche MAC inconnue -> port %d (attendu : -1)\n",
           table_rechercher(&psw->table,
               creer_mac(0x00,0x00,0x00,0x00,0x00,0x01)));

    return 0;
}