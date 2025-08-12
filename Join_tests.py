import socket
import time

SERVER = '127.0.0.1'
PORT = 12345
ENCODING = 'utf-8'
PASSWORD = 'OK'  # Mets ici ton mot de passe si ton serveur en demande

def connect_and_register(nick, user='user', realname='Real Name', password=None):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
    s.connect((SERVER, PORT))
    resp = ''
    if password:
        s.sendall(f"PASS {password}\r\n".encode(ENCODING))
        resp += recv_all(s)
    s.sendall(f"NICK {nick}\r\n".encode(ENCODING))
    resp += recv_all(s)
    s.sendall(f"USER {user} 0 * :{realname}\r\n".encode(ENCODING))
    resp += recv_all(s)
    return s, resp

def send_and_recv(sock, msg):
    sock.sendall(f"{msg}\r\n".encode(ENCODING))
    return recv_all(sock)

def recv_all(sock):
    buff = ''
    try:
        while True:
            part = sock.recv(4096).decode(ENCODING)
            if not part:
                break
            buff += part
            if part.endswith('\r\n'):
                break
    except socket.timeout:
        pass
    return buff

def test_join_normal():
    print("========== Test JOIN normal channel ==========")
    s, resp = connect_and_register('tester1', password=PASSWORD)
    print("Registration response:\n", resp.strip())
    resp = send_and_recv(s, 'JOIN #testchannel')
    print("JOIN #testchannel response:\n", resp.strip())
    s.close()

def test_join_multiple_channels():
    print("\n========== Test JOIN multiple channels (should fail) ==========")
    s, resp = connect_and_register('tester2', password=PASSWORD)
    print("Registration response:\n", resp.strip())
    resp = send_and_recv(s, 'JOIN #chan1,#chan2')
    print("JOIN #chan1,#chan2 response:\n", resp.strip())
    s.close()

def test_join_invalid_channel():
    print("\n========== Test JOIN invalid channel name ==========")
    s, resp = connect_and_register('tester3', password=PASSWORD)
    print("Registration response:\n", resp.strip())
    resp = send_and_recv(s, 'JOIN invalidchannel')
    print("JOIN invalidchannel response:\n", resp.strip())
    s.close()

def test_join_already_joined():
    print("\n========== Test JOIN already joined channel ==========")
    s, resp = connect_and_register('tester4', password=PASSWORD)
    print("Registration response:\n", resp.strip())
    resp = send_and_recv(s, 'JOIN #repeat')
    print("First JOIN #repeat response:\n", resp.strip())
    resp = send_and_recv(s, 'JOIN #repeat')
    print("Second JOIN #repeat response (should be empty or no error):\n", resp.strip())
    s.close()

def test_join_without_registration():
    print("\n========== Test JOIN without registration ==========")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
    s.connect((SERVER, PORT))
    resp = send_and_recv(s, 'JOIN #testchannel')
    print("JOIN without registration response:\n", resp.strip())
    s.close()

def test_join_wrong_password():
    print("\n========== Test JOIN with wrong password (if server requires PASS) ==========")
    try:
        s, resp = connect_and_register('tester5', password="wrongpassword")
        print("Registration response:\n", resp.strip())
        resp = send_and_recv(s, 'JOIN #testchannel')
        print("JOIN #testchannel response with wrong password:\n", resp.strip())
        s.close()
    except Exception as e:
        print("Error (likely connection closed due to wrong password):", e)

def test_join_with_key():
    print("\n========== Test JOIN with modes (key) ==========")
    s, resp = connect_and_register('tester6', password=PASSWORD)
    print("Registration response:\n", resp.strip())
    resp = send_and_recv(s, 'JOIN #secretchan key123')
    print("JOIN #secretchan with key response:\n", resp.strip())
    s.close()

if __name__ == '__main__':
    test_join_normal()
    test_join_multiple_channels()
    test_join_invalid_channel()
    test_join_already_joined()
    test_join_without_registration()
    test_join_wrong_password()
    test_join_with_key()
