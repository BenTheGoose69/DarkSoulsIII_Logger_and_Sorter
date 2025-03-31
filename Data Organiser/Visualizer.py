import sqlite3
import pandas as pd
import matplotlib.pyplot as plt
import os


# Connect to SQLite database
conn = sqlite3.connect("Logging_data.db")


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


def general_graphs():
    print(
        f"""\033[1;34m
  ___                             _    ___                   _        
 / __| ___  _ _   ___  _ _  __ _ | |  / __| _ _  __ _  _ __ | |_   ___
| (_ |/ -_)| ' \ / -_)| '_|/ _` || | | (_ || '_|/ _` || '_ \| ' \ (_-<
 \___|\___||_||_|\___||_|  \__,_||_|  \___||_|  \__,_|| .__/|_||_|/__/
                                                      |_|             
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


def main_menu():

    while True:
        print(
            f"""\033[1;34m
 __  __        _         __  __                 
|  \/  | __ _ (_) _ _   |  \/  | ___  _ _  _  _ 
| |\/| |/ _` || || ' \  | |\/| |/ -_)| ' \| || |
|_|  |_|\__,_||_||_||_| |_|  |_|\___||_||_|\_,_|
        \033[0m"""
        )
        print("What kind of graph would you like to generate?\n")
        print(f"\033[1;34m[1]\033[0m General graphs")
        print(f"\033[1;34m[2]\033[0m Player specific graphs")
        print(f"\033[1;34m[3]\033[0m Exit")
        print()

        choice = input("Enter your choice:\033[1;34m ")
        print(f"\033[0m")

        if choice == "1":
            print("You selected Option 1")
        elif choice == "2":
            print("You selected Option 2")
        elif choice == "3":
            print("Exiting... Goodbye!")
            break
        else:
            print("Invalid choice, please try again.")


# Convert HH:MM:SS format to total playtime in hours
def time_to_hours(time_str):
    try:
        h, m, s = map(int, time_str.split(":"))
        return h + m / 60 + s / 3600  # Convert to total hours
    except ValueError:
        return None  # Handle incorrect formats gracefully


def bonfire_level():

    # Query playtime data
    query = """
        SELECT timestamp, level FROM bonfires WHERE player_id = 2 ORDER BY timestamp 
    """
    df = pd.read_sql_query(query, conn)

    df["playtime_hours"] = df["timestamp"].apply(time_to_hours)

    # Drop rows where playtime is invalid
    df = df.dropna(subset=["playtime_hours"])

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
    plt.xlabel("Total Playtime (Hours)")
    plt.ylabel("Level")
    plt.title("Bonfire Level Progression Over Playtime")
    plt.legend()
    plt.grid(True)

    # Save as image
    plt.savefig("bonfire_level_progression.png")
    plt.show()


def bossfights():
    query = """
        SELECT status, COUNT(*) as count FROM bossfights WHERE player_id = 1 GROUP BY status;
    """
    df = pd.read_sql_query(query, conn)

    # Pie chart
    plt.figure(figsize=(6, 6))
    plt.pie(
        df["count"],
        labels=df["status"],
        autopct="%1.1f%%",
        colors=["red", "orange", "green"],
    )
    plt.title("Boss Fight Outcomes")

    # Save as image
    plt.savefig("boss_fight_outcomes.png")
    plt.show()


def deaths():
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


if __name__ == "__main__":
    print_logo()
    main_menu()
