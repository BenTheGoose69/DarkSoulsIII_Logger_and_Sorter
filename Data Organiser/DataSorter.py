import os
import sqlite3
from datetime import datetime

DB_FILE = "logging_data.db"

def create_database():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    
    cursor.executescript("""
    CREATE TABLE IF NOT EXISTS player (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      steam_id INTEGER UNIQUE,
      character_name VARCHAR(255),
      total_playtime TIMESTAMP,
      total_death INTEGER DEFAULT 0,
      total_soul_recovery_time INTEGER DEFAULT 0,
      total_souls INTEGER DEFAULT 0
    );

    CREATE TABLE IF NOT EXISTS bossfights (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      player_id INTEGER,
      status TEXT,
      boss_name VARCHAR(255),
      timestamp TIMESTAMP,
      FOREIGN KEY (player_id) REFERENCES player (id)
    );

    CREATE TABLE IF NOT EXISTS bonfires (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      player_id INTEGER,
      bonfire_name VARCHAR(255),
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
    """)
    
    conn.commit()
    conn.close()




def main():
    create_database()

if __name__ == "__main__":
    main()