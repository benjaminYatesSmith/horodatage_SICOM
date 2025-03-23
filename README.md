# horodatage_SICOM/Imagerie en Temps GPS

## Description du projet
Ce projet vise à horodater avec précision des images numériques à l'aide du temps GPS, avec une précision d'1 milliseconde. L'objectif est d'implémenter un système combinant un microcontrôleur STM32 et un module GPS pour récupérer l'heure et l'afficher sur un écran LED. Une caméra capturera cette horloge et générera automatiquement un fichier de sous-titrage SRT.

## Organisation du dépôt
Le dépôt est structuré en deux répertoires principaux :

- **`raspberry_pi/`** : Contient le code Python pour le traitement des images et la gestion de la synchronisation avec le temps GPS.
- **`STM32/`** : Contient le code en C développé sur STM32CubeIDE pour l'acquisition du temps GPS et son affichage sur LED.

## Technologies utilisées
- STM32CubeIDE pour le développement embarqué sur STM32
- Python pour le traitement des données sur Raspberry Pi
- OpenCV pour la gestion des images et vidéos
- Protocoles GPS (NMEA, PPS) pour la synchronisation temporelle

## Membres du projet
- Gaspard Botté
- Benjamin Yates-Smith
- Eliott Canville
- Ethan Bareille

## Instructions d'utilisation
1. Déployer le code STM32 pour récupérer le temps GPS et l'afficher sur un écran LED.
2. Lancer le script Python sur Raspberry Pi pour capturer la vidéo et générer le fichier de sous-titrage.
3. Synchroniser les données et analyser la précision de l'horodatage.

## Références
- [Synchronisation GPS](https://timetoolsltd.com/gps/what-is-the-gps-clock/)
- [Formats de fichiers SRT](http://nicolas.anquetil1.free.fr/sous-titressrt.html)
