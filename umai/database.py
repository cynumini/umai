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
            CREATE TABLE "food" (
                "id"             INTEGER NOT NULL UNIQUE,
                "name"           TEXT    NOT NULL,
                "has_ingredient" INTEGER NOT NULL DEFAULT 0,
                "energy"         REAL    NOT NULL,
                PRIMARY KEY("id" AUTOINCREMENT)
            );
            CREATE TABLE "food_ingredient" (
                "id"             INTEGER NOT NULL UNIQUE,
                "food_id"        INTEGER NOT NULL,
                "ingredient_id"  INTEGER NOT NULL,
                "amount"         REAL    NOT NULL DEFAULT 0,
                PRIMARY KEY("id" AUTOINCREMENT),
                FOREIGN KEY("food_id") REFERENCES "food"("id"),
                FOREIGN KEY("ingredient_id") REFERENCES "food"("id")
            );
            """
            _ = cur.executescript(sql)
            con.commit()
        self.con = con
        self.cur = cur

    def add_food(self, name: str, energy: float):
        sql = 'INSERT INTO "food"(name, energy) VALUES (?, ?)'
        _ = self.cur.execute(sql, (name, energy))
        self.con.commit()

    def get_foods(self) -> list[tuple[int, str, int, float]]:
        sql = 'SELECT * FROM "food"'
        res = self.cur.execute(sql)
        return res.fetchall()

    def set_name(self, id: int, new_name: str):
        assert type(new_name) is str
        assert len(new_name) > 0
        sql = 'UPDATE "food" SET "name" = ? WHERE id = ?;'
        _ = self.cur.execute(sql, (new_name, id))
        self.con.commit()

    def set_has_ingredient(self, id: int, has_ingredient: bool):
        assert type(has_ingredient) is bool
        sql = 'UPDATE "food" SET "has_ingredient" = ? WHERE id = ?;'
        _ = self.cur.execute(sql, (has_ingredient, id))
        self.con.commit()

    def set_energy(self, id: int, energy: float):
        assert type(energy) is float
        assert energy > 0
        sql = 'UPDATE "food" SET "energy" = ? WHERE id = ?;'
        _ = self.cur.execute(sql, (energy, id))
        self.con.commit()

    def get_ingredients(self, id: int) -> list[tuple[int, int, float]]:
        sql = 'SELECT "id", "ingredient_id", "amount" FROM "food_ingredient" WHERE "food_id" = ?'
        res = self.cur.execute(sql, (id,))
        return res.fetchall()

    def get_name_from_id(self, id) -> str:
        sql = 'SELECT "name" FROM "food" WHERE "id" = ?'
        res = self.cur.execute(sql, (id,))
        return str(res.fetchone()[0])

    def add_ingredient(self, food_id: int, ingredient_id: int, amount: float):
        sql = """
        INSERT INTO "main"."food_ingredient"
        ("food_id", "ingredient_id", "amount")
        VALUES (?, ?, ?);
        """
        _ = self.cur.execute(sql, (food_id, ingredient_id, amount))
        self.con.commit()

    def delete_ingredient(self, id: int):
        sql = 'DELETE FROM "food_ingredient" WHERE "id" = ?'
        _ = self.cur.execute(sql, (id,))
        self.con.commit()

    def get_food_energy(self, id: int): 
        sql = 'SELECT "energy" FROM "food" WHERE "id" = ?'
        res = self.cur.execute(sql, (id,))
        return float(res.fetchone()[0])

    def __del__(self):
        self.cur.close()
        self.con.close()
