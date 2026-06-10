#include <stdio.h>
#include <string.h>
#include "trame.h"

Trame creer_trame(AdresseMAC src, AdresseMAC dst,
                  uint16_t type, const char *message)
{
    Trame t;
    int i, len;

    for (i = 0; i < 7; i++) t.preambule[i] = 0xAA;
    t.sfd         = 0xAB;
    t.destination = dst;
    t.source      = src;
    t.type        = type;

    len = strlen(message);
    if (len > TAILLE_MAX_DONNEES) len = TAILLE_MAX_DONNEES;
    memcpy(t.donnees, message, len);
    t.taille_donnees = len;

    /* Bourrage si données trop courtes */
    if (t.taille_donnees < TAILLE_MIN_DONNEES) {
        memset(t.donnees + t.taille_donnees, 0,
               TAILLE_MIN_DONNEES - t.taille_donnees);
        t.taille_donnees = TAILLE_MIN_DONNEES;
    }

    t.fcs[0] = t.fcs[1] = t.fcs[2] = t.fcs[3] = 0x00;
    return t;
}

void afficher_trame(Trame *t)
{
    printf("  Source      : "); afficher_mac(t->source);      printf("\n");
    printf("  Destination : "); afficher_mac(t->destination); printf("\n");
    printf("  Type        : 0x%04X", t->type);
    if      (t->type == 0x0800) printf(" (IPv4)");
    else if (t->type == 0x0806) printf(" (ARP)");
    else if (t->type == 0x86DD) printf(" (IPv6)");
    printf("\n");
    printf("  Donnees     : \"%s\"\n", (char *)t->donnees);
}

void afficher_trame_hex(Trame *t)
{
    int i;
    printf("  [Preambule] ");
    for (i = 0; i < 7; i++) printf("%02X ", t->preambule[i]);
    printf("\n  [SFD]       %02X\n", t->sfd);
    printf("  [DST]       ");
    for (i = 0; i < 6; i++) printf("%02X ", t->destination.octets[i]);
    printf("\n  [SRC]       ");
    for (i = 0; i < 6; i++) printf("%02X ", t->source.octets[i]);
    printf("\n  [Type]      %02X %02X\n",
           (t->type >> 8) & 0xFF, t->type & 0xFF);
    printf("  [Donnees]   ");
    for (i = 0; i < t->taille_donnees; i++) {
        printf("%02X ", t->donnees[i]);
        if ((i + 1) % 16 == 0) printf("\n              ");
    }
    printf("\n  [FCS]       %02X %02X %02X %02X\n",
           t->fcs[0], t->fcs[1], t->fcs[2], t->fcs[3]);
}
