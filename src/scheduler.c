#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

/* ─────────────────────────────────────────────
   File d'événements (FIFO circulaire)
   ───────────────────────────────────────────── */

void file_init(FileEvenements *f) {
    f->tete   = 0;
    f->queue  = 0;
    f->taille = 0;
}

/* Ajoute un événement en fin de file */
void enfiler(FileEvenements *f, Evenement ev) {
    if (f->taille >= FILE_MAX) {
        fprintf(stderr, "Erreur : file d'événements pleine\n");
        return;
    }
    f->evenements[f->queue] = ev;
    f->queue  = (f->queue + 1) % FILE_MAX; /* On avance de façon circulaire */
    f->taille++;
}

/* Retire et retourne le premier événement de la file */
Evenement defiler(FileEvenements *f) {
    Evenement ev = f->evenements[f->tete];
    f->tete   = (f->tete + 1) % FILE_MAX;
    f->taille--;
    return ev;
}

int file_est_vide(FileEvenements *f) {
    return f->taille == 0;
}

/* ─────────────────────────────────────────────
   Lancement d'un envoi depuis une station
   ───────────────────────────────────────────── */

void lancer_envoi(Reseau *r, FileEvenements *f,
                  int idx_src, int idx_dst,
                  uint16_t type, uint8_t *donnees, int taille) {

    /* Récupère les adresses MAC source et destination */
    AdresseMAC mac_src = r->noeuds[idx_src].equipement.station.mac;
    AdresseMAC mac_dst;

    if (idx_dst == -1) {
        /* Envoi en broadcast */
        mac_dst = MAC_BROADCAST;
    } else {
        mac_dst = r->noeuds[idx_dst].equipement.station.mac;
    }

    /* Construit la trame */
    Trame t = creer_trame(mac_dst, mac_src, type, donnees, taille);

    /* Cherche le switch directement connecté à la station source */
    int voisin = -1;
    for (int i = 0; i < r->nb_liens; i++) {
        if (r->liens[i].source == idx_src) { voisin = r->liens[i].destination; break; }
        if (r->liens[i].destination == idx_src) { voisin = r->liens[i].source; break; }
    }
    if (voisin == -1) {
        fprintf(stderr, "Erreur : station [%d] non connectée\n", idx_src);
        return;
    }

    /* Le port d'arrivée = port du switch connecté à cette station */
    int port = get_port(r, voisin, idx_src);

    /* Crée le premier événement et l'ajoute dans la file */
    Evenement ev = { t, idx_src, voisin, port };
    enfiler(f, ev);

    printf("[ENVOI] Station [%d] → Switch [%d] (port %d)\n",
           idx_src, voisin, port);
}

/* ─────────────────────────────────────────────
   Traitement d'un événement
   ───────────────────────────────────────────── */

void traiter_evenement(Reseau *r, Evenement *ev, FileEvenements *f) {

    Noeud *noeud = &r->noeuds[ev->destinataire];

    /* ── Cas 1 : la trame arrive chez une station ── */
    if (noeud->type == TYPE_STATION) {
        printf("[RECEPTION] Station [%d] reçoit une trame de [%d]\n",
               ev->destinataire, ev->expediteur);
        afficher_trame(ev->trame);
        return;
    }

    /* ── Cas 2 : la trame arrive chez un switch ── */
    Switch *sw = &noeud->equipement.sw;

    printf("[SWITCH %d] Trame reçue sur le port %d\n",
           ev->destinataire, ev->port_arrivee);

    /* Étape 1 : APPRENTISSAGE
       Le switch note que l'adresse MAC source est joignable via port_arrivee */
    table_ajouter(&sw->table, ev->trame.source, ev->port_arrivee);
    printf("  → Apprentissage : ");
    afficher_mac(ev->trame.source);
    printf(" sur port %d\n", ev->port_arrivee);

    /* Étape 2 : COMMUTATION
       Le switch cherche le port de sortie pour l'adresse destination */
    int port_dest = -1;

    /* Si c'est un broadcast, on flood toujours (pas besoin de chercher) */
    if (!est_broadcast(ev->trame.destination))
        port_dest = table_rechercher(&sw->table, ev->trame.destination);

    if (port_dest != -1) {
        /* ── UNICAST : on connaît le port de sortie ── */
        int voisin = get_voisin(r, ev->destinataire, port_dest);
        int port_chez_voisin = get_port(r, voisin, ev->destinataire);

        printf("  → Unicast : destination connue sur port %d → nœud [%d]\n",
               port_dest, voisin);

        Evenement nouveau = { ev->trame, ev->destinataire, voisin,
                              (port_chez_voisin == -1) ? 0 : port_chez_voisin };
        enfiler(f, nouveau);

    } else {
        /* ── FLOOD : destination inconnue ou broadcast ──
           On envoie sur tous les ports sauf celui d'où vient la trame */
        printf("  → Flood (destination inconnue ou broadcast)\n");

        int port = 0; /* numéro de port courant du switch */
        for (int i = 0; i < r->nb_liens; i++) {
            int src = r->liens[i].source;
            int dst = r->liens[i].destination;

            /* Ce lien concerne-t-il notre switch ? */
            if (src != ev->destinataire && dst != ev->destinataire) {
                continue;
            }

            /* On ne renvoie pas sur le port d'entrée */
            if (port != ev->port_arrivee) {
                int voisin = (src == ev->destinataire) ? dst : src;
                int port_chez_voisin = get_port(r, voisin, ev->destinataire);

                printf("  → Envoi sur port %d → nœud [%d]\n", port, voisin);

                Evenement nouveau = { ev->trame, ev->destinataire, voisin,
                                      (port_chez_voisin == -1) ? 0 : port_chez_voisin };
                enfiler(f, nouveau);
            }
            port++;
        }
    }
}

/* ─────────────────────────────────────────────
   Boucle principale de simulation
   ───────────────────────────────────────────── */

void simuler(Reseau *r, FileEvenements *f) {
    int iteration = 0;

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║        DÉMARRAGE DE LA SIMULATION    ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    while (!file_est_vide(f)) {
        iteration++;
        printf("┄┄┄ Itération %d ┄┄┄\n", iteration);

        /* On retire le premier événement de la file */
        Evenement ev = defiler(f);

        /* On le traite (peut ajouter de nouveaux événements dans la file) */
        traiter_evenement(r, &ev, f);
        printf("\n");
    }

    printf("╔══════════════════════════════════════╗\n");
    printf("║   SIMULATION TERMINÉE (%d itérations) ║\n", iteration);
    printf("╚══════════════════════════════════════╝\n");
}