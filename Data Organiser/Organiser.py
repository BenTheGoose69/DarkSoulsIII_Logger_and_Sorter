import os
import sqlite3
from datetime import datetime
import sys
import shutil

DB_FILE = "Logging_data.db"
UNPROCESSED_DATA = "Unprocessed_data"
PROCESSED_DATA = "Processed_data"



def create_database():
    """Creates the main logging database
    """
    
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.executescript(
        """
    CREATE TABLE IF NOT EXISTS player (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      steam_id INTEGER,
      character_name VARCHAR(100),
      total_playtime TIMESTAMP,
      total_death INTEGER DEFAULT 0,
      total_soul_recovery_time INTEGER DEFAULT 0,
      total_souls INTEGER DEFAULT 0
    );

    CREATE TABLE IF NOT EXISTS bossfights (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      player_id INTEGER,
      status VARCHAR(10),
      boss_name VARCHAR(30),
      timestamp TIMESTAMP,
      FOREIGN KEY (player_id) REFERENCES player (id)
      UNIQUE(player_id, status ,timestamp)
    );

    CREATE TABLE IF NOT EXISTS bonfires (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      player_id INTEGER,
      bonfire_name VARCHAR(50),
      level INTEGER,
      total_souls INTEGER,
      timestamp TIMESTAMP,
      FOREIGN KEY (player_id) REFERENCES player (id)
      UNIQUE(player_id, bonfire_name,timestamp)
    );

    CREATE TABLE IF NOT EXISTS sessions (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      player_id INTEGER,
      playtime_start TIMESTAMP,
      FOREIGN KEY (player_id) REFERENCES player (id)
      UNIQUE(player_id, playtime_start)
    );
    """
    )

    conn.commit()
    conn.close()


def process_txt_files(directory):
    """Searches for all the .txt files in the directory and gives it to 
    other functions for them to process the data
    Args:
        directory (string): The directory in which the logs are saved
    """
    
    for filename in os.listdir(directory):
        if filename.endswith(".txt"):
            # print(os.path.join(directory, filename))
            parse_txt_files(os.path.join(directory, filename))
            move_processed_file(filename)


def parse_txt_files(file_path):
    """Opens and parses .txt file
    Args:
        filepath (string): The filepath of the file you want to parse
    """
    
    with open(file_path, "r") as file:
        organise_data(file, file_path)


def organise_data(file, file_path):
    """Organises the data and puts it into the database
    Args:
        file (file): The log file you want to oraginse
        file_path (string): The filepath of the log
    """
    
    player_data = {
        "id": "",
        "steam_id": "",
        "character_name": "",
        "total_playtime": "",
        "total_death": 0,
        "total_soul_recovery_time": 0,
        "total_souls": 0,
    }
    player_data["character_name"], player_data["steam_id"] = (
        str(file_path).rsplit("\\")[1].rsplit("]")[0].rsplit("[")
    )

    player_data = check_player_in_database(player_data)

    print(f"\033[1;34m{player_data["character_name"]} \033[0m{player_data["steam_id"]}")

    lines = iter(file)
    for i, line in enumerate(lines, 1):
        sys.stdout.write(f"\r\033[1;31mLines Scanned: {i}\033[0m")
        sys.stdout.flush()

        if not line.strip():
            continue

        timestamp = ""
        match line.strip():

            # Death
            case "#":
                death_number = int(next(lines).strip())
                timestamp = next(lines).strip()
                if death_number > player_data["total_death"]:
                    player_data["total_death"] = death_number

                # print(f"Player died [{death_number}] at {timestamp} ")

            # Bonfire
            case "##":
                bonfire_name = next(lines).strip()
                level = int(next(lines).strip())
                currrent_souls = int(next(lines).strip())
                total_souls = int(next(lines).strip())
                timestamp = next(lines).strip()

                add_bonfire_rest(
                    player_data["id"], bonfire_name, level, total_souls, timestamp
                )
                if player_data["total_souls"] < total_souls:
                    player_data["total_souls"] = total_souls

                # print(f"Player rested at [{bonfire_name}] at level [{level}] with [{total_souls}] total souls at {timestamp}")

            # Soul recovery
            case "###":
                player_data["total_soul_recovery_time"] += 1
                recovered_souls = int(next(lines).strip())
                timestamp = next(lines).strip()

                # print(f"Player recovered [{recovered_souls}] souls at {timestamp} ")

            # Bossfight
            case "####":
                bossfight_status = next(lines).strip()
                boss_name = next(lines).strip()
                timestamp = next(lines).strip()
                add_bossfight(player_data["id"], bossfight_status, boss_name, timestamp)

                # print(f"Player {bossfight_status} {boss_name} at {timestamp} ")

            # Session start
            case "#####":
                timestamp = next(lines).strip()
                add_session(player_data["id"], timestamp)

                # print(f"Player started playing at {timestamp} ")

        if str(timestamp) > str(player_data["total_playtime"]):
            player_data["total_playtime"] = timestamp

    print()

    # print(f"Total deaths: {player_data['total_death']} ")
    # print(f"Total soul recoveries: {player_data['total_soul_recovery_time']} ")
    # print(player_data)

    update_player_in_database(player_data)


def check_player_in_database(player_data):
    """Checks, whether a player is in the database. By the end, the player will be in the database
    Args:
        player_data (player_data): The basic data about a player
    """

    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute(
        "SELECT * FROM player WHERE steam_id = ? AND character_name = ?",
        (player_data["steam_id"], player_data["character_name"]),
    )
    result = cursor.fetchone()

    if result:
        player_data["id"] = result[0]
        (
            player_data["total_playtime"],
            player_data["total_death"],
            player_data["total_soul_recovery_time"],
            player_data["total_souls"],
        ) = result[3:7]

    else:
        cursor.execute(
            "INSERT INTO player (steam_id, character_name) VALUES (?, ?)",
            (player_data["steam_id"], player_data["character_name"]),
        )
        conn.commit()
        player_data["id"] = cursor.lastrowid

    conn.close()
    return player_data


def update_player_in_database(player_data):
    """Updates a player's info in the database
    Args:
        player_data (player_data): The basic data about a player 
    """
    
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute(
        """
        UPDATE player 
        SET total_playtime = ?, total_death = ?, total_soul_recovery_time = ?, total_souls = ? 
        WHERE steam_id = ? AND character_name = ?
        """,
        (
            player_data["total_playtime"],
            player_data["total_death"],
            player_data["total_soul_recovery_time"],
            player_data["total_souls"],
            player_data["steam_id"],
            player_data["character_name"],
        ),
    )
    conn.commit()
    conn.close()


def add_bonfire_rest(player_id, bonfire_name, level, total_souls, timestamp):
    """Adds a bonfire rest to a player in the database
    Args:
        player_id(int): The ID of a player
        bonfire_name(string): The name of the bonfire rested at
        level(int): The level of the player at the bonfire 
        total_souls(int): All the souls the player collected in the game 
        timestamp(timestamp): A timestamp containing the ingame time
    """
    
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute(
        "INSERT  OR IGNORE INTO bonfires (player_id, bonfire_name, level, total_souls, timestamp) VALUES (?,?,?,?, ?)",
        (player_id, bonfire_name, level, total_souls, timestamp),
    )
    conn.commit()


def add_bossfight(player_id, bossfight_status, boss_name, timestamp):
    """Adds a bossfight event to a player in the database
    Args:
        player_id(int): The ID of a player 
        bossfight_status(string): can be ENTER / DIED / DEFEATED 
        boss_name(string): can be NIL / the name of the boss 
        timestamp(timestamp): A timestamp containing the ingame time
    """
    
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute(
        "INSERT  OR IGNORE INTO bossfights (player_id, status, boss_name, timestamp) VALUES (?,?,?,?)",
        (player_id, bossfight_status, boss_name, timestamp),
    )
    conn.commit()

    if boss_name != "NIL":
        cursor.execute(
            "SELECT boss_name FROM bossfights WHERE player_id = ? AND boss_name = ?",
            (player_id, boss_name),
        )
        result = cursor.fetchall()
        if result.count((boss_name,)) < 2:
            cursor.execute(
                "UPDATE bossfights SET boss_name = ? WHERE player_id = ? AND boss_name = 'NIL'",
                (boss_name, player_id),
            )
            conn.commit()
    conn.close()


def add_session(player_id, timestamp):
    """Adds a session start to a player in the database
    Args:
        player_id(int): The ID of a player 
        timestamp(timestamp): A timestamp containing the ingame time
    """
    
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute(
        "INSERT OR IGNORE INTO sessions (player_id, playtime_start) VALUES (?,?)",
        (player_id, timestamp),
    )
    conn.commit()



def move_processed_file(filename):
    """Moves the a file from UNPROCESSED_DATA to PROCESSED_DATA
    Args:
        filename (string): name of file to be moved
    """
    
    shutil.move(f"{UNPROCESSED_DATA}\\{filename}", f"{PROCESSED_DATA}\\{filename}")


def reset_txt_files(directory):
    """Move the processed logs back to their origin
    Args:
        directory (string): The directory where the files are 
    """
    
    for filename in os.listdir(directory):
        if filename.endswith(".txt"):
            move_back_processed_file(filename)


def move_back_processed_file(filename):
    """Moves back the processed file
    Args:
        filename (string): The name of the file to be moved back
    """
    
    shutil.move(f"{PROCESSED_DATA}\\{filename}", f"{UNPROCESSED_DATA}\\{filename}")


def delete_db():
    """deletes the database
    """
    
    if os.path.isfile(DB_FILE):
        os.remove(DB_FILE)


def organise_main():
    """The main function of the Organiser, starts the whole process and puts it in motion
    """
    
    print(f"\033[95mData organisation started...")
    create_database()
    process_txt_files(UNPROCESSED_DATA)
    print(f"\033[95mOrganisation finished!\033[0m")


def unorganise_main():
    """The opposite of the orginise_main function
    """
    
    print(f"\033[95mData reset started...\033[0m")
    delete_db()
    reset_txt_files(PROCESSED_DATA)
    print(f"\033[95mReset finished!\033[0m")
