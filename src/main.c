#include <stdio.h>
#include "reseau.h"
#include "config.h"
#include "trame.h"
#include "simulateur.h"
#include "stp.h"


/* =========================================================
   SCENARIO 0 : affiche le contenu d'un fichier de config .lan
   ========================================================= */
void scenario_0_afficher_config(const char *fichier)
{
    Reseau r;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 0 : affichage config .lan  ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    if (!charger_reseau(fichier, &r)) return;
    afficher_reseau(&r);
}

/* =========================================================
   SCENARIO 1 : reseau sans cycle, diffusion puis unicast
   ========================================================= */
void scenario_1()
{
    Reseau r;
    static FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 1 : diffusion puis unicast ║\n");
    printf("╚══════════════════════════════════════╝\n");

    if (!charger_reseau("../lan/mylan_no_cycle.lan", &r)) return;
    init_file(&file);

    AdresseMAC mac_st0 = r.noeuds[7].equipement.s.mac;
    AdresseMAC mac_st1 = r.noeuds[8].equipement.s.mac;

    Trame t1 = creer_trame(mac_st0, mac_st1, 0x0800, "Bonjour st1 !");
    ajouter_evenement(&file, creer_evt_envoi(0, 7, 8, t1));

    Trame t2 = creer_trame(mac_st1, mac_st0, 0x0800, "Bonjour st0, recu !");
    ajouter_evenement(&file, creer_evt_envoi(20, 8, 7, t2));

    lancer_simulation(&r, &file, 0);
}

/* =========================================================
   SCENARIO 2 : deux envois simultanes
   ========================================================= */
void scenario_2()
{
    Reseau r;
    static FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 2 : deux envois simultanes ║\n");
    printf("╚══════════════════════════════════════╝\n");

    if (!charger_reseau("../lan/my_better_lan.lan", &r)) return;
    init_file(&file);

    AdresseMAC mac_st3 = r.noeuds[3].equipement.s.mac;
    AdresseMAC mac_st4 = r.noeuds[4].equipement.s.mac;

    Trame t1 = creer_trame(mac_st3, mac_st4, 0x0800, "Message de st3 vers st4");
    ajouter_evenement(&file, creer_evt_envoi(0, 3, 4, t1));

    Trame t2 = creer_trame(mac_st4, mac_st3, 0x0800, "Message de st4 vers st3");
    ajouter_evenement(&file, creer_evt_envoi(0, 4, 3, t2));

    lancer_simulation(&r, &file, 0);
}

/* =========================================================
   SCENARIO 3 : affichage hexadecimal d'une trame
   ========================================================= */
void scenario_3()
{
    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 3 : affichage hexa          ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    AdresseMAC src = creer_mac(0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF);
    AdresseMAC dst = creer_mac(0x11, 0x22, 0x33, 0x44, 0x55, 0x66);
    Trame t = creer_trame(src, dst, 0x0800, "Hello !");

    printf("=== Affichage lisible ===\n");
    afficher_trame(&t);
    printf("\n=== Affichage hexadecimal brut ===\n");
    afficher_trame_hex(&t);
}

/* =========================================================
   SCENARIO 4 : tempete de broadcast SANS STP
   ========================================================= */
void scenario_4_tempete_broadcast()
{
    Reseau r;
    static FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 4 : TEMPETE DE BROADCAST   ║\n");
    printf("║  (reseau avec cycles, sans STP)       ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    if (!charger_reseau("../lan/mylan.lan", &r)) return;
    init_file(&file);

    AdresseMAC src       = r.noeuds[7].equipement.s.mac;
    AdresseMAC broadcast = creer_mac(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    Trame t = creer_trame(src, broadcast, 0x0806, "ARP: qui a 130.79.80.5 ?");
    ajouter_evenement(&file, creer_evt_envoi(0, 7, -1, t));

    lancer_simulation(&r, &file, 0);
}

/* =========================================================
   SCENARIO 5 : convergence STP sur my_better_lan.lan
   puis envoi broadcast : ne boucle plus
   ========================================================= */
void scenario_5_stp()
{
    Reseau r;
    static FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 5 : CONVERGENCE STP        ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    if (!charger_reseau("../lan/my_better_lan.lan", &r)) return;
    init_file(&file);

    demarrer_stp(&r, &file);
    lancer_simulation(&r, &file, 0);
    finaliser_stp(&r);
    afficher_stp_reseau(&r);

    printf("=== Envoi broadcast apres STP ===\n\n");
    init_file(&file);

    AdresseMAC src       = r.noeuds[3].equipement.s.mac;
    AdresseMAC broadcast = creer_mac(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    Trame t = creer_trame(src, broadcast, 0x0806, "ARP apres STP");
    ajouter_evenement(&file, creer_evt_envoi(0, 3, -1, t));

    lancer_simulation(&r, &file, 0);
}

/* =========================================================
   SCENARIO 6 : STP sur mylan.lan (plusieurs cycles)
   puis echange unicast entre st[7] et st[14].

   mylan.lan a 7 switchs formes en graphe avec plusieurs cycles.
   Sans STP ce serait une tempete. Avec STP les ports redondants
   sont bloques et la communication fonctionne normalement.
   ========================================================= */
void scenario_6_stp_multi_cycles()
{
    Reseau r;
    static FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 6 : STP + plusieurs cycles ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    if (!charger_reseau("../lan/mylan.lan", &r)) return;
    init_file(&file);

    /* Phase 1 : STP converge */
    demarrer_stp(&r, &file);
    lancer_simulation(&r, &file, 0);
    finaliser_stp(&r);
    afficher_stp_reseau(&r);

    /* Phase 2 : st[7] envoie a st[14]
       st[7] est connectee a sw[1], st[14] a sw[6]
       Le chemin passe par les switchs dont les ports sont ouverts */
    printf("=== Envoi unicast st[7] -> st[14] apres STP ===\n\n");
    init_file(&file);

    AdresseMAC mac_st7  = r.noeuds[7].equipement.s.mac;
    AdresseMAC mac_st14 = r.noeuds[14].equipement.s.mac;

    Trame t1 = creer_trame(mac_st7, mac_st14, 0x0800, "Salut st14 !");
    ajouter_evenement(&file, creer_evt_envoi(0, 7, 14, t1));

    lancer_simulation(&r, &file, 0);

    /* Phase 3 : st[14] repond a st[7]
       Cette fois les switchs connaissent deja mac_st7 -> transfert direct */
    printf("=== Reponse unicast st[14] -> st[7] ===\n\n");
    init_file(&file);

    Trame t2 = creer_trame(mac_st14, mac_st7, 0x0800, "Salut st7, recu !");
    ajouter_evenement(&file, creer_evt_envoi(0, 14, 7, t2));

    lancer_simulation(&r, &file, 0);
}

/* =========================================================
   SCENARIO 7 : apprentissage de la table de commutation
   On envoie plusieurs trames successives entre differentes
   stations pour montrer que les switchs apprennent les MACs
   et passent progressivement de la diffusion au transfert direct.
   ========================================================= */
void scenario_7_apprentissage()
{
    Reseau r;
    static FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 7 : apprentissage table    ║\n");
    printf("╚══════════════════════════════════════╝\n");

    if (!charger_reseau("../lan/mylan_no_cycle.lan", &r)) return;
    init_file(&file);

    AdresseMAC mac_st0 = r.noeuds[7].equipement.s.mac;
    AdresseMAC mac_st1 = r.noeuds[8].equipement.s.mac;
    AdresseMAC mac_st2 = r.noeuds[9].equipement.s.mac;

    /* t=0  : st0 -> st1 : table vide, diffusion */
    Trame t1 = creer_trame(mac_st0, mac_st1, 0x0800, "st0 vers st1 (1er envoi)");
    ajouter_evenement(&file, creer_evt_envoi(0, 7, 8, t1));

    /* t=20 : st1 -> st0 : switch connait mac_st0, transfert direct */
    Trame t2 = creer_trame(mac_st1, mac_st0, 0x0800, "st1 repond a st0");
    ajouter_evenement(&file, creer_evt_envoi(20, 8, 7, t2));

    /* t=40 : st0 -> st1 : les deux MACs sont connues, tout est direct */
    Trame t3 = creer_trame(mac_st0, mac_st1, 0x0800, "st0 vers st1 (2eme envoi, direct)");
    ajouter_evenement(&file, creer_evt_envoi(40, 7, 8, t3));

    /* t=60 : st2 -> st0 : st2 est inconnue, diffusion puis apprentissage */
    Trame t4 = creer_trame(mac_st2, mac_st0, 0x0800, "st2 entre dans le reseau");
    ajouter_evenement(&file, creer_evt_envoi(60, 9, 7, t4));

    lancer_simulation(&r, &file, 0);

    /* Affiche les tables de commutation a la fin pour voir l'apprentissage */
    printf("\n=== Tables de commutation apres simulation ===\n\n");
    int i;
    for (i = 0; i < r.nb_noeuds; i++) {
        if (r.noeuds[i].type == TYPE_SWITCH) {
            printf("Switch [%d] : ", i);
            afficher_table(&r.noeuds[i].equipement.sw.table);
        }
    }
}

/* =========================================================
   MAIN
   Commente / decommante les scenarios voulus.
   ========================================================= */
int main()
{
    // scenario_0_afficher_config("mylan.lan");
    // scenario_0_afficher_config("my_better_lan.lan");
    // scenario_0_afficher_config("mylan_no_cycle.lan");
    scenario_1();
    // scenario_2();
    // scenario_3();
    // scenario_4_tempete_broadcast(); 
    // scenario_5_stp(); 
    // scenario_6_stp_multi_cycles();
    // scenario_7_apprentissage();

    return 0;
}