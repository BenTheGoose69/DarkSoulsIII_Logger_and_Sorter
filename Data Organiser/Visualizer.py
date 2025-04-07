import sqlite3
import pandas as pd
import numpy as np

# import matplotlib
# matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os


# ötletek:
# - player timestamp valamizés, boss effektívség eltöltött időben,
# - boss legyőzési idők rangsorolása
# - játszási idők bar chartokon

# Connect to SQLite database
DB_FILE = "Logging_data.db"


def print_logo():
    os.system("cls" if os.name == "nt" else "clear")
    print(
        f"""\033[1;31m
        
     ______   _______ _________ _______                 _________ _______           _______  _       _________ _______  _______  _______ 
    (  __  \ (  ___  )\__   __/(  ___  )       |\     /|\__   __/(  ____ \|\     /|(  ___  )( \      \__   __// ___   )(  ____ \(  ____ )
    | (  \  )| (   ) |   ) (   | (   ) |       | )   ( |   ) (   | (    \/| )   ( || (   ) || (         ) (   \/   )  || (    \/| (    )|
    | |   ) || (___) |   | |   | (___) |       | |   | |   | |   | (_____ | |   | || (___) || |         | |       /   )| (__    | (____)|
    | |   | ||  ___  |   | |   |  ___  |       ( (   ) )   | |   (_____  )| |   | ||  ___  || |         | |      /   / |  __)   |     __)
    | |   ) || (   ) |   | |   | (   ) |        \ \_/ /    | |         ) || |   | || (   ) || |         | |     /   /  | (      | (\ (   
    | (__/  )| )   ( |   | |   | )   ( |         \   /  ___) (___/\____) || (___) || )   ( || (____/\___) (___ /   (_/\| (____/\| ) \ \__
    (______/ |/     \|   )_(   |/     \|          \_/   \_______/\_______)(_______)|/     \|(_______/\_______/(_______/(_______/|/   \__/
                                                                                                                                
                                                                                                                                
    \033[0m"""
    )


def player_specific_graphs():
    print(
        f"""\033[1;34m
 ___  _                         ___                 _   __  _        ___                   _        
| _ \| | __ _  _  _  ___  _ _  / __| _ __  ___  __ (_) / _|(_) __   / __| _ _  __ _  _ __ | |_   ___
|  _/| |/ _` || || |/ -_)| '_| \__ \| '_ \/ -_)/ _|| ||  _|| |/ _| | (_ || '_|/ _` || '_ \| ' \ (_-<
|_|  |_|\__,_| \_, |\___||_|   |___/| .__/\___|\__||_||_|  |_|\__|  \___||_|  \__,_|| .__/|_||_|/__/
               |__/                 |_|                                             |_|             
        \033[0m"""
    )


def print_players_infos():
    conn = sqlite3.connect("Logging_data.db")
    query = """
        SELECT id, steam_id, character_name FROM player;
    """
    df = pd.read_sql_query(query, conn)
    conn.close()

    # Handle cases where there might be no data
    if df.empty:
        print("No players recorded in the database.")
        return

    for index, row in df.iterrows():
        print(
            f"\033[1;34m[{row["id"]}]\033[0m {row["character_name"]}[{row["steam_id"]}]"
        )
    print("\033[0m")


def validate_player_name(name: str):

    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute("SELECT id FROM player WHERE character_name LIKE ?", (name + "%",))
    result = cursor.fetchone()
    conn.close()

    if result:
        print("Valid player name!")
        return [[result][0][0]]
    else:
        print("No player")
        return [0]


def validate_player_id(id: int):

    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute("SELECT id FROM player WHERE id = ?", (id,))
    result = cursor.fetchone()
    conn.close()

    if result:
        print("Valid player id!")
        return [[result][0][0]]
    else:
        print("No player")
        return [0]


def get_all_player_ids():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute("SELECT id FROM player")
    result = cursor.fetchall()
    conn.close()

    ids: int = []
    for item in result:
        ids.append(item[0])
    return ids


def get_player_name(id: int):
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute("SELECT character_name FROM player WHERE id = ?", (id,))
    result = cursor.fetchone()
    conn.close()

    return result[0]


def player_select():
    print(
        f"""\033[1;34m
 ___  _                         ___       _           _    _            
| _ \| | __ _  _  _  ___  _ _  / __| ___ | | ___  __ | |_ (_) ___  _ _  
|  _/| |/ _` || || |/ -_)| '_| \__ \/ -_)| |/ -_)/ _||  _|| |/ _ \| ' \ 
|_|  |_|\__,_| \_, |\___||_|   |___/\___||_|\___|\__| \__||_|\___/|_||_|
               |__/                                                     
        \033[0m"""
    )
    print("Please choose a player's name or the ID of a player!")
    print(
        "Examples: -id 15 \033[1;31mOR\033[0m -name John \033[1;31mOR\033[0m if you want all of them: -all "
    )
    print("If you want to go back, enter 0!\n")

    print_players_infos()

    choice = input("Enter your choice:\033[1;34m ")

    data = choice.lstrip().split(" ")
    if data[0] == "-id" and data[1].isdigit():
        return validate_player_id(data[1]), True
    elif data[0] == "-name":
        return validate_player_name(data[1]), True
    elif data[0] == "-all":
        return get_all_player_ids(), False
    elif data[0] == "0":
        return [0], False
    else:
        print("\033[1;31mInvalid choice!\033[0m")
        return [0]


def player_specific_graphs():

    while True:
        print(
            f"""\033[1;34m
 ___ _                     ___              _  __ _       ___               _       
| _ \ |__ _ _  _ ___ _ _  / __|_ __  ___ __(_)/ _(_)__   / __|_ _ __ _ _ __| |_  ___
|  _/ / _` | || / -_) '_| \__ \ '_ \/ -_) _| |  _| / _| | (_ | '_/ _` | '_ \ ' \(_-<
|_| |_\__,_|\_, \___|_|   |___/ .__/\___\__|_|_| |_\__|  \___|_| \__,_| .__/_||_/__/
            |__/              |_|                                     |_|           
            
        \033[0m"""
        )
        print("Please select the type of player specific graph you would like:\n")
        print(f"\033[1;34m[1]\033[0m Leveling progression throughout the game")
        print(f"\033[1;34m[2]\033[0m :)")
        print(f"\033[1;34m[Anything]\033[0m Exit")
        print()

        choice = input("Enter your choice:\033[1;34m ")

        if choice == "1":
            ids, show = player_select()
            bonfire_level(ids, show)
        elif choice == "2":
            print(":)")
        else:
            print(f"\033[0mReturning to main menu...")
            break


def general_graphs():

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
            print(f"\033[0mAverageBossFightTime.png!")
        else:
            print(f"\033[0mReturning to main menu...")
            break


# Convert HH:MM:SS format to total playtime in hours
def time_to_hours(time_str):
    try:
        h, m, s = map(int, time_str.split(":"))
        return h + m / 60 + s / 3600  # Convert to total hours
    except ValueError:
        return None  # Handle incorrect formats gracefully


def bonfire_level(ids: list[int], show: bool = True):
    if ids != [0]:
        for id in ids:

            conn = sqlite3.connect(DB_FILE)
            df = pd.read_sql_query(
                "SELECT timestamp, level FROM bonfires WHERE player_id = ? ORDER BY timestamp",
                conn,
                params=(id,),
            )
            conn.close()

            player_name = get_player_name(id)

            df["playtime_hours"] = df["timestamp"].apply(time_to_hours)

            # Drop rows where playtime is invalid
            # df = df.dropna(subset=["playtime_hours"])

            # Plot playtime progression
            plt.figure(figsize=(10, 5))
            plt.plot(
                df["playtime_hours"],
                df["level"],
                marker="o",
                linestyle="-",
                color="blue",
                label="Level Progression",
            )

            plt.xlabel("Playtime (Hours)")
            plt.ylabel("Level")
            plt.title(f"{player_name}: Level Progression Over Playtime")
            plt.legend()
            plt.grid(True)

            # Save as image
            plt.savefig(f"{player_name}_level_progression.png")
            print(f"\033[0mGenerated {player_name}_level_progression.png!")
            if show == True:
                plt.show()


def normalized_boss_attempts_chart():

    conn = sqlite3.connect("Logging_data.db")
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
    bars = ax.barh(y=df["boss_name"], width=df["attempts_per_player"], color="red")

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
    fig.savefig("NormalizedBossAttempts.png")
    plt.show()


def bossfight_time_chart():

    conn = sqlite3.connect("Logging_data.db")
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
    bars = ax.barh(y=df["boss_name"], width=df["seconds_difference"], color="red")

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
    fig.savefig("AverageBossFightTime.png")
    plt.show()


def deaths():
    conn = sqlite3.connect("Logging_data.db")
    query = "SELECT character_name, total_death FROM player ORDER BY total_death DESC;"
    df = pd.read_sql_query(query, conn)

    # Plot bar chart
    plt.figure(figsize=(10, 5))
    plt.bar(df["character_name"], df["total_death"], color="red")

    # Formatting
    plt.xlabel("Player")
    plt.ylabel("Total Deaths")
    plt.title("Total Deaths per Player")
    plt.xticks(rotation=45)  # Rotate player names for readability
    plt.grid(axis="y", linestyle="--", alpha=0.7)

    # Save as image
    plt.savefig("total_deaths_per_player.png")
    plt.show()
