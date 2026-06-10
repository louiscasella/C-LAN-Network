#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

#define TAILLE_LIGNE 256

static void lire_mac(const char *chaine, AdresseMAC *mac)
{
    unsigned int o[6];
    sscanf(chaine, "%x:%x:%x:%x:%x:%x",
           &o[0], &o[1], &o[2], &o[3], &o[4], &o[5]);
    mac->octets[0] = (uint8_t)o[0]; mac->octets[1] = (uint8_t)o[1];
    mac->octets[2] = (uint8_t)o[2]; mac->octets[3] = (uint8_t)o[3];
    mac->octets[4] = (uint8_t)o[4]; mac->octets[5] = (uint8_t)o[5];
}

static void lire_ip(const char *chaine, AdresseIP *ip)
{
    unsigned int o[4];
    sscanf(chaine, "%u.%u.%u.%u", &o[0], &o[1], &o[2], &o[3]);
    ip->octets[0] = (uint8_t)o[0]; ip->octets[1] = (uint8_t)o[1];
    ip->octets[2] = (uint8_t)o[2]; ip->octets[3] = (uint8_t)o[3];
}

int charger_reseau(const char *nom_fichier, Reseau *r)
{
    FILE *f;
    char ligne[TAILLE_LIGNE];
    int nb_equip, nb_liens;
    int i;
    char *tok;

    f = fopen(nom_fichier, "r");
    if (f == NULL) {
        printf("Erreur : impossible d'ouvrir '%s'\n", nom_fichier);
        return 0;
    }

    *r = creer_reseau_vide();

    fgets(ligne, TAILLE_LIGNE, f);
    sscanf(ligne, "%d %d", &nb_equip, &nb_liens);

    for (i = 0; i < nb_equip; i++) {
        int type;
        AdresseMAC mac;

        fgets(ligne, TAILLE_LIGNE, f);
        ligne[strcspn(ligne, "\n")] = '\0';

        tok  = strtok(ligne, ";");
        type = atoi(tok);

        if (type == TYPE_SWITCH) {
            int nb_ports, priorite;
            tok = strtok(NULL, ";"); lire_mac(tok, &mac);
            tok = strtok(NULL, ";"); nb_ports = atoi(tok);
            tok = strtok(NULL, ";"); priorite = atoi(tok);
            ajouter_switch(r, creer_switch(mac, nb_ports, priorite));
        } else {
            AdresseIP ip;
            tok = strtok(NULL, ";"); lire_mac(tok, &mac);
            tok = strtok(NULL, ";"); lire_ip(tok, &ip);
            ajouter_station(r, creer_station(mac, ip));
        }
    }

    for (i = 0; i < nb_liens; i++) {
        int n1, n2, poids;
        fgets(ligne, TAILLE_LIGNE, f);
        ligne[strcspn(ligne, "\n")] = '\0';
        sscanf(ligne, "%d;%d;%d", &n1, &n2, &poids);
        ajouter_lien(r, n1, n2, poids);
    }

    fclose(f);
    return 1;
}
