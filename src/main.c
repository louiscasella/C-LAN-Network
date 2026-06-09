#include <stdio.h>
#include <string.h>
#include "reseau.h"
#include "trame.h"
#include "scheduler.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <fichier.lan>\n", argv[0]);
        return 1;
    }

    /* Chargement du réseau */
    Reseau r = charger_reseau(argv[1]);
    afficher_reseau(r);
    printf("\n");

    /* Cherche automatiquement la première et la deuxième station */
    int idx_st1 = -1, idx_st2 = -1;
    for (int i = 0; i < r.nb_noeuds; i++) {
        if (r.noeuds[i].type == TYPE_STATION) {
            if (idx_st1 == -1)      idx_st1 = i;
            else if (idx_st2 == -1) idx_st2 = i;
        }
    }
    if (idx_st1 == -1 || idx_st2 == -1) {
        fprintf(stderr, "Erreur : pas assez de stations dans ce réseau\n");
        return 1;
    }
    printf("Stations trouvées : [%d] et [%d]\n\n", idx_st1, idx_st2);

    /* ── Envoi 1 : st1 → st2 (table vide, va flooder) ── */
    FileEvenements file;
    file_init(&file);

    uint8_t msg[] = "Bonjour !";
    lancer_envoi(&r, &file, idx_st1, idx_st2,
                 ETHER_TYPE_IPv4, msg, strlen((char*)msg));
    simuler(&r, &file);

    /* Affichage des tables après le premier envoi */
    printf("\n=== Tables de commutation après le 1er envoi ===\n");
    for (int i = 0; i < r.nb_noeuds; i++) {
        if (r.noeuds[i].type == TYPE_SWITCH) {
            printf("Switch [%d] :\n", i);
            afficher_table_commutation(r.noeuds[i].equipement.sw.table);
        }
    }

    /* ── Envoi 2 : st2 → st1 (le switch va apprendre st2, puis router vers st1) ── */
    printf("\n=== Envoi retour : st2 → st1 ===\n");
    file_init(&file);
    lancer_envoi(&r, &file, idx_st2, idx_st1,
                 ETHER_TYPE_IPv4, msg, strlen((char*)msg));
    simuler(&r, &file);

    printf("\n=== Tables de commutation finales ===\n");
    for (int i = 0; i < r.nb_noeuds; i++) {
        if (r.noeuds[i].type == TYPE_SWITCH) {
            printf("Switch [%d] :\n", i);
            afficher_table_commutation(r.noeuds[i].equipement.sw.table);
        }
    }

    return 0;
}