from typing import override

from PySide6.QtCore import (
    QAbstractTableModel,
    QModelIndex,
    QObject,
    Qt,
    Slot,
)

from umai.database import Database


class TableModel(QAbstractTableModel):
    dishes: list[tuple[int, str, float]]
    database: Database

    def __init__(self, database: Database, parent: QObject | None = None) -> None:
        super().__init__(parent)
        self.database = database
        self.dishes = self.database.get_dishes()

    @override
    def rowCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return len(self.dishes)

    @override
    def columnCount(self, parent) -> int:  # pyright: ignore[reportIncompatibleMethodOverride, reportUnknownParameterType, reportMissingParameterType]
        return 2

    @override
    def data(self, index: QModelIndex, role: int):  # pyright: ignore[reportIncompatibleMethodOverride]
        if role != Qt.ItemDataRole.DisplayRole:
            print(index, Qt.ItemDataRole(role).name)
        return self.dishes[index.row()][index.column() + 1]

    @override
    def headerData(self, section: int, orientation: Qt.Orientation, role: int):  # pyright: ignore[reportIncompatibleMethodOverride]
        if orientation == Qt.Orientation.Vertical:
            return self.dishes[section][0]
        elif orientation == Qt.Orientation.Horizontal:
            return ["Name", "Energy"][section]

    @Slot(str, float)  # pyright: ignore[reportAny]
    def add_dish(self, name: str, energy: float):
        assert type(name) is str
        assert len(name) > 0
        assert type(energy) is float
        assert 1000 > energy > 0

        self.database.add_dish(name, energy)
        index = len(self.dishes)
        self.beginInsertRows(QModelIndex(), index, index)
        self.dishes = self.database.get_dishes()
        self.endInsertRows()
