'''
In beiden Scripts benötigte Methode
'''
import pandas as pd
import os

def create_df():
    n = input("Nummer des Logs eingeben: ")
    data = f"data_{n}.csv"
    df = None
    if os.path.isfile(data):
        print(f"Df für Log {n} wird erstellt")
        df = pd.read_csv(data)
    else:
        print("Ungültiger Wert")
    return df, n
