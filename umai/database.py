import sqlite3
from pathlib import Path

import platformdirs


class Database:
    con: sqlite3.Connection
    cur: sqlite3.Cursor

    def __init__(self) -> None:
        data_dir = Path(platformdirs.user_data_dir("umai", "cynumini"))
        data_dir.mkdir(parents=True, exist_ok=True)
        database_file = data_dir / "database.db"
        exist = database_file.exists()
        con = sqlite3.connect(database_file)
        cur = con.cursor()
        if not exist:
            sql = """
            CREATE TABLE "dish" (
                "id"     INTEGER NOT NULL UNIQUE,
                "name"   TEXT    NOT NULL,
                "energy" REAL    NOT NULL,
                PRIMARY KEY("id" AUTOINCREMENT)
            );
            """
            _ = cur.execute(sql)
            con.commit()
        self.con = con
        self.cur = cur

    def add_dish(self, name: str, energy: float):
        sql = 'INSERT INTO "dish"(name, energy) VALUES (?, ?)'
        _ = self.cur.execute(sql, (name, energy))
        self.con.commit()

    def get_dishes(self) -> list[tuple[int, str, float]]:
        sql = 'SELECT * FROM "dish"'
        res = self.cur.execute(sql)
        return res.fetchall()

    def __del__(self):
        self.cur.close()
        self.con.close()
