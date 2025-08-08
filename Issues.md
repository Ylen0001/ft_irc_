# ISSUES LIST

### IRSSI :

- IRSSI a des comportements importants à relever pour ne pas être induit en erreur
- Si dans IRSSI je tente de join un channel en +i, sans avoir été invité, toutes les prochaines tentatives de join sur ce channel, même en ayant été invité entre temps, ne seront pas transmises au serveur, pour éviter un spam. Donc il faut /quote JOIN #chan une fois invité pour pouvoir join le serveur correctement. Ou juste fermer l'onglet ouvert lors de la tentative infructueuse, avec /window close, et retenter /join #chan.
- Si on un Client change de NICK dans nc, tout les autres clients du serveur en sont notifiés. Dans irssi, la notification n'est reçu que si le Client concerné partage un channel avec lui.
