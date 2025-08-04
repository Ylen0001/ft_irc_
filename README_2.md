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
- Ajout de _hostname (adresse IP client) dans la classe Client + getter/Setter
- Ajout de la méthode Client::getPrefix() pour le formatage des messages clients lorsqu'ils sont redispatché à d'autres clients.

18/06 :

- Création de la classe Channel
- Elle contient une map int fd/Client* comme db_clients, ce sera important de bien faire attention à enlever le client des channels concerné lorsqu'il se déconnecte/quit. Sinon les pointeurs seront nulls, et ça va poser des problèmes.
- Ajout d'une map string/Channel pour lister les channels dans Server.
- Construction de handleJOIN --> Bloc important
  - 1 - Vérifier si le client est registered.
  - 2 - Vérifier si l'arg est vide où si le format post cmd est bon.
  - 3 - Vérifier que le nom du Channel est valide "#" avant le nom.
  - 4 - Récupérer où créer le Channel.
  - 5 - Ajouter le client au Channel.
  - 6 - Notifier les clients du channel
  - 7 - Envoyer le topic/liste des users (À IMPLËMENTER)
- Méthode de debug pour afficher les channels (On en a chié putain)
  - Surcharge d'opérateur pour Channel
  - Getter pour la map _clients de Channel
  - Méthode d'affichage des channels dans Server
- Méthode Server::RemoveClientFromAllChannels
- Suppression d'un channel si plus aucun clients connectés.

19/06 :

- RemoveAllClientsFromChannels()
- HandlePART()
- HandleQUIT()

20/06 :

- HandlePRIVMSG
  Ici on est sur un gros morceau.
- On lit la cible avec ss >> target.
- On récupère ensuite tout le reste de la ligne avec getline(ss, restOfLine).
- Le message doit commencer par ':' (convention IRC), sinon erreur.
- On extrait le message après les ':'.
- Si la cible commence par #, c’est un channel, sinon un utilisateur.
- Pour les channels, on vérifie que le channel existe et que l’expéditeur y est inscrit.
- On envoie le message avec le format :prefix PRIVMSG target :message.
- Pour un utilisateur, on cherche par nickname et on envoie directement.
- En cas d’erreur, on renvoie un message d’erreur adapté.

21/06 :

- Handle PING/PONG
- Gestion des operators de channel. Dans la classe channel on a un set(int fd) _operators qui stocke les fd des operators pour chaque channel. Avec les getter/setter adaptés. J'ai également ajouté un checker bool IsOperator().
- Debug operator dans Channel << ajouté.
- HandleKICK (Gros morceau)
- HandleTOPIC
- HandleINVITE (Presque bon, il manque la partie à ajouter quand handleMODE sera ready)
- Ajout du set int _authorizedClients dans Channel, pour INVITE et MODE. + getter/setter/checker + Debug.

POUR LA PROCHAINE SESSION :

- Server::handleMODE()
- Terminer Server::handleINVITE()

04/07

- HandleMODE() En cours
- Gérer dans JOIN le flag du channel pour accepter un client.
- Dans debug afficher le mode du channel.
