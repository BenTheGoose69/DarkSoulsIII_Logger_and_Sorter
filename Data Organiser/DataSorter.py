import os
import sqlite3
from datetime import datetime

DB_FILE = "Logging_data.db"
UNPROCESSED_DATA = "Unprocessed_data"
PROCESSED_DATA = "Processed_data"


def create_database():
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
      death_count INTEGER NULL,
      boss_name VARCHAR(30),
      timestamp TIMESTAMP,
      FOREIGN KEY (player_id) REFERENCES player (id)
    );

    CREATE TABLE IF NOT EXISTS bonfires (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      player_id INTEGER,
      bonfire_name VARCHAR(50),
      level INTEGER,
      total_souls INTEGER,
      timestamp TIMESTAMP,
      FOREIGN KEY (player_id) REFERENCES player (id)
    );

    CREATE TABLE IF NOT EXISTS sessions (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      player_id INTEGER,
      playtime_start TIMESTAMP,
      FOREIGN KEY (player_id) REFERENCES player (id)
    );
    """
    )

    conn.commit()
    conn.close()


def parse_txt_files(file_path):
    with open(file_path, "r") as file:
        organise_data(file, file_path)


def organise_data(file, file_path):
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

    lines = iter(file)
    for line in lines:

        match line.strip():

            # Death
            case "#":
                death_number = int(next(lines).strip())
                time = next(lines).strip()
                print(f"Player died [{death_number}] at {time} ")
                if death_number > player_data["total_death"]:
                    player_data["total_death"] = death_number

            # Bonfire
            case "##":
                bonfire_name = next(lines).strip()
                level = int(next(lines).strip())
                currrent_souls = int(next(lines).strip())
                total_souls = int(next(lines).strip())
                timestamp = next(lines).strip()
                print(
                    f"Player rested at [{bonfire_name}] at level [{level}] with [{total_souls}] total souls at {timestamp}"
                )

            # Soul recovery
            case "###":
                player_data["total_soul_recovery_time"] += 1
                recovered_souls = int(next(lines).strip())
                timestamp = next(lines).strip()
                print(f"Player recovered [{recovered_souls}] souls at {timestamp} ")

            # Bossfight
            case "####":
                bossfight_status = next(lines).strip()
                boss_name = next(lines).strip()
                timestamp = next(lines).strip()
                print(f"Player {bossfight_status} {boss_name} at {timestamp} ")

            # Session start
            case "#####":
                session_start = next(lines).strip()
                print(f"Player started playing at {session_start} ")

    print(f"Total deaths: {player_data['total_death']} ")
    print(f"Total soul recoveries: {player_data['total_soul_recovery_time']} ")
    print(player_data)

    update_player_data(player_data)


def check_player_in_database(player_data):

    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute(
        "SELECT * FROM player WHERE steam_id = ? AND character_name = ?",
        (player_data["steam_id"], player_data["character_name"]),
    )
    result = cursor.fetchone()

    if result:
        print(result)
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


def update_player_data(player_data):
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute(
        """
        UPDATE player 
        SET total_playtime = ?, total_death = ?, total_soul_recovery_time = ?, total_souls = ? 
        WHERE steam_id = ? AND character_name = ?""",
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


"""
* Explanation for the logging:
*
* #
* death number
* when it happened
*
* ##
* resting at which bonfire
* current level
* current souls
* total souls
* when it happened
*
* ###
* recovered souls
* when it happened
*
* ####
* ENTERED / DIED / DEFEATED (bossfights)
* boss name / NIL
* when it happened
*
* #####
* Sat down to play at what time
"""


def process_txt_files(directory):
    for filename in os.listdir(directory):
        if filename.endswith(".txt"):
            print(os.path.join(directory, filename))
            parse_txt_files(os.path.join(directory, filename))


def main():
    create_database()
    process_txt_files(UNPROCESSED_DATA)


if __name__ == "__main__":
    main()
