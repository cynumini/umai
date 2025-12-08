from typing import override
from PySide6.QtCore import (
    QAbstractListModel,
    QAbstractTableModel,
    QModelIndex,
    QObject,
    Slot,
)
from umai.database import Database


class FoodListModel(QAbstractListModel):
    database: Database
    foods: list[tuple[int, str, bool, int]]

    def __init__(self, database: Database, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self.database = database
        self.foods = self.database.get_foods()

    @override
    def rowCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return len(self.foods)

    @override
    def data(self, index: QModelIndex, role: int):  # pyright: ignore[reportIncompatibleMethodOverride]
        return self.foods[index.row()][1]
