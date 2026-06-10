#include <stdio.h>
#include "reseau.h"
#include "config.h"
#include "trame.h"
#include "simulateur.h"

/* =========================================================
   SCENARIO 1 : réseau sans cycle
   st0 envoie à st1 (table vide -> diffusion).
   st1 répond à st0 (switch connait déjà st0 -> direct).
   ========================================================= */
void scenario_1()
{
    Reseau r;
    FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 1 : diffusion puis unicast ║\n");
    printf("╚══════════════════════════════════════╝\n");

    if (!charger_reseau("../lan/mylan_no_cycle.lan", &r)) return;
    init_file(&file);

    AdresseMAC mac_st0 = r.noeuds[7].equipement.s.mac;
    AdresseMAC mac_st1 = r.noeuds[8].equipement.s.mac;

    /* t=0 : st0 envoie a st1 */
    Trame t1 = creer_trame(mac_st0, mac_st1, 0x0800, "Bonjour st1 !");
    ajouter_evenement(&file, creer_evt_envoi(0, 7, 8, t1));

    /* t=20 : st1 repond a st0 */
    Trame t2 = creer_trame(mac_st1, mac_st0, 0x0800, "Bonjour st0, recu !");
    ajouter_evenement(&file, creer_evt_envoi(20, 8, 7, t2));

    lancer_simulation(&r, &file, 0);
}

/* =========================================================
   SCENARIO 2 : deux envois simultanes
   st3 envoie a st4 ET st4 envoie a st3 en meme temps (t=0).
   ========================================================= */
void scenario_2()
{
    Reseau r;
    FileEvenements file;

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
   SCENARIO 4 : tempete de broadcast SANS STP
   Reseau avec cycles -> la trame boucle a l'infini.
   On limite a 40 evenements pour que ca reste lisible.
   ========================================================= */
void scenario_4_tempete_broadcast()
{
    Reseau r;
    FileEvenements file;

    printf("\n\n╔══════════════════════════════════════╗\n");
    printf("║  SCENARIO 4 : TEMPETE DE BROADCAST   ║\n");
    printf("║  (reseau avec cycles, sans STP)       ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    if (!charger_reseau("../lan/mylan.lan", &r)) return;
    init_file(&file);

    AdresseMAC src       = r.noeuds[7].equipement.s.mac;
    AdresseMAC broadcast = creer_mac(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    /* Une seule trame ARP broadcast suffit a saturer le reseau */
    Trame t = creer_trame(src, broadcast, 0x0806, "ARP: qui a 130.79.80.5 ?");
    ajouter_evenement(&file, creer_evt_envoi(0, 7, -1, t));

    lancer_simulation(&r, &file, 0);

    printf("CONCLUSION : sans STP, une trame boucle indefiniment.\n");
}

/* =========================================================
   MAIN
   Commente / decommante les scenarios voulus.
   ========================================================= */
int main()
{
    // scenario_1();
    // scenario_2(); 
    scenario_4_tempete_broadcast(); 

    return 0;
}
