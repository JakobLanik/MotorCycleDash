'''
Script zum graphischen Ausgeben der Strecke
'''

import folium
import methods as md

def create_route(df, n):
    if df is not None:

        df = df[(df["lat"] != 0) & (df["lon"] != 0)]

        route = df[["lat", "lon"]].values.tolist()
        r = folium.Map(location=route[0], zoom_start=15)

        folium.PolyLine(
                route,
                color="blue",
                weight=5,
                opacity=0.8
        ).add_to(r)

        folium.Marker(route[0], popup="Start", icon=folium.Icon(color="green")).add_to(r)
        folium.Marker(route[-1], popup="Ziel", icon=folium.Icon(color="red")).add_to(r)
        r.save(f"route_{n}.html")
    else:
        print("Route wurde nicht erstellt. Keine gültigen Daten erhalten.")


if __name__ == "__main__":
    try:
        df, n = md.create_df()
	create_route(df, n)
    except Exception as e:
        print("Route konnte nicht erstellt Werden")
