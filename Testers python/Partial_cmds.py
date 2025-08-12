#!/usr/bin/env python3
import socket
import time

HOST = "127.0.0.1"   # Ton serveur IRC
PORT = 12345          # Port IRC de ton serveur
PASSWORD = "OK"      # Mot de passe attendu par ton serveur

def send_fragments(sock, fragments, delay=0.5):
    """Envoie une commande en plusieurs morceaux avec un délai entre chaque."""
    for frag in fragments:
        sock.sendall(frag.encode())
        time.sleep(delay)

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    print(f"[+] Connecté à {HOST}:{PORT}")

    # Test 1 : PASS fragmenté mais correct
    print("[*] Envoi PASS correct en fragments...")
    send_fragments(s, ["PASS ", PASSWORD, "\r\n"], delay=0.3)

    # Test 2 : NICK en fragments
    print("[*] Envoi NICK en fragments...")
    send_fragments(s, ["N", "I", "CK mynick", "\r\n"], delay=0.3)

    # Test 3 : USER en fragments
    print("[*] Envoi USER en fragments...")
    send_fragments(s, ["USER ", "myuser 0 * :", "Real Name", "\r\n"], delay=0.3)

    # Test 4 : JOIN en fragments
    print("[*] Envoi JOIN en fragments...")
    send_fragments(s, ["JOIN ", "#42", "\r\n"], delay=0.3)

    # Test 5 : Commande incomplète (jamais terminée)
    print("[*] Envoi commande incomplète (ne sera jamais terminée)...")
    send_fragments(s, ["PRIVMSG ", "#42 :", "coucou incomplet"], delay=0.3)
    time.sleep(2)

    print("[*] Fermeture connexion")
    s.close()

if __name__ == "__main__":
    main()
