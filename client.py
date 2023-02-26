import socket, json
sample_msg = {"type":"single", "mode":"scan", "card_id":"123", "print_id":156}

HEADER = 64
PORT = 5050
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"
SERVER = "192.168.56.1"
ADDR = (SERVER, PORT)

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(ADDR)

def send(msg):
    message = msg.encode(FORMAT)
    msg_length = len(message)
    send_length = str(msg_length).encode(FORMAT)
    send_length += b' ' * (HEADER - len(send_length))
    client.send(send_length)
    client.send(message)
    return(client.recv(2048).decode(FORMAT))

while True:
    request = input("reques>> ")
    if request == "end":
        response = send(DISCONNECT_MESSAGE)
    elif request == "sample":
        response = send(json.dumps(sample_msg))
    else:
        response = send(request)
    print(response)



