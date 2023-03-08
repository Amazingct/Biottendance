import socket 
import threading
import json
import pandas as pd
import os
import Database as db

HEADER = 64
PORT = 5059
SERVER = socket.gethostbyname(socket.gethostname())
ADDR = (SERVER, PORT)
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADDR)
AllStudents = db.Students("Data")
sample = {}



def process_request(msg):
    global sample
    msg = json.loads(msg)
    print(msg["mode"], "request made")
    # add attendance request
    if msg["mode"] == "add_atd":
        if msg["card_id"] != None:
            student = AllStudents.fetch_data(msg["card_id"], "card_id")
            if type(student) is int:
                 return json.dumps({"mode":"response", "message": "unregistred"})
            r = db.addAttendance(student["name"])     
        else:
            student = AllStudents.fetch_data(msg["print_id"], "print_id")
            if type(student) is int:
                 return json.dumps({"mode":"response", "message": "unregistered"})
            r = db.addAttendance(student["name"])   

        return json.dumps({"mode":"response", "message": r[1]})

    # add student request
    elif msg["mode"] == "add_std":
        #save id details and request student name
        new_id = AllStudents.get_lenght()
        student_name = input("Enter Student name: ")
        sample = {"student_id":new_id, "name":student_name, "card_id":msg["card_id"],"print_id":msg["print_id"]}
        r = AllStudents.update_data(sample)
        return json.dumps({"mode":"response", "message": r[1]})
    
    elif msg["mode"]== "test":
        return json.dumps({"mode":"response", "message": "Test Succesful"})



    else:
        return json.dumps({"mode":"response", "message": "Wrong Message"})

        

def handle_client(conn, addr):
    print(f"[NEW CONNECTION] {addr} connected.")

    connected = True
    while connected:
        # msg_length = conn.recv(HEADER).decode(FORMAT)
        # if msg_length:
        #     msg_length = int(msg_length)
        #     msg = conn.recv(msg_length).decode(FORMAT)
        #     if msg == DISCONNECT_MESSAGE:
        #         connected = False

        msg = conn.recv(HEADER)
        msg = str(msg)[2:-1]
        print(f"[{addr}] {str(msg)}")
        response = process_request(msg)


        conn.send(response.encode("utf-8"))

        #conn.send(response.encode(FORMAT))
        
        

    conn.close()
        

def start():
    server.listen()
    print(f"[LISTENING] Server is listening on {SERVER}")
    while True:
        conn, addr = server.accept()
        thread = threading.Thread(target=handle_client, args=(conn, addr))
        thread.start()
        print(f"[ACTIVE CONNECTIONS] {threading.activeCount() - 1}")


print("[STARTING] server is starting...")
start()