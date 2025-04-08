import sqlite3
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os

DB_FILE = "Logging_data.db"


def menu():
    """Main menu for Player graphs
    """

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
        print(f"\033[1;34m[2]\033[0m Playtime session lengths")
        print(f"\033[1;34m[3]\033[0m Bossfight defeating lengths")
        print(f"\033[1;34m[4]\033[0m Bossfight attempts")
        print(f"\033[1;34m[Anything]\033[0m Exit")
        print()

        choice = input("Enter your choice:\033[1;34m ")

        if choice == "1":
            ids, show = player_select()
            level_chart(ids, show)
        elif choice == "2":
            ids, show = player_select()
            session_chart(ids, show)
        elif choice == "3":
            ids, show = player_select()
            player_bossfight_time_chart(ids, show)
        elif choice == "4":
            ids, show = player_select()
            player_boss_attempts_chart(ids, show)
        else:
            print(f"\033[0mReturning to main menu...")
            break


def player_select():
    """Brings up a menu, to select which player's graph should be made
    Returns:
        [int]: If valid: [ids], If not: [0]
    """

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


def print_players_infos():
    """Prints all the players in the database
    """

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
    """Checks if a player is in the database by his/her name
    Args:
        name (str): Character name you want to validate
    Returns:
        list[int]: If valid: [id], If not: [0]
    """

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
    """Checks if a player is in the database by his/her id
    Args:
        id (int): Character id you want to validate
    Returns:
        list[int]: If valid: [id], If not: [0]
    """

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
    """Gives back all the player's id's in the database
    Returns:
        list[int]: Containing all the player id's
    """

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
    """Returns a player's name by his/her id
    Args:
        id (int): player's id you want the name of
    Returns:
        string: the character's name
    """

    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute("SELECT character_name FROM player WHERE id = ?", (id,))
    result = cursor.fetchone()
    conn.close()

    return result[0]


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


def level_chart(ids: list[int], show: bool = True):
    """Generates a chart about a player's leveling throughout the game
    Args:
        ids (list[int]): Player id's you want the graph about
        show(bool): If you want to show the complete graph at the end or not
    """
    
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
                color="BlueViolet",
                label="Level Progression",
            )

            plt.xlabel("Playtime (Hours)", fontsize=14, fontweight="bold")
            plt.ylabel("Level", fontsize=14, fontweight="bold")
            plt.title(
                f"{player_name}: Level Progression Over Playtime",
                fontsize=16,
                fontweight="bold",
            )
            plt.legend()
            plt.grid(True)

            # Save as image
            if not os.path.exists("Player_graphs"):
                os.makedirs("Player_graphs")
            plt.savefig(f"Player_graphs/{player_name}({id})_level_progression.png")
            print(f"\033[0mGenerated {player_name}({id})_level_progression.png!")
            if show == True:
                plt.show()


def session_chart(ids: list[int], show: bool = True):
    """Generates a chart about the length of a player's gaming sessions
    Args:
        ids (list[int]): Player id's you want the graph about
        show(bool): If you want to show the complete graph at the end or not
    """
    
    if ids == [0]:
        return

    for id in ids:
        conn = sqlite3.connect(DB_FILE)
        df = pd.read_sql_query(
            """
                SELECT player.character_name, player.total_playtime, sessions.playtime_start
                FROM player
                JOIN sessions ON sessions.player_id = player.id
                WHERE player.id = ?""",
            conn,
            params=(id,),
        )
        player_name = df.at[1, "character_name"]

        # Turning database data to usable
        corrected_values = df["playtime_start"].to_list()
        corrected_values.append(df.at[1, "total_playtime"])
        session_lengths: list[int] = []
        for i in range(len(corrected_values) - 1):
            length = time_to_minutes(corrected_values[i + 1]) - time_to_minutes(
                corrected_values[i]
            )
            if length > 5:
                session_lengths.append(int(length))

        # Making dataframe for chart
        df_sessions = pd.DataFrame(data={"session_length": session_lengths})
        df_sessions["indexes"] = list(range(1, len(df_sessions) + 1))

        # Dynamic size
        fig_width = max(8, len(df) * 0.5)
        fig, ax = plt.subplots(figsize=(fig_width, 5))

        # Text above bars
        bars = plt.bar(
            df_sessions["indexes"], df_sessions["session_length"], color="DarkOrchid"
        )
        for bar in bars:
            height = bar.get_height()
            plt.text(
                bar.get_x() + bar.get_width() / 2,  # X position
                height,  # Y position (top of bar)
                str(height),  # Text to display
                ha="center",
                va="bottom",
                fontsize=9,
            )

        # Formatting
        ax.spines["top"].set_visible(False)
        ax.spines["right"].set_visible(False)
        ax.spines["left"].set_visible(False)
        ax.spines["bottom"].set_color("#DDDDDD")

        ax.set_xlabel(
            f"Total sessions: {len(df_sessions)}", fontsize=14, fontweight="bold"
        )
        ax.set_ylabel("Session length in minutes", fontsize=14, fontweight="bold")
        ax.set_title(
            f"{player_name}: Playsession lenghts", fontsize=16, fontweight="bold"
        )
        ax.tick_params(axis="both", labelsize=12)
        ax.grid(axis="y", linestyle="--", alpha=0.7)

        fig.tight_layout()

        # Save as image
        if not os.path.exists("Player_graphs"):
            os.makedirs("Player_graphs")
        plt.savefig(f"Player_graphs/{player_name}({id})_session_length.png")
        print(f"\033[0mGenerated {player_name}({id})_session_length.png!")
        if show == True:
            plt.show()


def player_bossfight_time_chart(ids: list[int], show: bool = True):
    """Generates a chart about how much time a player needed to defeat a certain boss
    Args:
        ids (list[int]): Player id's you want the graph about
        show(bool): If you want to show the complete graph at the end or not
    """
    
    if ids == [0]:
        return

    for id in ids:
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
        WHERE status IN ('ENTER', 'DEFEATED')  AND player_id = ?
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
        df = pd.read_sql_query(
            query,
            conn,
            params=(id,),
        )
        conn.close()

        player_name = get_player_name(id)

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
        bars = ax.barh(y=df["boss_name"], width=df["seconds_difference"], color="MidnightBlue")

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
        ax.set_title(
            f"{player_name}: boss defeating time in seconds",
            fontsize=16,
            fontweight="bold",
        )
        ax.tick_params(axis="both", labelsize=12)
        ax.grid(axis="x", linestyle="--", alpha=0.7)

        fig.tight_layout()
        # Save as image
        if not os.path.exists("Player_graphs"):
            os.makedirs("Player_graphs")
        plt.savefig(f"Player_graphs/{player_name}({id})_bossfight_length.png")
        print(f"\033[0mGenerated {player_name}({id})_bossfight_length.png!")
        if show == True:
            plt.show()


def player_boss_attempts_chart(ids: list[int], show: bool = True):
    """Generates a chart about how many times a player had to fight a boss to defeat it
    Args:
        ids (list[int]): Player id's you want the graph about
        show(bool): If you want to show the complete graph at the end or not
    """
    
    if ids == [0]:
        return

    for id in ids:

        conn = sqlite3.connect(DB_FILE)
        query = """
                SELECT boss_name, 
                COUNT(*) AS total_attempts, 
                COUNT(DISTINCT player_id) AS num_players
                FROM bossfights
                WHERE status = "ENTER" AND player_id = ?
                GROUP BY boss_name;
            """
        df = pd.read_sql_query(
            query,
            conn,
            params=(id,),
        )
        conn.close()
        
        player_name=get_player_name(id)

        # Handle cases where there might be no data
        if df.empty:
            print("No boss fights recorded in the database.")
            return

        # Adjust figure size dynamically based on number of bosses
        fig_height = max(8, len(df) * 0.5)
        fig, ax = plt.subplots(figsize=(14, fig_height))

        # Create bar chart
        bars = ax.barh(y=df["boss_name"], width=df["total_attempts"], color="Navy")

        # Add labels at the end of the bars
        for bar, value in zip(bars, df["total_attempts"]):
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

        ax.set_xlabel("Number of attempts", fontsize=14, fontweight="bold")
        ax.set_ylabel("Boss Name", fontsize=14, fontweight="bold")
        ax.set_title(
            f"{player_name}: Bossfight attempts", fontsize=16, fontweight="bold"
        )
        ax.tick_params(axis="both", labelsize=12)
        ax.grid(axis="x", linestyle="--", alpha=0.7)

        fig.tight_layout()
        # Save as image
        if not os.path.exists("Player_graphs"):
            os.makedirs("Player_graphs")
        plt.savefig(f"Player_graphs/{player_name}({id})_bossfight_attempts.png")
        print(f"\033[0mGenerated {player_name}({id})_bossfight_attempts.png!")
        if show == True:
            plt.show()
