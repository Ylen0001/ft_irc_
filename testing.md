    Tests IRC ----> IRSSI

### PASS

- Bon pass : OK
- Bad pass : OK (Précision : Lorsque l'on se connecte à IRSSI avec un bad pass, irssi programme une nouvelle tentative de connection 1m plus tard, même si entre temps on s'est connecté avec un bon pass. Donc la session s'interrompt)

### USER

- OK : Connexion simple avec IRSSI (PASS/NICK/USER).
- OK : Retenter USER après avoir déjà register (462)
- OK : Username déjà pris (Accepté)
- OK : USER avant PASS (Erreur 451)
- OK : USER sans real name "USER Foo 0 *" (461)
- OK : USER 0 * :Bar --> Valide
- OK : USER 0 * : (461)
- OK : USER 0 * : et espace (461)

### NICK 

- OK : Nick mauvaise synthaxe (+ de 9 caractères / caractères spéciaux)
- OK : Nick correct
- OK : Changement de NICK
- OK : Nick déjà pris
- OK : Si opérateur d'un channel change de NICK, il garde ses droits.
- OK : Un client nick1 se déconnecte, et un client2 prend son nick.

### QUIT 

- OK : QUIT sans argument
- OK : QUIT :Bye dans un channel (Le quit et le message sont bien broadcast aux autres membres du channel, et le client est bien removed des channels)
- OK : QUIT Bye dans un channel
- OK : QUIT :Bye et QUIT Bye sans être dans un channel (Pas de broadcast)
- OK : QUIT avant d'être fully registered
- OK : QUIT avant PASS
- OK : QUIT avec message de + de 512 Bytes
- PAS OK : Le PART n'est pas broadcast aux gens du channel avant le broadcast de QUIT
