'''
Script zum plotten der Graphen aus der output file des Tachos
'''

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sb
import methods as md


sb.set_theme(style="darkgrid")


def create_time_plot(df, col):
    df["timestamp"] = pd.to_datetime(df["timestamp"], unit="ms")

    plt.figure(figsize=(10, 5))

    sb.lineplot(
        data=df,
        x="timestamp",
        y=col,
        marker="o",
    )

    plt.xlabel("Zeit")
    plt.ylabel(f"{col}")
    plt.title(f"{col} in Abhängigkeit von t")
    plt.tight_layout()
    plt.show()


def print_time_plot():
    not_in = ["timestamp", "gps_time", "lat", "lon"]
    x = [i for i in df.columns if i not in not_in]

    for i in x:
        create_time_plot(df, i)


def create_tire_temp(df):
    plt.figure(figsize=(10, 5))

    sb.lineplot(
        data=df,
        x="timestamp",
        y="tireR",
        label="R",
    )

    sb.lineplot(
        data=df,
        x="timestamp",
        y="tireL",
        label="L",
    )

    plt.xlabel("Zeit")
    plt.ylabel("tire_temp")
    plt.title("Reifentemperatur")
    plt.tight_layout()
    plt.show()


def create_boxplot(df):
    plt.figure(figsize=(10, 5))
    sb.boxplot(data=df, y="speed")

    plt.title("boxplot der Geschwindigkeit")
    plt.ylabel("Geschwindigkeit")
    plt.show()


if __name__ == "__main__":
    try:
        df, n = md.create_df()

        create_boxplot(df)
        print_time_plot()
        create_tire_temp(df)

    except:
        print("Graphen konnten nicht erstellt werden")
