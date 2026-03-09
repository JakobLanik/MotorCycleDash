import pandas as pd
import matplotlib.pyplot as plt


# Dateien / Ordner
DATA_FILE = "DA_Alvin.csv"
OUT_DIR = "graphs"

# Plot-Einstellungen
DPI = 300
START_ANGLE = 90


def save_pie_with_legend(
    series: pd.Series,
    title: str,
    legend_title: str,
    out_path: str,
    order=None,
):
    """Pie Chart mit externer Legende (%, Anzahl)."""

    # Werte zählen
    counts = series.value_counts(dropna=False)

    # Feste Reihenfolge (optional)
    if order is not None:
        counts = counts.reindex(order).dropna()

    total = counts.sum()
    percentages = 100 * counts / total

    # Neue Figure
    fig, ax = plt.subplots()

    # Pie Chart
    wedges, _ = ax.pie(
        counts,
        startangle=START_ANGLE,
    )

    # Legenden-Texte
    legend_labels = [
        f"{label}: {pct:.1f}% ({count})"
        for label, pct, count in zip(counts.index.astype(str), percentages, counts)
    ]

    # Legende
    ax.legend(
        wedges,
        legend_labels,
        title=legend_title,
        loc="center left",
        bbox_to_anchor=(1, 0.5),
        frameon=False,
    )

    ax.set_title(title)
    ax.axis("equal")

    # Speichern
    fig.savefig(out_path, dpi=DPI, bbox_inches="tight")
    plt.close(fig)

def save_bar_1_to_5(
    series: pd.Series,
    title: str,
    xlabel: str,
    ylabel: str,
    out_path: str,
):
    """Balkendiagramm für Bewertungen von 1 bis 5."""

    # Werte bereinigen
    data = pd.to_numeric(series, errors="coerce").dropna().astype(int)

    # Zählen (1–5 erzwingen)
    counts = data.value_counts().reindex([1, 2, 3, 4, 5], fill_value=0)

    fig, ax = plt.subplots()

    bars = ax.bar(
        counts.index,
        counts.values,
    )

    # Beschriftungen
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    ax.set_xticks([1, 2, 3, 4, 5])

    # Werte über Balken schreiben
    for bar in bars:
        height = bar.get_height()
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            height,
            f"{int(height)}",
            ha="center",
            va="bottom",
        )

    fig.savefig(out_path, dpi=DPI, bbox_inches="tight")
    plt.close(fig)


def main():

    # CSV laden
    df = pd.read_csv(DATA_FILE)

    # 1) Motorradfahrer
    save_pie_with_legend(
        df["motorradfahrer"],
        "Fährst du aktuell Motorrad?",
        "Motorradfahrer",
        f"{OUT_DIR}/anteil_fahrer.png",
        order=["Ja", "Nein"],
    )

    # 2) Alter
    save_pie_with_legend(
        df["alter"],
        "Altersverteilung",
        "Alter",
        f"{OUT_DIR}/alter.png",
        order=["unter 18", "18-25", "25-40", "40-60", "60+"],
    )

    # 3) Fahrttyp
    save_pie_with_legend(
        df["fahrttyp"],
        "Fahrttypen",
        "Fahrttyp",
        f"{OUT_DIR}/fahrttyp.png",
    )

    print("Alle Grafiken wurden gespeichert ✅")

def main2():
    df = pd.read_csv(DATA_FILE)

    # 4) Zufriedenheit (Skala 1–5)
    save_bar_1_to_5(
        df["relevanz_geschwindigkeitsanzeige"],
        "Relevanz der Funktion: Geschwindigkeitsanzeige",
        "Bewertung (1 = schlecht, 5 = sehr gut)",
        "Anzahl Personen",
        f"{OUT_DIR}/r_geschw.png",
    )
    save_bar_1_to_5(
        df["Rrelevanz_motor_reifentemperatur"],
        "Relevanz der Funktion: Motor- und Reifentemperatur",
        "Bewertung (1 = schlecht, 5 = sehr gut)",
        "Anzahl Personen",
        f"{OUT_DIR}/r_motor.png",
    )
    save_bar_1_to_5(
        df["relevanz_neigungswinkel_fahrdynamik"],
        "Relevanz der Funktion: Neigungswinkel/Fahrdynamik",
        "Bewertung (1 = schlecht, 5 = sehr gut)",
        "Anzahl Personen",
        f"{OUT_DIR}/r_lean.png",
    )
    save_bar_1_to_5(
        df["relevanz_datenspeicherung"],
        "Relevanz der Funktion: Datenspeicherung (SD-Karte)",
        "Bewertung (1 = schlecht, 5 = sehr gut)",
        "Anzahl Personen",
        f"{OUT_DIR}/r_daten.png",
    )
    save_bar_1_to_5(
        df["relevanz_display"],
        "Relevanz der Funktion: Übersichtliches Display",
        "Bewertung (1 = schlecht, 5 = sehr gut)",
        "Anzahl Personen",
        f"{OUT_DIR}/r_display.png",
    )

def main3():

    # CSV laden
    df = pd.read_csv(DATA_FILE)

    # 1) Motorradfahrer
    save_pie_with_legend(
        df["zahlungsbereitschaft"],
        "Zahlungsbereitschaft: Würdest du für ein erweitertes Motorrad-Tacho-System Geld ausgeben?",
        "Zahlungsbereitschaft",
        f"{OUT_DIR}/zahlungsbereitschaft.png",
        order=["Ja", "Vielleicht", "Nein"],
    )

    # 2) Alter
    save_pie_with_legend(
        df["real_preis"],
        "Zahlungsbereitschaft: Welcher Preis wäre für dich realistisch?",
        "Preis",
        f"{OUT_DIR}/real_preis.png",
        order=["Unter 100€", "100-150€", "150-200€", "200-250€", "Über 250€"],
    )



# Script starten
if __name__ == "__main__":
    main()
    main2()
    main3()
