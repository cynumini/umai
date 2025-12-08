from typing import override
from PySide6.QtCore import QAbstractTableModel, QModelIndex, QObject, Slot
from umai.database import Database


class IngredientTableModel(QAbstractTableModel):
    database: Database
    ingredients: list[tuple[int, int, float]]

    def __init__(self, database: Database, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self.database = database
        self.ingredients = []

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
        self.ingredients = self.database.get_ingredients(id)
        print(self.ingredients)
        self.endResetModel()
