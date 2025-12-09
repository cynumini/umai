from typing import override
from PySide6.QtCore import QAbstractTableModel, QModelIndex, QObject, Slot
from umai.database import Database


class IngredientTableModel(QAbstractTableModel):
    database: Database
    ingredients: list[tuple[int, int, float]]
    id: int

    def __init__(self, database: Database, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self.database = database
        self.ingredients = []
        self.id = -1

    @override
    def rowCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return len(self.ingredients)

    @override
    def columnCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return 2

    @override
    def data(self, index: QModelIndex, role: int):  # pyright: ignore[reportIncompatibleMethodOverride]
        match index.column():
            case 0:
                ingredient_id = self.ingredients[index.row()][1]
                name = self.database.get_name_from_id(ingredient_id)
                return name
            case 1:
                amount = self.ingredients[index.row()][2]
                return amount
            case _:
                raise Exception("not implemented yet")

    @Slot(int)  # pyright: ignore[reportAny]
    def set_food(self, id: int) -> None:
        self.beginResetModel()
        self.id = id
        self.ingredients = self.database.get_ingredients(self.id)
        self.endResetModel()

    @Slot(int, int, float)  # pyright: ignore[reportAny]
    def add_ingredient(self, food_id: int, ingredient_id: int, amount: float):
        self.database.add_ingredient(food_id, ingredient_id, amount)
        index = len(self.ingredients)
        self.beginInsertRows(QModelIndex(), index, index)
        self.ingredients = self.database.get_ingredients(self.id)
        self.endInsertRows()

    @Slot(int)
    def delete_ingredient(self, id: int):
        self.beginResetModel()
        self.database.delete_ingredient(id)
        self.ingredients = self.database.get_ingredients(self.id)
        self.endResetModel()

    @Slot(result=int)
    def calc_energy(self):
        total_amount = 0
        total_energy = 0
        for i in self.ingredients:
            energy = self.database.get_food_energy(i[1])
            amount = i[2]
            total_amount += amount
            total_energy += energy / 100 * amount
        return total_energy / total_amount * 100

    @Slot(int, result=int)
    def get_id(self, row: int) -> int:
        assert row >= 0
        return self.ingredients[row][0]
