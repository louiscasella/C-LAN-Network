#include <stdio.h>
#include "stp.h"

/* =========================================================
   COMPARAISON
   ========================================================= */

/* Retourne -1 si a<b, 0 si a==b, 1 si a>b */
static int comparer_mac(AdresseMAC a, AdresseMAC b)
{
    int i;
    for (i = 0; i < 6; i++) {
        if (a.octets[i] < b.octets[i]) return -1;
        if (a.octets[i] > b.octets[i]) return  1;
    }
    return 0;
}

/*
 * Retourne 1 si le BPDU annonce une meilleure racine que ce qu'on connait.
 * Regles STP :
 *   1. Priorite la plus basse gagne
 *   2. A egalite : MAC la plus basse gagne
 */
static int est_meilleure_racine(BPDU bpdu, AdresseMAC racine_mac, int racine_prio)
{
    if (bpdu.prio_racine < racine_prio) return 1;
    if (bpdu.prio_racine > racine_prio) return 0;
    return comparer_mac(bpdu.mac_racine, racine_mac) < 0;
}

/* =========================================================
   UTILITAIRES
   ========================================================= */

/* Retourne le numero de port de idx_switch vers voisin, ou -1 */
static int trouver_port_vers(Reseau *r, int idx_switch, int voisin)
{
    int i, port = 0;
    for (i = 0; i < r->nb_liens; i++) {
        Lien *l = &r->liens[i];
        if (l->noeud1 == idx_switch || l->noeud2 == idx_switch) {
            int v = (l->noeud1 == idx_switch) ? l->noeud2 : l->noeud1;
            if (v == voisin) return port;
            port++;
        }
    }
    return -1;
}

/* Retourne le poids du lien sur le port numero 'port' du switch */
static int poids_du_port(Reseau *r, int idx_switch, int port_cherche)
{
    int i, port = 0;
    for (i = 0; i < r->nb_liens; i++) {
        Lien *l = &r->liens[i];
        if (l->noeud1 == idx_switch || l->noeud2 == idx_switch) {
            if (port == port_cherche) return l->poids;
            port++;
        }
    }
    return 0;
}

/* =========================================================
   ENVOI DE BPDU
   Programme un EVT_RECEVOIR_BPDU pour chaque voisin switch,
   sauf le port port_exclure (-1 = aucun exclu).
   ========================================================= */
static void envoyer_bpdu_voisins(Reseau *r, FileEvenements *file,
                                  int idx_switch, int port_exclure,
                                  int temps_actuel)
{
    Switch *sw = &r->noeuds[idx_switch].equipement.sw;
    int i, port = 0;

    /* Le BPDU que ce switch envoie :
       "je connais cette racine, avec ce cout" */
    BPDU bpdu;
    bpdu.mac_racine    = sw->racine_mac;
    bpdu.prio_racine   = sw->racine_priorite;
    bpdu.cout_racine   = sw->cout_racine;
    bpdu.mac_emetteur  = sw->mac;
    bpdu.prio_emetteur = sw->priorite;

    for (i = 0; i < r->nb_liens; i++) {
        Lien *l = &r->liens[i];
        if (l->noeud1 == idx_switch || l->noeud2 == idx_switch) {
            int voisin = (l->noeud1 == idx_switch) ? l->noeud2 : l->noeud1;

            /* On envoie uniquement vers les switchs voisins */
            if (r->noeuds[voisin].type == TYPE_SWITCH && port != port_exclure) {
                Evenement evt;
                evt.temps        = temps_actuel + l->poids;
                evt.type         = EVT_RECEVOIR_BPDU;
                evt.noeud_src    = idx_switch;
                evt.noeud_dst    = voisin;
                /* port d'arrivee chez le voisin */
                evt.port_arrivee = trouver_port_vers(r, voisin, idx_switch);
                evt.data.bpdu    = bpdu;
                ajouter_evenement(file, evt);
            }
            port++;
        }
    }
}

/* =========================================================
   MISE A JOUR DES PORTS
   Apres chaque changement de racine ou de cout, on recalcule
   quel port est RACINE et on met les autres a DESIGNE.
   La phase BLOQUE est calculee apres convergence par finaliser_stp.
   ========================================================= */
static void recalculer_ports(Reseau *r, int idx_switch, int port_racine)
{
    Switch *sw = &r->noeuds[idx_switch].equipement.sw;
    int i, port = 0;

    for (i = 0; i < r->nb_liens; i++) {
        Lien *l = &r->liens[i];
        if (l->noeud1 == idx_switch || l->noeud2 == idx_switch) {
            if (port_racine == -1) {
                /* Ce switch est la racine : tous ses ports sont designes */
                sw->ports[port].etat = PORT_DESIGNE;
            } else if (port == port_racine) {
                sw->ports[port].etat = PORT_RACINE;
            } else {
                sw->ports[port].etat = PORT_DESIGNE;
            }
            port++;
        }
    }
}

/* =========================================================
   TRAITEMENT D'UN BPDU RECU
   C'est le coeur de STP.
   On compare le BPDU recu avec ce qu'on connait deja.
   On ne propage QUE si quelque chose a change.
   ========================================================= */
void traiter_bpdu(Reseau *r, FileEvenements *file,
                  int idx_switch, int port_arrivee,
                  BPDU bpdu, int temps_actuel)
{
    Switch *sw = &r->noeuds[idx_switch].equipement.sw;
    int poids, nouveau_cout;

    printf("    [Switch %d] recoit BPDU racine=", idx_switch);
    afficher_mac(bpdu.mac_racine);
    printf(" prio=%d cout=%d de ", bpdu.prio_racine, bpdu.cout_racine);
    afficher_mac(bpdu.mac_emetteur);
    printf("\n");

    /* Cout total si on passe par ce port pour atteindre la racine */
    poids       = poids_du_port(r, idx_switch, port_arrivee);
    nouveau_cout = bpdu.cout_racine + poids;

    /* --- Cas 1 : meilleure racine --- */
    if (est_meilleure_racine(bpdu, sw->racine_mac, sw->racine_priorite)) {
        printf("    [Switch %d] meilleure racine, mise a jour et propagation\n",
               idx_switch);
        sw->racine_mac      = bpdu.mac_racine;
        sw->racine_priorite = bpdu.prio_racine;
        sw->cout_racine     = nouveau_cout;
        recalculer_ports(r, idx_switch, port_arrivee);
        /* Propage a tous les voisins SAUF celui d'ou vient le BPDU */
        envoyer_bpdu_voisins(r, file, idx_switch, port_arrivee, temps_actuel);

    /* --- Cas 2 : meme racine, chemin plus court --- */
    } else if (mac_egales(bpdu.mac_racine, sw->racine_mac)
               && bpdu.prio_racine == sw->racine_priorite
               && nouveau_cout < sw->cout_racine) {
        printf("    [Switch %d] chemin plus court, mise a jour et propagation\n",
               idx_switch);
        sw->cout_racine = nouveau_cout;
        recalculer_ports(r, idx_switch, port_arrivee);
        envoyer_bpdu_voisins(r, file, idx_switch, port_arrivee, temps_actuel);

    /* --- Cas 3 : rien de nouveau, on ignore --- */
    } else {
        printf("    [Switch %d] BPDU ignore (pas meilleur)\n", idx_switch);
    }
}

/* =========================================================
   DEMARRAGE STP
   Chaque switch se croit racine et envoie ses BPDUs initiaux.
   ========================================================= */
void demarrer_stp(Reseau *r, FileEvenements *file)
{
    int i;

    printf("Demarrage STP : chaque switch se croit racine...\n\n");

    for (i = 0; i < r->nb_noeuds; i++) {
        if (r->noeuds[i].type != TYPE_SWITCH) continue;
        Switch *sw = &r->noeuds[i].equipement.sw;

        /* Au depart tous les ports sont designes (on est "racine") */
        recalculer_ports(r, i, -1);

        printf("  Switch [%d] ", i);
        afficher_mac(sw->mac);
        printf(" (prio=%d) envoie ses BPDUs\n", sw->priorite);

        envoyer_bpdu_voisins(r, file, i, -1, 0);
    }
    printf("\n");
}

/* =========================================================
   FINALISATION STP
   Apres convergence, on parcourt chaque lien switch-switch
   et on bloque le port du switch qui est le moins bon.
   ========================================================= */
void finaliser_stp(Reseau *r)
{
    int i;

    for (i = 0; i < r->nb_liens; i++) {
        Lien *l = &r->liens[i];
        int n1 = l->noeud1;
        int n2 = l->noeud2;

        /* On ne traite que les liens switch-switch */
        if (r->noeuds[n1].type != TYPE_SWITCH) continue;
        if (r->noeuds[n2].type != TYPE_SWITCH) continue;

        Switch *sw1 = &r->noeuds[n1].equipement.sw;
        Switch *sw2 = &r->noeuds[n2].equipement.sw;

        int port1 = trouver_port_vers(r, n1, n2);
        int port2 = trouver_port_vers(r, n2, n1);

        /* On ne touche pas aux ports racine */
        if (sw1->ports[port1].etat == PORT_RACINE) continue;
        if (sw2->ports[port2].etat == PORT_RACINE) continue;

        /* Le switch avec le cout le plus bas vers la racine est designe.
           L'autre est bloque.
           Criteres : 1) cout, 2) priorite, 3) MAC */
        int sw1_gagne;
        if      (sw1->cout_racine < sw2->cout_racine)       sw1_gagne = 1;
        else if (sw1->cout_racine > sw2->cout_racine)       sw1_gagne = 0;
        else if (sw1->priorite < sw2->priorite)             sw1_gagne = 1;
        else if (sw1->priorite > sw2->priorite)             sw1_gagne = 0;
        else    sw1_gagne = comparer_mac(sw1->mac, sw2->mac) < 0;

        if (sw1_gagne) {
            sw1->ports[port1].etat = PORT_DESIGNE;
            sw2->ports[port2].etat = PORT_BLOQUE;
        } else {
            sw1->ports[port1].etat = PORT_BLOQUE;
            sw2->ports[port2].etat = PORT_DESIGNE;
        }
    }

    printf("[STP] Convergence terminee, ports redondants bloques.\n");
}