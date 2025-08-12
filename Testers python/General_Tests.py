import socket
import time

SERVER_HOST = 'localhost'
SERVER_PORT = 12345
PASS = "OK"
CHANNEL = "#testchannel"

def recv_all(sock, timeout=0.5):
    sock.settimeout(timeout)
    data = b""
    try:
        while True:
            part = sock.recv(4096)
            if not part:
                break
            data += part
    except socket.timeout:
        pass
    return data.decode(errors='ignore')

def send_and_recv(sock, msg, wait=0.2):
    sock.sendall(msg.encode())
    time.sleep(wait)
    return recv_all(sock)

def register_client(sock, nick, user):
    responses = []
    responses.append(send_and_recv(sock, f"PASS {PASS}\r\n"))
    responses.append(send_and_recv(sock, f"NICK {nick}\r\n"))
    responses.append(send_and_recv(sock, f"USER {user} 0 * :Real Name\r\n"))
    return responses

def join_channel(sock, channel):
    return send_and_recv(sock, f"JOIN {channel}\r\n")

def test_basic_commands():
    print("== Test: Auth + NICK + USER + JOIN channel ==")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER_HOST, SERVER_PORT))

    responses = register_client(s, "tester1", "user1")
    print("After registration:", "".join(responses))

    join_resp = join_channel(s, CHANNEL)
    print(f"Join channel response:\n{join_resp}")

    s.close()

def test_privmsg():
    print("== Test: PRIVMSG between clients ==")
    # Client 1
    s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s1.connect((SERVER_HOST, SERVER_PORT))
    register_client(s1, "alice", "userA")
    join_channel(s1, CHANNEL)

    # Client 2
    s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s2.connect((SERVER_HOST, SERVER_PORT))
    register_client(s2, "bob", "userB")
    join_channel(s2, CHANNEL)

    # alice sends PRIVMSG to bob
    msg = "PRIVMSG bob :Hello Bob!\r\n"
    s1.sendall(msg.encode())
    time.sleep(0.5)

    # bob should receive message
    resp_bob = recv_all(s2)
    print("Bob received:\n", resp_bob)

    s1.close()
    s2.close()

def test_channel_operator():
    print("== Test: Channel operator privileges ==")
    # User normal
    user = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    user.connect((SERVER_HOST, SERVER_PORT))
    register_client(user, "regular", "userReg")
    join_channel(user, CHANNEL)

    # Operator (simulate operator by giving op after join)
    op = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    op.connect((SERVER_HOST, SERVER_PORT))
    register_client(op, "opuser", "userOp")
    join_channel(op, CHANNEL)

    time.sleep(0.5)  # wait for joins to process

    # Here we try to make opuser operator - assuming server supports MODE +o <nick>
    # This depends on your server implementation.
    print("Try to give operator mode by opuser")
    op_mode_resp = send_and_recv(op, f"MODE {CHANNEL} +o opuser\r\n")
    print("MODE +o response:\n", op_mode_resp)

    print("Regular user tries to give operator mode (should fail)")
    user_mode_resp = send_and_recv(user, f"MODE {CHANNEL} +o regular\r\n")
    print("Regular user MODE +o response:\n", user_mode_resp)

    # Try channel operator commands as operator and as regular user
    # For example: KICK, TOPIC, INVITE, etc.
    commands = [
        ("KICK", f"{CHANNEL} regular :Kicked by op"),
        ("TOPIC", f"{CHANNEL} :New topic by op"),
        ("INVITE", f"regular {CHANNEL}"),
    ]

    for cmd, params in commands:
        print(f"Operator tries {cmd}")
        resp = send_and_recv(op, f"{cmd} {params}\r\n")
        print(resp)

        print(f"Regular user tries {cmd}")
        resp = send_and_recv(user, f"{cmd} {params}\r\n")
        print(resp)

    user.close()
    op.close()

if __name__ == "__main__":
    test_basic_commands()
    print("\n" + "="*50 + "\n")
    test_privmsg()
    print("\n" + "="*50 + "\n")
    test_channel_operator()
