#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reseau.h"

/* Parse une adresse MAC depuis une chaîne "aa:bb:cc:dd:ee:ff" */
static AdresseMAC parse_mac(const char *s) {
    AdresseMAC mac;
    sscanf(s, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac.octets[0], &mac.octets[1], &mac.octets[2],
           &mac.octets[3], &mac.octets[4], &mac.octets[5]);
    return mac;
}

/* Parse une adresse IP depuis une chaîne "a.b.c.d" */
static AdresseIP parse_ip(const char *s) {
    unsigned int a, b, c, d;
    sscanf(s, "%u.%u.%u.%u", &a, &b, &c, &d);
    return creer_ip((uint8_t)a, (uint8_t)b, (uint8_t)c, (uint8_t)d);
}

Reseau charger_reseau(const char *chemin) {
    Reseau r;
    r.nb_noeuds = 0;
    r.nb_liens  = 0;

    FILE *f = fopen(chemin, "r");
    if (!f) {
        fprintf(stderr, "Erreur : impossible d'ouvrir '%s'\n", chemin);
        return r;
    }

    /* ── Ligne d'en-tête ── */
    int nb_equip, nb_liens;
    fscanf(f, "%d %d\n", &nb_equip, &nb_liens);

    /* ── Équipements ── */
    for (int i = 0; i < nb_equip; i++) {
        char ligne[256];
        fgets(ligne, sizeof(ligne), f);

        /* Découpage par ';' */
        int    type_int = atoi(strtok(ligne, ";"));
        char  *mac_str  = strtok(NULL, ";");

        if (type_int == TYPE_SWITCH) {
            int   nb_ports  = atoi(strtok(NULL, ";"));
            int   priorite  = atoi(strtok(NULL, ";\n"));
            AdresseMAC mac  = parse_mac(mac_str);
            r.noeuds[i].type            = TYPE_SWITCH;
            r.noeuds[i].equipement.sw   = creer_switch(mac, nb_ports, priorite);
        } else {
            char  *ip_str  = strtok(NULL, ";\n");
            AdresseMAC mac = parse_mac(mac_str);
            AdresseIP  ip  = parse_ip(ip_str);
            r.noeuds[i].type                    = TYPE_STATION;
            r.noeuds[i].equipement.station       = creer_station(mac, ip);
        }
        r.nb_noeuds++;
    }

    /* ── Liens ── */
    for (int i = 0; i < nb_liens; i++) {
        int src, dst, poids;
        fscanf(f, "%d;%d;%d\n", &src, &dst, &poids);
        r.liens[i] = (Lien){src, dst, poids};
        r.nb_liens++;
    }

    fclose(f);
    return r;
}