# Projet SEE : Système de Mesure de Distance via HC-SR04

> Cible : **STM32MP157** | OS : **Buildroot** | Driver : **Kernel Space**

Ce projet implémente un système complet de mesure de distance ultrasonique sur une cible STM32MP157 exploitant la distribution Buildroot. L'architecture repose sur un **driver noyau** pour l'acquisition précise des données et un **dashboard web** pour l'analyse statistique en temps réel.

---

## Objectifs du Projet

* **Kernel Space** : Pilotage du capteur via un module noyau Linux dédié pour garantir sécurité, isolation et rapidité.
* **Précision Temporelle** : Utilisation d'interruptions et de `ktime` pour minimiser la gigue (*jitter*) lors de la mesure de l'écho.
* **Analyse Statistique** : Modélisation de la fiabilité du capteur par une distribution normale (Gaussienne) calculée sur une fenêtre glissante.

---

## Architecture Technique

### Couche Matérielle et Device Tree
Le capteur **HC-SR04** est interfacé avec la STM32MP1 via les connecteurs Arduino. Le **Device Tree (.dts)** a été modifié pour déclarer ces broches :

| Signal | Broche GPIO | Configuration |
| :--- | :--- | :--- |
| **TRIGGER** | `PE10` | Sortie |
| **ECHO** | `PG3` | Entrée |


## Guide d'Installation et Lancement

### Node
Avant de lancer les programmes, il faut créer le nœud de communication pour le driver :

```bash
mknod /dev/hcsr04 c 245 0
chmod 666 /dev/hcsr04
```

### Lancement du Serveur Django (PC/Host)

```bash
python manage.py runserver 0.0.0.0:8000
```

Exposer sur Internet :

```bash
ngrok http 8000
```

### Lancement du Bridge C (Cible)

```bash
./sensor_bridge
```
Le bridge va lire /dev/hcsr04, obtenir les nanosecondes, faire la conversion en cm, et envoyer le tout en UDP Broadcast sur le port 5000.

## Quick FIX :

### FIX Réseau & Accès SSH
Si le réseau n'est pas "UP" ou si l'accès SSH en root est rejeté :

```bash
killall dhcpcd
udhcpc -i eth0
```

#### Configurer l'accès SSH :

```bash
passwd root         
# Modifier /etc/ssh/sshd_config : 
# -> PermitRootLogin yes
# -> PasswordAuthentication yes
```

#### Relancer SSH
```bash
ssh-keygen -A        
kill $(pidof sshd)
/usr/sbin/sshd -D -e &
```

### FIX Ioctl
   
Pour que les modifications persistent après un flashage Buildroot, utiliser un overlay.

#### Créer le dossier d'overlay :

```bash
mkdir -p ~/buildroot-2025.02.7/board/stmicroelectronics/stm32mp157c-dk2/overlay/etc
```

#### Configurer le montage automatique (fstab) : Éditez overlay/etc/fstab pour monter /proc et /sys :

```bash
vi overlay/etc/fstab
```
Ajouter les 4 lignes suivantes
```
/dev/root    /         auto    rw         0    1
proc         /proc     proc    defaults   0    0
sysfs        /sys      sysfs   defaults   0    0
devtmpfs     /dev      devtmpfs defaults   0    0
```

L'ajout de devtmpfs permet à libgpiod d'accéder aux descripteurs de fichiers des GPIOs correctement.