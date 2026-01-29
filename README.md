Projet SEE : Système de Mesure de Distance via HC-SR04Ce projet implémente un système complet de mesure de distance ultrasonique sur une cible STM32MP157 exploitant la distribution Buildroot. 

L'architecture repose sur un driver noyau (Kernel Space) pour l'acquisition précise des données et un tableau de bord web pour l'analyse statistique en temps réel.

1. Objectifs du Projet

Kernel Space : Pilotage du capteur via un module noyau Linux dédié pour garantir sécurité, isolation et réactivité.

Précision Temporelle : Utilisation d'interruptions et de ktime pour minimiser la gigue (jitter) lors de la mesure de l'écho.

Analyse Statistique : Modélisation de la fiabilité du capteur par une distribution normale (Gaussienne) calculée sur une fenêtre glissante.

4. Architecture TechniqueA. Couche Matérielle et Device TreeLe capteur HC-SR04 est interfacé avec la STM32MP1 via les connecteurs Arduino :TRIGGER : GPIO PA12 (Configuré en sortie).ECHO : GPIO PA11 (Configuré en entrée avec support d'interruption).

Le Device Tree (.dts) a été modifié pour déclarer ces broches et permettre au driver de les revendiquer dynamiquement au chargement.

Conversion du temps en distance.

Transmission sécurisée de la donnée vers l'espace utilisateur via copy_to_user.C. 

Dashboard Web et Statistiques

L'interface frontend traite les données reçues pour fournir une analyse métrologique :

Traitement du Bruit : Calcul de la moyenne et de l'écart-type sur les 1000 derniers échantillons.

Visualisation Gaussienne : Génération d'une courbe de Gauss dynamique pour évaluer la dispersion des mesures. 

Une courbe étroite valide la précision de l'implémentation noyau.Alertes de Proximité : Jauge dynamique avec seuils de sécurité (Bleu / Orange / Rouge).
