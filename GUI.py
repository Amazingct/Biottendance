import tkinter as tk
window = tk.Tk()
# -------------


window.title('IOT BIOMETRICS AND RFID ATTENDANCE SYSTEM')
#FRAMES
table_frame = tk.Frame(window)
input_frame = tk.Frame(window, width=500,height=3)
terminal_frame = tk.Frame(window, width=500, height=350,bg="black")


#INPUT FRAME
def send_command():
    pass

text_box_label = tk.Label(input_frame, text='INPUT', width=50)
text_box = tk.Entry(input_frame, width=50)
send_button = tk.Button(input_frame, text='SEND', width=50, command=send_command)

text_box_label.grid(column=0, row=0)
text_box.grid(column=1, row=0)
send_button.grid(column=2, row=0)

#TERMINAL FRAME
# canva = tk.Canvas(terminal_frame, width=40, height=60)
# canvas_height=100
# canvas_width=40
# y = int(canvas_height / 2)
# canva.create_line(0, y, canvas_width, y )
# canva.pack()


#TABLE FRAME



#FRAME GRID
terminal_frame.grid(column=0,row=0)
input_frame.grid(column=0,row=100)



# -------------
window.mainloop()