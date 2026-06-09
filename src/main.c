#include <stdio.h>
#include "reseau.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <fichier.lan>\n", argv[0]);
        return 1;
    }

    Reseau r = charger_reseau(argv[1]);
    afficher_reseau(r);

    return 0;
}