import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from openpyxl import load_workbook
import time
import socket 
import threading
import json
import pandas as pd
import os
import Database as db

HEADER = 200
PORT = 5059
SERVER = socket.gethostbyname(socket.gethostname())
ADDR = ('', PORT)
FORMAT = 'utf-8'
DISCONNECT_MESSAGE = "!DISCONNECT"

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADDR)
AllStudents = db.Students("Data")
sample = {}
input_text = None



def process_request(msg):
    global sample
    msg = json.loads(msg)
    print(msg["mode"], "request made")
    # add attendance request
    if msg["mode"] == "add_atd":
        if msg["card_id"] != "":
            student = AllStudents.fetch_data(msg["card_id"], "card_id")
            if type(student) is int:
                 return json.dumps({"mode":"response", "message": "unregistred"}),{"mode":"response", "message": "unregistred"}
            r = db.addAttendance(student["name"])     
        else:
            student = AllStudents.fetch_data(msg["print_id"], "print_id")
            if type(student) is int:
                 return json.dumps({"mode":"response", "message": "unregistered"}), {"mode":"response", "message": "unregistered"}
            r = db.addAttendance(student["name"])   

        return json.dumps({"mode":"response", "message": r[1]}), {"mode":"response", "message": r[1]}

    # add student request
    elif msg["mode"] == "add_std":
        #save id details and request student name
        new_id = AllStudents.get_lenght()
        student_name = input("Enter Student name: ")
        sample = {"student_id":new_id, "name":student_name, "card_id":msg["card_id"],"print_id":msg["print_id"]}
        r = AllStudents.update_data(sample)
        return json.dumps({"mode":"response", "message": r[1]}), {"mode":"response", "message": r[1]}
    
    # get next availableprint id
    elif msg["mode"]== "get_id":
        next_id = 0
        try:
            next_id = new_id = AllStudents.get_lenght()
        except:
            pass
        return json.dumps({"mode":"response", "message": next_id}), {"mode":"response", "message": next_id}
    
    elif msg["mode"]== "test":
        return json.dumps({"mode":"response", "message": "Test Succesful"}), {"mode":"response", "message": "Test Succesful"}

    elif msg["mode"]== "clear_db":
        df = pd.DataFrame.from_dict(AllStudents.coulums)
        df.to_excel(AllStudents.record_path, index=False)
        return json.dumps({"mode":"response", "message": "Clear Successful"}), {"mode":"response", "message": "Clear Successful"}


    else:
        j = {"mode":"response", "message": "Wrong Message"}
        return json.dumps(j), j

def handle_client(conn, addr):
    print(f"[NEW CONNECTION] {addr} connected.")

    connected = True
   
    msg = conn.recv(HEADER)
    msg = str(msg)[2:-1]
    #print(f"[{addr}] {str(msg)}")
    response, actual = process_request(msg)

    print(actual["message"])
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
        #print(f"[ACTIVE CONNECTIONS] {threading.activeCount() - 1}")


class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.master.geometry("1800x500")  # Set window size
        self.master.configure(bg="#1c2d4f")  # Set window background color
        self.pack(fill="both", expand=True)

        # Left side: console and text box
        left_frame = tk.Frame(self, width=700, height=500, bg="#1c2d4f")
        left_frame.pack(side="left", fill="both", expand=True)

        # Console widget
        console_label = tk.Label(left_frame, text="Console", bg="#1c2d4f", fg="#FFFFFF", font=("Helvetica", 14, "bold"))
        console_label.pack(side="top", pady=10)

        # Scrollable console widget
        console_frame = tk.Frame(left_frame)
        console_frame.pack(side="top", padx=10, pady=10, fill="both", expand=True)

        self.console = tk.Text(console_frame, height=20, width=150, wrap="none", bg="#242a3f", fg="#FFFFFF")
        self.console.pack(side="left", fill="both", expand=True)

        scrollbar = tk.Scrollbar(console_frame, orient="vertical", command=self.console.yview)
        scrollbar.pack(side="right", fill="y")

        self.console.config(yscrollcommand=scrollbar.set)

        # Input widget
        input_frame = tk.Frame(left_frame, bg="#1c2d4f")
        input_frame.pack(side="bottom", pady=10)
        input_label = tk.Label(input_frame, text="Input:", bg="#1c2d4f", fg="#FFFFFF")
        input_label.pack(side="left", padx=10)
        self.input_entry = tk.Entry(input_frame, width=40)
        self.input_entry.pack(side="left")
        send_button = tk.Button(input_frame, text="Send", command=self.send, bg="#3a4d7c", fg="#FFFFFF")
        send_button.pack(side="left", padx=10)

        # Right side: table
        right_frame = tk.Frame(self, width=400, height=500, bg="#1c2d4f")
        right_frame.pack(side="right", fill="both", expand=True)

        # Sheet selection widget
        sheet_label = tk.Label(right_frame, text="Select File:", bg="#1c2d4f", fg="#FFFFFF", font=("Helvetica", 14, "bold"))
        sheet_label.pack(side="top", pady=10)
        sheet_button = tk.Button(right_frame, text="Browse...", command=self.browse_file, bg="#3a4d7c", fg="#FFFFFF")
        sheet_button.pack(side="top", padx=10, pady=10)



        # Table name label
        self.table_name_label = tk.Label(right_frame, text="", bg="#1c2d4f", fg="#FFFFFF", font=("Helvetica", 14, "bold"))
        self.table_name_label.pack(side="top", pady=10)

        # Table widget
        self.sheet_canvas = tk.Canvas(right_frame, width=380, height=460, bg="#1c2d4f", highlightthickness=0)
        self.sheet_canvas.pack(side="left", fill="both", expand=True)

        # # Graphic
        # graphic_frame = tk.Frame(left_frame, bg="#1c2d4f")
        # graphic_frame.pack(side="bottom", fill="both", expand=True)
        # graphic = tk.PhotoImage(file="images/ace.PNG")
        # graphic_label = tk.Label(graphic_frame, image=graphic)
        # graphic_label.image = graphic
        # graphic_label.pack()

    def send(self):
        # Get input from user and clear input entry
        global input_text
        input_text = self.input_entry.get()
        self.console.insert(tk.END, f"> {input_text}\n")
        self.console.see(tk.END)

    def browse_file(self):
        # Open file dialog to select file
        file_path = filedialog.askopenfilename()

        # Load selected sheet from file
        workbook = load_workbook(filename=file_path)
        sheet = workbook.active

        # Create table to display sheet data
        rows = sheet.rows
        columns = sheet.columns
        table = []
        for row in rows:
            row_data = []
            for cell in row:
                row_data.append(cell.value)
            table.append(row_data)

        # Set table name label
        file_name = file_path.split("/")[-1]
        self.table_name_label.config(text=file_name)

        # Clear previous widgets in sheet canvas
        self.sheet_canvas.delete("all")

        # Display table in sheet canvas using grid()
        for i, row in enumerate(table):
            for j, value in enumerate(row):
                cell = tk.Label(self.sheet_canvas, text=value, borderwidth=1, relief="solid", bg="#3a4d7c", fg="#FFFFFF")
                cell.grid(row=i, column=j, sticky="nsew")

        # Update sheet canvas
        self.sheet_canvas.update_idletasks()
        self.sheet_canvas.config(scrollregion=self.sheet_canvas.bbox("all"))

root = tk.Tk()

root.configure(bg="#1c2d4f")  # Set window background color
app = Application(master=root)

# Add the rest of your code here
# ...


    





def print(*texts, sep=" ", end="\n"):
    text = sep.join([str(text) for text in texts]) + end
    text = text.upper()
    app.console.insert(tk.END, f"{text}\n")

def input(prompt=""):
    print(prompt, end="")
    app.console.see(tk.END)
    global input_text
    while input_text is None:
        time.sleep(0.1)
    text = input_text
    input_text = None
    #clear textbox
    app.input_entry.delete(0, tk.END)
    return text



    


def update_console(self):
    start()


# Start thread to update console text
thread = threading.Thread(target=update_console, args=(app,))
thread.daemon = True
thread.start()
app.mainloop()



