import pandas as pd 
import os
import time
from datetime import date
from datetime import datetime
def dict_arrange(data):
    for key, value in data.items():
        data.update({key:[value]})
    return data

def register_student(name, card_id, print_id):
    pass

def student_sign_in(id_type, id, time_in):
    pass

def get_date():
    today = date.today()
    return str(today)

def get_time_in():
    now = datetime.now()
    dt_string = now.strftime("%H:%M:%S")
    return dt_string

class Students:
    def __init__(self, location, data_to_store = ["student_id", "name", "card_id","print_id"]) -> None:
        self.record_path = os.path.join(location,"students.xlsx")
        self.coulums = dict(zip(data_to_store, [[]]* len(data_to_store)))

        if os.path.exists(self.record_path):
            pass
        else:
            df = pd.DataFrame.from_dict(self.coulums)
            df.to_excel(self.record_path, index=False)
        

    def save_data(self,data):
        writer = pd.ExcelWriter(self.record_path)
        data.to_excel(writer, index=False)
        writer.save()
    
    def load_data(self):
        data = pd.read_excel(self.record_path)
        return data

    def fetch_data(self, known, col="student_id"):
        try:
            df = self.load_data()
            # get row
            data_in_col = list(df[col])
            index = data_in_col.index(int(known))
            return df.iloc[index]
        except:
            return 0
    
    def get_lenght(self):
        data=self.load_data()
        return len(data.index)

    def update_data(self,data_dict):
        print("Data to Update:", data_dict)
        new_row = data_dict.copy()
        previous_data = self.load_data()
        id = data_dict["student_id"]
        data_dict.pop("student_id")
        info = data_dict

        #if id is there, jst update values
        if int(id) in list(previous_data["student_id"]):
            for coulum, value in info.items():
                #find id row and replace values
                print(coulum, value)
                previous_data.at[id,coulum]=value
                self.save_data(previous_data)
            return 4,"Data exits-updated instead"
        
        if new_row["card_id"] != "":
            if int(new_row["card_id"]) in list(previous_data["card_id"]):
                return 3, "card already added"
            
        if new_row["print_id"] != "":
            if int(new_row["print_id"]) in list(previous_data["print_id"]):
                return 2, "print already added"

        if new_row["name"] in list(previous_data["name"]):
            return 0, "name already added"

        else:
            #create new row
            to_be_added = list(new_row.values())
            previous_data.loc[len(previous_data.index)] = to_be_added
            print(previous_data)
            self.save_data(previous_data)
            return 1, "sucess"
        
    # def delete_student(self, id):
    #     data=self.load_data()
    #     row_to_be_removed = data.iloc[id]

def addAttendance(name, location="Data", data_to_store= ["sn", "name", "time-in"]):
    record_path = os.path.join(location,get_date()+".xlsx")
    coulums = dict(zip(data_to_store, [[]]* len(data_to_store)))
    if os.path.exists(record_path):
        pass
    else:
        df = pd.DataFrame.from_dict(coulums)
        df.to_excel(record_path, index=False)

    # add to table
    previous_data = pd.read_excel(record_path)

    if name in list(previous_data["name"]):
        return 0,  "student alredy signed-in"
        
    else:
        #create new row
        to_be_added = [len(previous_data.index), name, get_time_in()]
        previous_data.loc[len(previous_data.index)] = to_be_added
        print(previous_data)

        #save
        writer = pd.ExcelWriter(record_path)
        previous_data.to_excel(writer, index=False)
        writer.save()
        return 1, "sucess"
        



# data = Students("Data")
# new_id = data.get_lenght()

# sample = {"student_id":new_id, "name":"daniel casg", "card_id":"578","print_id":"68"}
# data.update_data(sample)
# print(data.fetch_data(600, "print_id")) 

# student = data.fetch_data(23, "card_id")
# addAttendance(student["name"])




