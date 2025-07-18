# FT_IRC

### I - Internet Relay Chat

Il s'agit d'un protocole de communication textuelle en temps réel. Salons de discussions, conversations privées.

### II - Fonctionnement de base

Client-Serveur : L'utilisateur utilise un client IRC (ex : *HexChat*, *mIRC*) pour se connecter à un serveur IRC (ex : irc.libera.chat).

Canaux de discussion : Une fois connecté, l'user peut join des channels (#linux).

Nickname : Chaque user choisit un pseudo, il peut être protégé par un service d'identification (*Nickserv*)).

### III - Architecture technique

- Les serveurs IRC sont interconnectés en réseau (ex : réseau *Libera*, *EFnet*).
- Chaque serveur relaye les messages aux autres serveurs et clients connectés.
- IRC fonctionne sur le port **6667** (par défaut), et **6697** pour une connexion sécurisée (SSL/TLS).

### IV - Structure d'un message IRC

```cpp
[:prefix] COMMAND param1 param2 ... [:last parameter with spaces]

```

- prefix (optionnel) : indique l'expéditeur (ex : nick!user@host.)
- COMMAND : commande IRC (NICK, JOIN, PRIVMSG, etc.)
- params : paramètres séparés par des espaces
- : Indique que le reste de la ligne est un seul paramètre (avec espaces).

```cpp
Ex : :nick!user@host PRIVMSG #channel :Hello everyone!

```

### V - Processus de connexion (Selon la RFC)

Un client IRC doit s'enregister avec la séquence de requests :

NICK `<nickname>`

USER `<username> <hostname> <servername>` :`<realname>`

Le serveur n'autorise pas d'autres commandes tant que le client n'a pas envoyé NICK et USER.

### VI - Réponses du serveur (numeric replies)

Le serveur envoie des codes numériques comme réponses (ex: 001, 433, etc.)

- 001 : Bienvenue (connexion réussie)
- 433 : Nickname déjà utilisé
- PING / PONG : Vérification de connectivité
- 401 : No such nick/channel
- 331 / 332 : Sujet du canal (topic)
- 353 / 366 : LIstes des users d'un canal
- 403 : No such channel

### VII - Format Réseau

Les messages échangés doivent être terminés par \r\n, inclus, dans une limite de 512 caractères.

### IX - Les sockets (IMPORTANT)

Un socket est une interface logicielle pour communiquer à travers un réseau. Dans irc, on crée un serveur TCP qui acceptes plusieurs connexions clients IRC. En C++ un socket est représenté par un fd.

Les 5 étapes :

- Créer le socket : socket() --> Crée la prise réseau (Non connectée).
- Lier : bind() --> Associe IP + port au socket.
- Écouter : listen() --> Attend des connecionx entrantes.
- Accepter : accept() --> Accepte une connexion client (donne un nouveau fd).
- Communiquer : recv() / send() --> Envoie ou reçoit des messages.

___________ BUILD THE THING___________

fonction int socket(int domain, int type, int protocol)

- domain : AF_INET -> IPv4
- type : Type de socket SOCK_STREAM -> TCP (Fiable, orienté connexion.)
- protocol : 0 (Le système choisit le protocole par défaut pour le couple domain/type).

fonction int bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen)

struct sockaddr {
               sa_family_t sa_family;
               char        sa_data[14];
           }

Class serveur :

vect de clients, vect de channels

client IRSSI -> authentification.

fonctions : socket(), bind() -> accorder aux sockets le port sur la machine, handle erreurs. (ports supérieurs à 6000).

Penser à fermer le socket dans le destructeur.

Constructeur qui gère la création de socket, bind.

1 - build la classe server

2 - Démarrer un serveur

3 - Ouvrir un serveur sur un port défini

l'exec prend un port et un mdp.

4 - test avec netcat (utilitaire command line).

_________ Résumé _________

### I - Les sockets

Pour irc, on va donc utiliser la fonction socket(), afin de créer un socket serveur (Dans les paramêtres de la fonction, on décide du type de serveur.).

Donc on crée un socket serveur, qu'on bind() ensuite a un numéro de port du PC. Une fois que c'est fait, on envoie le serveur à la fonction listen(), une façon de dire "Le serveur est prêt à accepter de nouveaux clients".

Dés que listen observe une information sur le socket serveur, on décide ou pas d'accepter la connexion entrante avec la fonction accept(). Via accept() on crée un nouveau socket client qui représente la connexion entre le serveur et CE client précis. On enregistre ce client dans un fd, pour pouvoir lui parler ensuite.

**On l'enregistre dans un vecteur de fd _clients. Puis, pour l'ajouter au vecteur _pollfds afin de pouvoir le surveiller, on va créer une struct pollfd. pollfd.fd = _clients[i], pollfd.event = POLLIN (Si on souhaite surveiller des messages entrants), et ensuite on va faire _pollfds.push_back(_clients[i]).**

### II - La fonction poll()

Cette fonction prend en paramètres un vecteur de struct pollfd.

```cpp
struct pollfd {
    int fd;         // Le file descriptor à surveiller (socket)
    short events;   // Ce que tu veux surveiller (ex: POLLIN)
    short revents;  // Ce qui s'est réellement passé (rempli par poll())
};
```

Dans ce vecteur, on va créer une struct pollfd pour chaque socket, serveur et client. Et on va l'envoyer à la fonction, qui va surveiller chaque socket.

a - Return values de Poll()

- Si > 0 : Nombre de sockets concerné par des évènements.
- Si 0 : Aucun évènement détecté.
- Si -1 : Erreur

b - Évènements

- pollfds.events -> Il s'agit du type d'évènement que l'on souhaite surveiller. Ex : Si on fait pollfds[2].events = POLLIN (| POLLOUT si on souhaite surveiller plusieurs types d'events), on précise qu'on veut surveiller un message entrant sur ce socket.
- pollfds.revents -> Il s'agit du type d'évènement qui est détecté sur le serveur.

c - POLLIN

- Le flag POLLIN est un état qui signifie que des données sont prêtes à être lues sur le socket concerné.
- Dans le cas d'un socket serveur il signifie qu'une nouvelle connexion entrante est détectée.
- Dans le cas d'un socket client, que ce dernier a envoyé un nouveau message.
- POLLIN étant un flag, un revent peut-être POLLIN et POLLHUP par exemple. Le client peut avoir envoyé un message ET s'être deconnecté,
- Ne posons pas de question, ça saoule leur connerie de gestion des états.

### III - Serveur minimal Echo

On a crée un serveur minimal echo, qui accepte bien la connexion d'un client, et lui renvoie son message.

Donc

- On gère bien la création + binding + écoute du socket Serveur.
- On accepte bien les nouveaux clients.
- On enregistre le fd de chaque nouveau client dans notre vecteur _db_clients.
- On détecte bien les évènement POLLIN.
- On renvoie bien le même message.

Désormais il faut passer à la suite.

### IV - Parser de message IRC

Notre serveur doit

- Comprendre les commandes du client (NICK, USER, JOIN, PRIVMSG, etc).
- Répondre en respectant le protocole IRC (RFC 2812)
- Garantir qu'un client n'ayabnt pas encore envoyé NICK/USER ne peut rien faire d'autre.

Step 1 :

Implémenter le parser de lignes IRC (terminées par \\r\n).

- Le buffer peut accumuler les données reçues
- Découpe en lignes complètes (ex: "NICK John\r\nUSER J 0 * :RealName\r\n" = 2 messages.)
- Stocke les messages prêts dans une file pour le traitement.

Step 2 :

Créer une fonction handleMessage(Client&, const std::string&)

- Cette fonction lira la commande (NICK, USER, etc.) et appliquera la logique IRC.

Step 3 :

Faire une gestion minimale du protocole IRC en implémentant

- NICK
- USER
- PING/PONG
- QUIT

### V - Mode non bloquant et Multiplexage

Mode non-bloquant : Par défaut, lors de l'appel de socket(), le fd est mis en mode bloquant. Ce qui signifie par exemple que recv() bloquera l'exécution du programme en attendant qu'il y ait une donnée à lire. Par défaut, poll() "prévient" ce problème par l'utilisation de mutliplexage, cependant cela reste une sécurité de passer les sockets/fd en mode non-bloquant. Comme ça, si rien n'arrive sur le fd, recv renvoie -1 et passe à autre chose.


Multiplexage : Imaginons qu'on a 1000 clients à surveiller. On ne pourrait pas les tester un par un pour vérifier si un évènement a eu lieu dessus, ni même utiliser un thread par client, ce serait beaucoup trop couteux. Donc poll() utilise une boucle qui va passer à travers chaque client/server, si il détecte un event, le notifie, et passe au prochain. 

### XXX - ASTUCES

- Si on utilise une boucle en C++, c'est que le code n'est pas optimisé. On peut toujours utiliser des .remove, .find etc...
- Ne pas hésiter à define des alias pour améliorer la lisibilité du code, ex : #defineClientMap std::unordered_map<int, Client>.
- Limiter au maximum les niveaux d'indentations dans une fonction. C'est souvent non nécessaire, et preuve que le code n'est pas optimal.
- Le keyword continue au sein d'une boucle veut dire "Sort de la boucle, et continue".
- L'utilisation de stringstream plutôt que std::string : Nous permet d'utiliser des fonctions pour manipuler la chaîne de caractère plus facilement, par
  exemple ss() qui renvoie le premier mot seul, et qui garde en mémoire la position de la tête de lecture. Donc si on ré-utilise ss, on récupère le second mot.
