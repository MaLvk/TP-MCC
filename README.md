# Compte-rendu Travaux Pratique : Commande MCC basique avec une carte Nucleo-STM32G474RE 
par VAN KETS Maëlle et HERAULT Félix

## Paramétrage des timers pour générer 4 PWM
**Cahier des charges :**
* Fréquence de la PWM : 16kHz
* Temps mort minimum : 2us
* Résolution minimum : 10 bits

**Détermination ARR et PSC :**  
$$\frac{f_{TIM}}{f_{PWM}}=(PSC+1)(ARR+1)$$ avec $f_{TIM}=160MHz$ et $f_{PWM}=16kHz$
$$10^4=(PSC+1)(ARR+1)$$
Or on souhaite une résolution de minimum 10 bits : $ARR>2^{10}=1024$  
On pose donc : $ARR=1999$. Donc on obtient : $PSC=7$.

**Détermination du temps mort :**  
