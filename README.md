# SAE 2.3 — Simulateur de réseau local en C

**IUT Robert Schuman — Département Informatique**

Projet réalisé en binôme. Ce simulateur modélise un réseau local Ethernet avec commutation de trames et protocole STP, à l'aide d'un ordonnanceur à événements discrets.

---

## Compilation et lancement

```bash
make
./simulateur
```

Les fichiers `.lan` doivent se trouvent dans le fichier lan.

---

## Architecture du projet

| Fichier | Rôle |
|---|---|
| `reseau.h/c` | Structures de données : AdresseMAC, AdresseIP, Station, Switch, Reseau |
| `config.h/c` | Chargement d'un réseau depuis un fichier `.lan` |
| `trame.h/c` | Structure d'une trame Ethernet, affichage lisible et hexadécimal |
| `simulateur.h/c` | Ordonnanceur à événements discrets, commutation de trames |
| `stp.h/c` | Protocole STP : échange de BPDUs, élection de la racine, blocage des ports |
| `main.c` | Scénarios de simulation |

---

## L'ordonnanceur à événements discrets

Le simulateur fonctionne comme ns3 : le temps n'avance pas en continu mais par **sauts d'événement**. Une file d'événements triée par temps contient les actions à effectuer. À chaque tour de boucle, l'événement le plus tôt est dépilé et traité. Si cet événement génère de nouveaux événements (ex : un switch qui retransmet une trame), ils sont insérés dans la file avec un temps futur égal à `temps_actuel + poids_du_lien`.

Trois types d'événements coexistent dans la même file :
- `EVT_ENVOYER_TRAME` : une station initie un envoi
- `EVT_RECEVOIR_TRAME` : un switch ou une station reçoit une trame
- `EVT_RECEVOIR_BPDU` : un switch reçoit un BPDU STP

---

## Format des fichiers `.lan`

```
<nb_equipements> <nb_liens>
2;<mac>;<nb_ports>;<priorite>     # switch
1;<mac>;<ip>                       # station
...
<indice1>;<indice2>;<poids>        # lien
```

Les switchs sont toujours listés en premier, suivis des stations.
Poids des liens : 10 Mb/s = 100, 100 Mb/s = 19, 1 Gb/s = 4.

---

## Scénarios

Pour lancer un scénario, décommenter la ligne correspondante dans le `main()` de `main.c`.

---

### Scénario 0 — Affichage d'un fichier de configuration

```c
scenario_0_afficher_config("mylan.lan");
```

Charge un fichier `.lan` et affiche le réseau : liste des switchs et stations avec leurs adresses MAC/IP, et liste des liens avec leurs poids. Permet de vérifier que le parseur lit correctement le fichier.

---

### Scénario 1 — Diffusion puis unicast

**Fichier :** `mylan_no_cycle.lan` (7 switchs, 8 stations, sans cycle)

```c
scenario_1();
```

- **t=0** : la station [7] envoie une trame à la station [8]. Les tables de commutation sont vides, le switch diffuse sur tous ses ports *(flood)*.
- **t=20** : la station [8] répond à [7]. Les switchs ont appris la MAC de [7] pendant le premier envoi : la réponse est acheminée **directement** sans diffusion.

Démontre le mécanisme d'apprentissage de la table de commutation et la transition diffusion → unicast.

---

### Scénario 2 — Deux envois simultanés

**Fichier :** `my_better_lan.lan` (3 switchs, 2 stations)

```c
scenario_2();
```

- **t=0** : la station [3] envoie à [4] **et** la station [4] envoie à [3] au même instant.

Démontre que l'ordonnanceur gère correctement plusieurs événements au même temps `t` et que les deux trames progressent en parallèle dans le réseau.

---

### Scénario 3 — Affichage hexadécimal d'une trame

```c
scenario_3();
```

Crée une trame Ethernet et l'affiche de deux façons :
- **Affichage lisible** : source, destination, type de protocole, données
- **Affichage brut** : contenu octet par octet en hexadécimal, champ par champ (préambule, SFD, adresses, type, données, FCS)

Démontre la conformité de la structure `Trame` avec le format réel d'une trame Ethernet.

---

### Scénario 4 — Tempête de broadcast sans STP

**Fichier :** `mylan.lan` (7 switchs interconnectés avec plusieurs cycles)

```c
scenario_4_tempete_broadcast();
```

- **t=0** : une station envoie une seule trame ARP broadcast (`FF:FF:FF:FF:FF:FF`).
- Sans STP, les cycles dans le réseau font reboucler la trame indéfiniment. Le nombre d'événements dans la file **double à chaque instant** jusqu'à saturation complète.

Démontre le problème des boucles de niveau 2 et justifie l'existence du protocole STP.

---

### Scénario 5 — Convergence STP (réseau simple)

**Fichier :** `my_better_lan.lan` (3 switchs, 1 cycle)

```c
scenario_5_stp();
```

**Phase 1 — Convergence STP :**
- Chaque switch se croit racine et envoie ses BPDUs initiaux.
- Les switchs comparent les BPDUs reçus avec leurs informations courantes. Si un BPDU annonce une meilleure racine (priorité plus basse, ou à égalité MAC plus basse), le switch met à jour ses informations et propage le BPDU à ses voisins.
- La propagation s'arrête naturellement quand plus aucun BPDU n'apporte d'information meilleure : la file se vide.

**Résultat :** switch [2] (priorité 256, la plus basse) est élu racine. Le port redondant de switch [0] vers switch [2] est bloqué.

**Phase 2 — Envoi broadcast après STP :**
- La même trame broadcast qu'au scénario 4 est envoyée. Cette fois le port bloqué rejette la trame et **la boucle n'a pas lieu**.

---

### Scénario 6 — STP sur réseau complexe + échange unicast

**Fichier :** `mylan.lan` (7 switchs, 3 cycles imbriqués)

```c
scenario_6_stp_multi_cycles();
```

**Phase 1 — Convergence STP :**
Switch [0] (MAC `01:45:23:A6:F7:01`, la plus basse) est élu racine. Trois ports sont bloqués pour casser les trois cycles :

| Switch | Port bloqué | Vers | Raison |
|---|---|---|---|
| sw[4] | port 1 | sw[2] | sw[2] a un coût vers racine plus faible (4 < 8) |
| sw[5] | port 1 | sw[1] | même coût (4=4), MAC de sw[1] plus basse |
| sw[6] | port 1 | sw[3] | même coût (8=8), MAC de sw[3] plus basse |

**Phase 2 — Envoi unicast st[7] → st[14] :**
- Première trame diffusée (MACs inconnues), bloquée aux ports STP, arrive à destination sans boucle.

**Phase 3 — Réponse unicast st[14] → st[7] :**
- Chaque switch sur le chemin connaît déjà la MAC destination grâce à la phase 2 : **transfert direct à chaque saut**, aucune diffusion.

---

### Scénario 7 — Apprentissage progressif de la table

**Fichier :** `mylan_no_cycle.lan`

```c
scenario_7_apprentissage();
```

Quatre envois successifs pour montrer l'évolution de l'apprentissage :

| Instant | Envoi | Comportement |
|---|---|---|
| t=0 | st[7] → st[8] | Table vide : **diffusion** |
| t=20 | st[8] → st[7] | MAC de st[7] connue : **unicast direct** |
| t=40 | st[7] → st[8] | Les deux MACs connues : **unicast direct** dans les deux sens |
| t=60 | st[9] → st[7] | Nouvelle station inconnue : **diffusion** puis apprentissage |

En fin de simulation, les tables de commutation de tous les switchs sont affichées pour montrer exactement ce que chaque switch a appris.

---

## Poids des liens STP

| Débit | Poids |
|---|---|
| 10 Mb/s | 100 |
| 100 Mb/s | 19 |
| 1 Gb/s | 4 |