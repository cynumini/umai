from typing import override
from PySide6.QtCore import (
    QAbstractListModel,
    QModelIndex,
    QObject,
    Qt,
    Slot,
)
from umai.database import Database


class FoodListModel(QAbstractListModel):
    TextRole = Qt.ItemDataRole.UserRole + 1
    ValueRole = Qt.ItemDataRole.UserRole + 2
    database: Database
    foods: list[tuple[int, str, bool, int]]

    def __init__(self, database: Database, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self.database = database
        self.foods = []

    @override
    def rowCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return len(self.foods)

    @override
    def data(self, index: QModelIndex, role: int):  # pyright: ignore[reportIncompatibleMethodOverride]
        row = self.foods[index.row()]
        if role == self.TextRole:
            return f"{row[1]} ({row[3]})"
        elif role == self.ValueRole:
            return row[0]
        else:
            None

    def roleNames(self):
        return {
            self.TextRole: b"Text",
            self.ValueRole: b"Value",
        }

    @Slot(int)  # pyright: ignore[reportAny]
    def set_food(self, id: int) -> None:
        self.beginResetModel()
        foods = self.database.get_foods()
        self.foods = []
        for food in foods:
            if food[0] != id:
                self.foods.append(food)
        self.endResetModel()
