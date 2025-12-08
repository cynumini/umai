from sys import exception
from typing import override

from PySide6.QtCore import (
    QAbstractTableModel,
    QByteArray,
    QModelIndex,
    QObject,
    Qt,
    Slot,
)

from umai.database import Database

StrRole = Qt.ItemDataRole.UserRole + 0
BoolRole = Qt.ItemDataRole.UserRole + 1
FloatRole = Qt.ItemDataRole.UserRole + 2


class TableModel(QAbstractTableModel):
    foods: list[tuple[int, str, bool, int]]
    headers: list[str]
    database: Database

    def __init__(self, database: Database, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self.database = database
        self.foods = self.database.get_foods()
        self.headers = ["Name", "Has ingredient", "Energy"]

    @override
    def rowCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return len(self.foods)

    @override
    def columnCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return len(self.headers)

    @override
    def data(self, index: QModelIndex, role: int):  # pyright: ignore[reportIncompatibleMethodOverride]
        if role != Qt.ItemDataRole.DisplayRole:
            print(index, Qt.ItemDataRole(role).name)
        match index.column():
            case 1:
                return bool(self.foods[index.row()][2])
            case _:
                return self.foods[index.row()][index.column() + 1]

    @override
    def headerData(self, section: int, orientation: Qt.Orientation, role: int):  # pyright: ignore[reportIncompatibleMethodOverride]
        if orientation == Qt.Orientation.Vertical:
            return self.foods[section][0]
        elif orientation == Qt.Orientation.Horizontal:
            return ["Name", "Has ingredient", "Energy"][section]

    @override
    def setData(self, index: QModelIndex, value, role: int) -> bool:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        assert role == Qt.ItemDataRole.EditRole
        id: int = self.foods[index.row()][0]
        match index.column():
            case 0:
                self.database.set_name(id, value)
                self.foods = self.database.get_foods()
                self.dataChanged.emit(index, index)
                return True
            case 1:
                self.database.set_has_ingredient(id, value)
                self.foods = self.database.get_foods()
                self.dataChanged.emit(index, index)
                return True
            case 2:
                self.database.set_energy(id, float(value))
                self.foods = self.database.get_foods()
                self.dataChanged.emit(index, index)
                return True
            case _:
                raise Exception("not implemented yet")
        return False

    @Slot(str, float)  # pyright: ignore[reportAny]
    def add_dish(self, name: str, energy: float):
        assert type(name) is str
        assert len(name) > 0
        assert type(energy) is float
        assert 1000 > energy > 0

        self.database.add_food(name, energy)
        index = len(self.foods)
        self.beginInsertRows(QModelIndex(), index, index)
        self.foods = self.database.get_foods()
        self.endInsertRows()

    @Slot(int, result=int)  # pyright: ignore[reportAny]
    def get_id(self, row: int) -> int:
        return self.foods[row][0]
