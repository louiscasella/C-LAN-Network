#ifndef TRAME_H
#define TRAME_H

#include <stdint.h>
#include "reseau.h"

#define TAILLE_MAX_DONNEES 1500
#define TAILLE_MIN_DONNEES   46

typedef struct {
    uint8_t    preambule[7];
    uint8_t    sfd;
    AdresseMAC destination;
    AdresseMAC source;
    uint16_t   type;
    uint8_t    donnees[TAILLE_MAX_DONNEES];
    int        taille_donnees;
    uint8_t    fcs[4];
} Trame;

Trame creer_trame(AdresseMAC src, AdresseMAC dst,
                  uint16_t type, const char *message);
void  afficher_trame(Trame *t);
void  afficher_trame_hex(Trame *t);

#endif /* TRAME_H */
