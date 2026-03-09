'''
KI-generiertes Script, dass die Diagramme und Graphen in der Arbeit eineitlich im Design von matplotlib dargestellt sind
'''

import matplotlib.pyplot as plt

# Plot-Einstellungen (wie in deinem Script)
DPI = 300
START_ANGLE = 90


def save_pie_with_legend_counts(
    counts_dict: dict,
    title: str,
    legend_title: str,
    out_path: str,
    order=None,
):
    """Pie Chart mit externer Legende (%, Anzahl) – gleicher Stil wie main.py"""

    # Reihenfolge festlegen
    if order is not None:
        labels = order
        counts = [counts_dict[label] for label in labels]
    else:
        labels = list(counts_dict.keys())
        counts = list(counts_dict.values())

    total = sum(counts)
    percentages = [100 * c / total for c in counts]

    fig, ax = plt.subplots()

    wedges, _ = ax.pie(
        counts,
        startangle=START_ANGLE,
    )

    legend_labels = [
        f"{label}: {pct:.1f}% ({count})"
        for label, pct, count in zip(labels, percentages, counts)
    ]

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

    fig.savefig(out_path, dpi=DPI, bbox_inches="tight")
    plt.close(fig)


def main():

    OUT_DIR = "graphs"

    fragen = {
        "Motorradfahrer": {"Ja": 21, "Nein": 1},
        "Regelmäßig Touren": {"Ja": 15, "Nein": 7},
        "Motorrad älter als 5 Jahre": {"Ja": 17, "Nein": 5},
        "Erweiterungen o.äh. verbaut": {"Ja": 4, "Nein": 18},
        "Interesse an unserer Erweiterung": {"Ja": 12, "Nein": 10},
    }

    for frage, werte in fragen.items():
        filename = (
            frage.replace(" ", "_")
            .replace("ä", "ae")
            .replace("ö", "oe")
            .replace("ü", "ue")
            .replace(".", "")
        )

        save_pie_with_legend_counts(
            werte,
            frage,
            "Antwort",
            f"{OUT_DIR}/{filename}.png",
            order=["Ja", "Nein"],
        )

    print("Alle Ja/Nein-Piecharts wurden gespeichert ✅")


if __name__ == "__main__":
    main()
