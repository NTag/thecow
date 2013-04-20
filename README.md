COW : Chat Over the World
======

Initialement, COW a été conçu pour être un programme de chat ne nécessitant aucune installation, pour pouvoir être utilisé depuis son établissement scolaire.
Malheureusement, il utilise plusieurs ports >=10000, qui sont finalement souvent bloqués par les écoles. Il faudrait utiliser le port 80.

Il s'agit d'une architecture clients/serveur. Un programme de mise à jour est inclus. J'ai codé COW en C++ avec Qt 4.5.

Voici les sources, disponibles sous licence GNU/GPL.

#Requis :
Qt>=4.5

Normalement, ça compile correctement sous Windows, Linux et Mac.

Le fichier documentation.pdf détaille les protocoles que j'ai imaginés pour COW.
