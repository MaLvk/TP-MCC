# TP de Systèmes d'Acquisition et de Commande

#### VAN KETS Maëlle   HERAULT Félix

### Objectif Du TP:
- Réaliser un shell pour commander le hacheur
- Réaliser la commande des 4 transistors du hacheur en commande complémentaire décalée
- Faire l'acquisition des différents capteurs
- Réaliser l'asservissement, en temps réel, en courant et en vitesse.

On utilisera la configuration des pins suivantes :
![configuration des pins](https://user-images.githubusercontent.com/93315025/210317783-d0a380b7-1c48-44de-8739-24f10886b619.png "Configuration des pins")

## Shell de commande du hacheur
Le shell nous permet d'appeler differentes commandes qui permettent de choisir les differents paramètres à appliquer à la MCC.

Les differentes commandes sont:
- "start"         : démarre la PWM à envoyer au moteur.
- "stop"          : fait arreter le moteur.
- "init"          : initialise le hacheur, en enlevant le HAL_OverCurrent.
- "pinout"           : affiche le numero des differents pin nécéssaire à l'installation de la carte sur le hacheur.
- "help"          : affiche sur le terminale toutes les fonctions et leur utilité.
- "speed = XXXX"  : permet de mettre la valeur de la vitesse de la MCC, avec XXXX une valeur comprise entre 0 et 1023, 512 étant la valeur ou le moteur ne tourne pas.
- "get ADC"       : permet d'afficher les valeurs du courant toutes les secondes.
- "get pos"       : permet d'afficher les valeurs la vitesse (en rd/s) du moteur toutes les secondes.
- "current = X.X" : permet d'asservir le moteur en courant.

Le shell fonctionne bien, le seul problème rencontré a été pour effacer une valeur lorsque l'on écrit dessus.

## Commande des 4 transistors du hacheur en commande complémentaire décalé
### Génération des 4 PWM
Pour réaliser la commande des 4 transistors nous avons utilisé le timer 1 qui nous renvoie directement en complémentaire décalée 4 signaux de PWM. 

Nous devions respecter le cahier des charges suivant :
- Fréquence de la PWM : 16kHz.
- Temps mort minimum : 2us.
- Résolution minimum : 10bits.

Au vu de ce cahier des charges, nous avons donc configuré le Timer 1 avec les paramètres suivant :
- Source : Internal Clock.
- Prescaler : 9.
- ARR : 1023.
- DeadTime : 200.

Il ne faut pas oublier de mettre les "pulse" des deux channels à 512 (c'est-à-dire la moitié de 1024) pour ne pas faire tourner notre moteur au départ. Il faut aussi penser à activer l'auto-reload preload qui permet de finir de compter avant de changer du CCR.

La visualisation des 4 PWM à l'oscilloscope donne :
![pwm](https://user-images.githubusercontent.com/93315025/210318338-f4bd1e4e-3b92-44f0-9a45-1a71186bdec3.png "4 PWM en commande complémentaire décalée")

Nous obtenons donc bien nos 4 PWM en complémentaire décalée.

La commande "start" dans le shell ou l'appui sur le bouton bleu permettra de démarrer les 4 PWM.

### Commande du hacheur
Après lecture de la documentation, nous savons comment brancher les pins du hacheur avec ceux de la carte STM32. Nous avons choisis les phases Yellow et Red.
![image](https://user-images.githubusercontent.com/93315025/210566867-9b4654bc-0e96-4144-998e-0cb4a260e89e.png)
- 12 : Yellow Phase Top
- 13 : Red Phase Top
- 30 : Yellow Phase Bottom
- 31 : Red Phase Bottom
- 33 : Fault Reset Command
- 16 : Yellow Phase Hall Current
- A et B sont les deux pistes de l'encodeur.

Les pins 12, 13, 30 et 31 servent pour envoyer les PWM au hacheur et ainsi commandé les transistors. La pin 16 nous servira plus tard pour obtenir une mesure de courant et le pin 33 permet d'enlever les sécurités de l'onduleur en passant la pin ISO_RESET à l'état haut pendant 2us au minimum.

La commande "pinout" du shell permet de résumer ces différents branchements. La commande "init" permet quand à elle de réaliser la séquence d'amorçage du hacheur en venant toggle la pin ISO_RESET.

### Vitesse et tests
Nous avons donc créer une commande "speed = XXXX" qui prend une valeur entre 0 et 1023 correspondant au rapport cyclique pour contrôler la vitesse du moteur.

Nous avons donc réaliser différents tests dans l'ordre suivant :
- Rapport cyclique de 50% : le moteur est à l'arrêt.
- Rapport cyclique de 60% : le moteur tourne dans un sens.
- Rapport cyclique de 100% : le moteur est bloqué par le hacheur qui se met en "HALL OVER CURRENT". 
- Rapport cyclique de 0% : le moteur est bloqué par le hacheur qui se met en "HALL OVER CURRENT".

Nous remarquons donc que si on effectue une trop grande variation de vitesse, le hacheur se bloque en "HALL OVER CURRENT" car il y a eu un pic de courant. C'est pourquoi nous allons essayer lors de ce TP d'asservir le moteur en courant et en vitesse.

## Acquisition des différents capteurs
### Capteurs courants
Lorsque l'on observe à l'oscilloscope les tensions obtenues aux pins 16 et 35 du connecteur 37 broches, on obtient une tension de 2.53V lorsque le moteur est à l'arrêt. Si on commence à tourner, une des deux phases va augmenter pendant que l'autre va diminuer. L'addition des deux valeurs donnera constamment 5V.

Pour la mesure du courant, on configure un ADC pour faire la mesure d'une des deux valeurs de courant puis on met en place un DMA pour qu'il transmette ces deux valeurs à un buffer donné. La commande "get ADC" permet d'afficher les valeurs du courants toutes les secondes dans le shell.

### Capteur position

On configure le timer 3 en encodeur mode pour récupérez à intervalle de temps régulier la position de votre moteur. On obtient la vitesse du moteur en faisant la différence de position entre 2 instants. Pour éviter de dépasser les valeurs maximales et minimales de notre compteur, on fixe la valeur du compteur à VALEUR_MAX/2 après chaque mesure.

La commande "get pos" permet d'afficher les valeurs de la vitesse en rad/s toutes les secondes dans le shell.


Nous avons pris la liberté de rajouter une commande "quit" permettant de quitter les affichages toutes les secondes (de vitesse ou de courant) pour pouvoir écrire tranquillement nos commandes dans le shell.

## Asservissement en courant et en vitesse 
### En courant
L'asservissement en courant a été effectué. A l'aide de la commande "current = X.X", on peut faire tourner le moteur pour qu'il ait un courant de X.X A. 

### En vitesse
Nous n'avons malheureusement pas eu le temps de réaliser l'asservissement en vitesse.


