#include <stdio.h>
#include <string.h>
#include "simulateur.h"
#include "stp.h"

/* =========================================================
   GESTION DE LA FILE D'EVENEMENTS
   ========================================================= */

void init_file(FileEvenements *file)
{
    file->nb_evenements = 0;
}

/* Tri a bulles par temps croissant */
static void trier_file(FileEvenements *file)
{
    int i, j;
    Evenement tmp;
    for (i = 0; i < file->nb_evenements - 1; i++) {
        for (j = 0; j < file->nb_evenements - 1 - i; j++) {
            if (file->evenements[j].temps > file->evenements[j+1].temps) {
                tmp                  = file->evenements[j];
                file->evenements[j]   = file->evenements[j+1];
                file->evenements[j+1] = tmp;
            }
        }
    }
}

void ajouter_evenement(FileEvenements *file, Evenement evt)
{
    if (file->nb_evenements >= MAX_EVENEMENTS) {
        printf("File pleine (%d evenements max)\n", MAX_EVENEMENTS);
        return;
    }
    file->evenements[file->nb_evenements] = evt;
    file->nb_evenements++;
    trier_file(file);
}

/* Cree un evenement d'envoi de trame */
Evenement creer_evt_envoi(int temps, int src, int dst, Trame trame)
{
    Evenement evt;
    evt.temps        = temps;
    evt.type         = EVT_ENVOYER_TRAME;
    evt.noeud_src    = src;
    evt.noeud_dst    = dst;
    evt.port_arrivee = -1;
    evt.data.trame   = trame;
    return evt;
}

/* =========================================================
   LOGIQUE DU SWITCH (commutation de trames)
   ========================================================= */

static int chercher_port(TableCommutation *table, AdresseMAC mac)
{
    int i;
    for (i = 0; i < table->nb_entrees; i++)
        if (mac_egales(table->entrees[i].mac, mac))
            return table->entrees[i].port;
    return -1;
}

static void apprendre_mac(TableCommutation *table, AdresseMAC mac, int port)
{
    int i;
    for (i = 0; i < table->nb_entrees; i++) {
        if (mac_egales(table->entrees[i].mac, mac)) {
            table->entrees[i].port = port;
            return;
        }
    }
    if (table->nb_entrees < MAX_ENTREES_TABLE) {
        table->entrees[table->nb_entrees].mac  = mac;
        table->entrees[table->nb_entrees].port = port;
        table->nb_entrees++;
    }
}

static int trouver_port_vers(Reseau *r, int idx_switch, int noeud_idx)
{
    int i, port = 0;
    for (i = 0; i < r->nb_liens; i++) {
        Lien *l = &r->liens[i];
        if (l->noeud1 == idx_switch || l->noeud2 == idx_switch) {
            int voisin = (l->noeud1 == idx_switch) ? l->noeud2 : l->noeud1;
            if (voisin == noeud_idx) return port;
            port++;
        }
    }
    return -1;
}

static void traiter_trame_switch(Reseau *r, FileEvenements *file,
                                  int idx_switch, int port_arrivee,
                                  Trame *trame, int temps_actuel)
{
    Switch *sw = &r->noeuds[idx_switch].equipement.sw;
    int i, port_dst;

    /* Un port bloque par STP ne recoit pas */
    if (port_arrivee >= 0 && sw->ports[port_arrivee].etat == PORT_BLOQUE) {
        printf("    [Switch %d] port %d BLOQUE, trame rejetee\n",
               idx_switch, port_arrivee);
        return;
    }

    /* Apprentissage de la MAC source */
    apprendre_mac(&sw->table, trame->source, port_arrivee);
    printf("    [Switch %d] Appris MAC ", idx_switch);
    afficher_mac(trame->source);
    printf(" sur port %d\n", port_arrivee);

    port_dst = chercher_port(&sw->table, trame->destination);

    if (port_dst != -1) {
        /* MAC connue : transfert direct */
        printf("    [Switch %d] MAC connue sur port %d, transfert direct\n",
               idx_switch, port_dst);
        int port = 0;
        for (i = 0; i < r->nb_liens; i++) {
            Lien *l = &r->liens[i];
            if (l->noeud1 == idx_switch || l->noeud2 == idx_switch) {
                int voisin = (l->noeud1 == idx_switch) ? l->noeud2 : l->noeud1;
                if (port == port_dst) {
                    Evenement evt = creer_evt_envoi(
                        temps_actuel + l->poids, idx_switch, voisin, *trame);
                    evt.type = EVT_RECEVOIR_TRAME;
                    ajouter_evenement(file, evt);
                    break;
                }
                port++;
            }
        }
    } else {
        /* MAC inconnue : diffusion sur tous les ports non bloques sauf arrivee */
        printf("    [Switch %d] MAC inconnue, diffusion\n", idx_switch);
        int port = 0;
        for (i = 0; i < r->nb_liens; i++) {
            Lien *l = &r->liens[i];
            if (l->noeud1 == idx_switch || l->noeud2 == idx_switch) {
                int voisin = (l->noeud1 == idx_switch) ? l->noeud2 : l->noeud1;
                if (port != port_arrivee
                    && sw->ports[port].etat != PORT_BLOQUE) {
                    Evenement evt = creer_evt_envoi(
                        temps_actuel + l->poids, idx_switch, voisin, *trame);
                    evt.type = EVT_RECEVOIR_TRAME;
                    ajouter_evenement(file, evt);
                }
                port++;
            }
        }
    }
}

/* =========================================================
   BOUCLE PRINCIPALE
   ========================================================= */

void lancer_simulation(Reseau *r, FileEvenements *file, int limite_evenements)
{
    int temps_actuel = -1;
    int nb_traites   = 0;

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║     DEBUT DE LA SIMULATION           ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    while (file->nb_evenements > 0) {

        if (limite_evenements > 0 && nb_traites >= limite_evenements) {
            printf("LIMITE DE %d EVENEMENTS ATTEINTE\n\n", limite_evenements);
            break;
        }

        /* Depile le premier evenement */
        Evenement evt = file->evenements[0];
        int i;
        for (i = 0; i < file->nb_evenements - 1; i++)
            file->evenements[i] = file->evenements[i + 1];
        file->nb_evenements--;
        nb_traites++;

        if (evt.temps != temps_actuel) {
            temps_actuel = evt.temps;
            printf("==== t = %d ====================================\n",
                   temps_actuel);
        }

        if (evt.type == EVT_ENVOYER_TRAME) {

            printf("  Station [%d] envoie vers ", evt.noeud_src);
            afficher_mac(evt.data.trame.destination);
            printf("\n");
            afficher_trame(&evt.data.trame);

            /* Trouve le switch voisin et programme la reception */
            for (i = 0; i < r->nb_liens; i++) {
                Lien *l = &r->liens[i];
                if (l->noeud1 == evt.noeud_src || l->noeud2 == evt.noeud_src) {
                    int voisin = (l->noeud1 == evt.noeud_src)
                                 ? l->noeud2 : l->noeud1;
                    if (r->noeuds[voisin].type == TYPE_SWITCH) {
                        Evenement rec = creer_evt_envoi(
                            temps_actuel + l->poids,
                            evt.noeud_src, voisin, evt.data.trame);
                        rec.type = EVT_RECEVOIR_TRAME;
                        rec.port_arrivee = trouver_port_vers(r, voisin, evt.noeud_src);
                        ajouter_evenement(file, rec);
                        break;
                    }
                }
            }

        } else if (evt.type == EVT_RECEVOIR_TRAME) {

            int dest = evt.noeud_dst;
            Noeud *n = &r->noeuds[dest];

            if (n->type == TYPE_SWITCH) {
                printf("  Switch [%d] recoit une trame\n", dest);
                int port = trouver_port_vers(r, dest, evt.noeud_src);
                traiter_trame_switch(r, file, dest, port,
                                     &evt.data.trame, temps_actuel);
            } else {
                printf("  Station [%d] recoit : \"%s\"\n",
                       dest, (char *)evt.data.trame.donnees);
                if (mac_egales(evt.data.trame.destination,
                               n->equipement.s.mac))
                    printf("    -> Trame acceptee\n");
                else
                    printf("    -> Trame ignoree (pas pour moi)\n");
            }

        } else if (evt.type == EVT_RECEVOIR_BPDU) {

            /* Delegue a stp.c */
            traiter_bpdu(r, file,
                         evt.noeud_dst,
                         evt.port_arrivee,
                         evt.data.bpdu,
                         temps_actuel);
        }

        printf("\n");
    }

    printf("╔══════════════════════════════════════╗\n");
    printf("║     FIN DE LA SIMULATION             ║\n");
    printf("╚══════════════════════════════════════╝\n");
}