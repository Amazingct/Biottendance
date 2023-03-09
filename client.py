import socket, json
sample_msg2 = {"mode":"add_atd", "card_id":None, "print_id":7}
sample_msg = {"mode":"add_std", "card_id":55, "print_id":40}


HEADER = 64
PORT = 5059
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"
SERVER = "192.168.1.145"
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
    request = input("request>> ")
    if request == "end":
        response = send(DISCONNECT_MESSAGE)
    elif request == "sample":
        response = send(json.dumps(sample_msg))
    elif request == "sample2":
        response = send(json.dumps(sample_msg3))
    else:
        response = send(request)
    print(response)



