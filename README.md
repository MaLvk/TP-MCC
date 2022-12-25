# TP de Systèmes d'Acquisition et de Commande

#### VAN KETS Maëlle   HERAULT Félix

### Objectif Du TP:

- Réaliser un shell pour commander le hacheur
- Réaliser la commande des 4 transistors du hacheur en commande complémentaire décalée
- Faire l'acquisition des différents capteurs
- Réaliser l'asservissement, en temps réel, en courant et en vitesse.


## Shell de commande du hacheur

Le shell nous permet d'appeler differentes commandes qui permettent de choisir les differents paramètres à appliquer à la MCC.

Les differentes commandes sont:
- "start"         : démarre la PWM à envoyer au moteur.
- "stop"          : fait arreter le moteur.
- "init"          : initialise le hacheur, en enlevant le HAL_OverCurrent.
- "pin"           : affiche le numero des differents pin nécéssaire à l'installation de la carte sur le hacheur.
- "help"          : affiche sur le terminale toutes les fonctions et leur utilité.
- "speed = XXXX"  : permet de mettre la valeur de la vitesse de la MCC, avec XXXX une valeur comprise entre 0 et 1023, 512 étant la valeur ou le moteur ne tourne pas.

Le shell fonctionne bien, le seul problème rencontré a été pour effacer une valeur lorsque l'on écrit dessus.

## Commande des 4 transistors du hacheur en commande complémentaire décalé

Pour réaliser la commande des 4 transistors nous avons utiliser le timer .... qui nous renvoie directement en complementaire décalée 4 signaux de PWM.

## Acquisition des differents capteurs



## Asservissement en courant et en vitesse 



