import sqlite3
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os

DB_FILE = "Logging_data.db"


def menu():
    """Main menu for General graphs
    """

    while True:
        print(
            f"""\033[1;34m
  ___                             _    ___                   _        
 / __| ___  _ _   ___  _ _  __ _ | |  / __| _ _  __ _  _ __ | |_   ___
| (_ |/ -_)| ' \ / -_)| '_|/ _` || | | (_ || '_|/ _` || '_ \| ' \ (_-<
 \___|\___||_||_|\___||_|  \__,_||_|  \___||_|  \__,_|| .__/|_||_|/__/
                                                      |_|             
        \033[0m"""
        )
        print("Please select the type of general graph you would like:\n")
        print(f"\033[1;34m[1]\033[0m Normalized Boss Fight Attempts per Player")
        print(f"\033[1;34m[2]\033[0m Average bossfight length")
        print(f"\033[1;34m[Anything]\033[0m Exit")
        print()

        choice = input("Enter your choice:\033[1;34m ")

        if choice == "1":
            normalized_boss_attempts_chart()
            print(f"\033[0mGenerated NormalizedBossAttempts.png!")
        elif choice == "2":
            bossfight_time_chart()
            print(f"\033[0mGenerated AverageBossFightTime.png!")
        else:
            print(f"\033[0mReturning to main menu...")
            break


def time_to_hours(time_str):
    """Convert HH:MM:SS format to total playtime in hours
    Args:
        time_str (string): time string you want to change
    Returns:
        int: The total playtime in hours
    """
    
    try:
        h, m, s = map(int, time_str.split(":"))
        return h + m / 60 + s / 3600  # Convert to total hours
    except ValueError:
        return None  # Handle incorrect formats gracefully


def time_to_minutes(time_str):
    """Convert HH:MM:SS format to total playtime in minutes
    Args:
        time_str (string): time string you want to change
    Returns:
        int: The total playtime in minutes
    """
    
    try:
        h, m, s = map(int, time_str.split(":"))
        return m + h * 60 + s / 60  # Convert to total minutes
    except ValueError:
        return None  # Handle incorrect formats gracefully

##############
#   CHARTS   #
##############

def normalized_boss_attempts_chart():
    """Generates a chart about how many attempts players needed to defeat a boss
    """

    conn = sqlite3.connect(DB_FILE)
    query = """
        SELECT boss_name, 
		COUNT(*) AS total_attempts, 
        COUNT(DISTINCT player_id) AS num_players
        FROM bossfights
		WHERE status = "ENTER"
        GROUP BY boss_name;
    """
    df = pd.read_sql_query(query, conn)
    conn.close()

    # Handle cases where there might be no data
    if df.empty:
        print("No boss fights recorded in the database.")
        return

    # Normalize attempts per player
    df["attempts_per_player"] = df["total_attempts"] / df["num_players"]

    # Adjust figure size dynamically based on number of bosses
    fig_height = max(8, len(df) * 0.5)
    fig, ax = plt.subplots(figsize=(14, fig_height))

    # Create bar chart
    bars = ax.barh(y=df["boss_name"], width=df["attempts_per_player"], color="Maroon")

    # Add labels at the end of the bars
    for bar, value in zip(bars, df["attempts_per_player"]):
        ax.text(
            bar.get_width() + 0.5,
            bar.get_y() + bar.get_height() / 2,
            f"{value:.1f}",
            va="center",
            ha="left",
            fontsize=12,
            fontweight="bold",
            color="black",
        )

    # Formatting
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.spines["left"].set_visible(False)
    ax.spines["bottom"].set_color("#DDDDDD")

    ax.set_xlabel("Average Attempts per Player", fontsize=14, fontweight="bold")
    ax.set_ylabel("Boss Name", fontsize=14, fontweight="bold")
    ax.set_title(
        "Normalized Boss Fight Attempts per Player", fontsize=16, fontweight="bold"
    )
    ax.tick_params(axis="both", labelsize=12)
    ax.grid(axis="x", linestyle="--", alpha=0.7)

    fig.tight_layout()
    if not os.path.exists("General_graphs"):
        os.makedirs("General_graphs")
    fig.savefig("General_graphs/NormalizedBossAttempts.png")
    plt.show()


def bossfight_time_chart():
    """Generates a chart about how much time players needed to deafeat a boss on average
    """

    conn = sqlite3.connect(DB_FILE)
    query = """
WITH raw_data AS (
    SELECT
        player_id,
        boss_name,
        status,
        timestamp,
        ROW_NUMBER() OVER (PARTITION BY player_id, boss_name ORDER BY timestamp) AS rn,
        LEAD(status) OVER (PARTITION BY player_id, boss_name ORDER BY timestamp) AS next_status,
        LAG(status) OVER (PARTITION BY player_id, boss_name ORDER BY timestamp) AS prev_status
    FROM bossfights
    WHERE status IN ('ENTER', 'DEFEATED')
),
validated_data AS (
    SELECT
        player_id,
        boss_name,
        status,
        timestamp,
        CASE 
            -- Flag ENTER only if followed by DEFEATED
            WHEN status = 'ENTER' AND next_status = 'DEFEATED' THEN 'Valid'
            -- Flag DEFEATED if preceded by ENTER
            WHEN status = 'DEFEATED' AND prev_status = 'ENTER' THEN 'Valid'
            ELSE 'Invalid'
        END AS data_quality_flag
    FROM raw_data
)
SELECT
    boss_name,
    
    -- Count the number of valid ENTER and DEFEATED pairs
    COUNT(*) AS valid_pairs,
    
    -- Sum of valid ENTER timestamps in seconds
    SUM(CASE WHEN status = 'ENTER' AND data_quality_flag = 'Valid' THEN 
        (CAST(SUBSTR(timestamp, 1, 2) AS INTEGER) * 3600) +  -- Hours to seconds
        (CAST(SUBSTR(timestamp, 4, 2) AS INTEGER) * 60) +  -- Minutes to seconds
        (CAST(SUBSTR(timestamp, 7, 2) AS INTEGER))  -- Seconds
    ELSE 0 END) AS total_seconds_entered,
    
    -- Sum of valid DEFEATED timestamps in seconds
    SUM(CASE WHEN status = 'DEFEATED' AND data_quality_flag = 'Valid' THEN 
        (CAST(SUBSTR(timestamp, 1, 2) AS INTEGER) * 3600) +  -- Hours to seconds
        (CAST(SUBSTR(timestamp, 4, 2) AS INTEGER) * 60) +  -- Minutes to seconds
        (CAST(SUBSTR(timestamp, 7, 2) AS INTEGER))  -- Seconds
    ELSE 0 END) AS total_seconds_defeated,
    
    -- Count the number of invalid entries
    SUM(CASE WHEN data_quality_flag = 'Invalid' THEN 1 ELSE 0 END) AS invalid_entries
FROM validated_data
WHERE data_quality_flag = 'Valid'
GROUP BY boss_name
ORDER BY valid_pairs DESC;

    """
    df = pd.read_sql_query(query, conn)
    conn.close()

    # Handle cases where there might be no data
    if df.empty:
        print("No boss fights recorded in the database.")
        return

    df["seconds_difference"] = (
        df["total_seconds_defeated"] - df["total_seconds_entered"]
    ) / ((df["valid_pairs"] - df["invalid_entries"]) / 2)

    # Adjust figure size dynamically based on number of bosses
    fig_height = max(8, len(df) * 0.5)
    fig, ax = plt.subplots(figsize=(14, fig_height))

    # Create bar chart
    bars = ax.barh(y=df["boss_name"], width=df["seconds_difference"], color="FireBrick")

    # Add labels at the end of the bars
    for bar, value in zip(bars, df["seconds_difference"]):
        ax.text(
            bar.get_width() + 0.5,
            bar.get_y() + bar.get_height() / 2,
            f"{value:.1f}",
            va="center",
            ha="left",
            fontsize=12,
            fontweight="bold",
            color="black",
        )

    # Formatting
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.spines["left"].set_visible(False)
    ax.spines["bottom"].set_color("#DDDDDD")

    ax.set_xlabel("Seconds", fontsize=14, fontweight="bold")
    ax.set_ylabel("Boss Name", fontsize=14, fontweight="bold")
    ax.set_title("Average boss fight length in seconds", fontsize=16, fontweight="bold")
    ax.tick_params(axis="both", labelsize=12)
    ax.grid(axis="x", linestyle="--", alpha=0.7)

    fig.tight_layout()
    if not os.path.exists("General_graphs"):
        os.makedirs("General_graphs")
    fig.savefig("General_graphs/AverageBossFightTime.png")
    plt.show()
