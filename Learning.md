                        ===== Notions mal comprises / mal maitrisée =====

I - Const

CONST ; Ce qui est après ne peut-être modifié.

Le but est de securiser le code, on ne veut pas que nos données soient toutes modifiables. 
Par ex : Dans le cas d'un getter. Je veux que depuis le main, on puisse avoir accès à une donnée sensible, private de de la classe Server, parce que j'en ai besoin. Cependant, je ne veux pas qu'on puisse la modifier. Donc je renvoie une référence constante sur la donnée en question.

II - Using, namespace et typedef/define

Admettons que j'ai une structure MaStruct avec un int a, et une fonction b.
Plutot que d'écrire à chaque fois MaStruct::b, je veux faire en sorte d'écrire juste b.
Donc en utilisant using MaStruct::b, je peux désormais dans mon code juste écrire b().

Ensuite viens le namespace. Un namespace, c'est une boite qu'on crée et dans laquelle on range des choses, exemple :


namespace Outils {
    void afficher() {
        std::cout << "Coucou !\n";
    }
}

int main() {
    // Outils::afficher();     // version longue
    using Outils::afficher;    // je prends juste afficher
    afficher();                // plus besoin de Outils::
}

using namespace quand à lui, c'est pour dire au compilateur "Si jamais tu rencontres b() et/où a, tu peux rajouter MaStruct:: devant. 

#define sert à créer des alias textuels. Exemple : #define ClientMap std::map<int, Client> va dire au compilateur "Dés que tu croises le mot ClientMap, remplace le par std::map<int, Client>", ce que le compilateur fera. Ce n'est pas sécurisé par le compilateur. 

typedef sert également à créer des alias. Cependant, il s'agit ici d'alias de type, et sécurisé. Il permet en quelque sorte de faire des types plus précis. Admettons que je veux un type int pair. Je peux faire typedef int pair, et dans mon code faire pair = 2. Il sera reconnu comme un int. 

III - Argument par défaut 

void broadcast(const std::string& msg, int excepted_fd = -1);

Voici un prototype de fonction avec argument par défaut. Ce qui signifie qu'on peut utiliser cette fonction de deux façons. D'une part en envoyant une valeur en argument pour excepted_fd, ou sans envoyer de valeur en argument, ce qui le cas échéant attribuera la valeur défini par défaut dans le prototype à la variable.

IV - Foncteur (Fonction-Objet)

Dans notre code on a ça :

	struct MatchFd {
		int fd;
		MatchFd(int fd) : fd(fd) {};
		
		//Convention : RHS = Right hand side / LHS = Left hand side
		bool operator()(const pollfd &rhs) { return fd == rhs.fd; }
	};

Il s'agît ici à la fois d'une structure, d'une mini classe (Puisqu'elle a un constructeur), et d'une fonction avec la surcharge d'opérateur (). 
C'est principalement utilisé pour la fonction find_if. 

