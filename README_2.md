## _______     DIARY    ________

15/07 :

- Getter manquants dans Client/Server.
- Implementation du debug (Surcharge d'operateur dans Server/Client + printConnectedClients).
- Ajout des commandes handleNICK/handleUSER/handleREALNAME. Met bien à jour les variables dans le client concerné

+ registered si jamais toutes les infos nécessaires ont été rentrées.

Notion interessantes abordees : getter de map/vector.

16/07

- Handle le password pour lancer l'exécutable et se connecter au serveur.
- Création de la variable _password dans Server + getter/setter.
- Retouche des commandes NICK/USER pour faire en sorte qu'elle ne s'exécute que si le password a été successfully entered.
- Implémentation de Server::handlePASS().
- Implémentation de Server::sendToClient(). --> Boucle de send pour s'assurer de bien envoyer la totalité d'un message
- Mise à jour des fonctions handleNICK/USER
- Gestion du control+C --> extern volatile sig_atomic_t g_running dans le Server.hpp, puis définition dans le main.cpp, et dans Server::run while(true)    --> while(g_running)

17/06 :

- Nickname's already used case
- Passage en mode non bloquant pour les sockets serveur/clients ---> cf Mode Bloquant/Non-bloquant et Multiplexage de poll() dans README.md
