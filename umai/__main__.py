import sys
from PySide6.QtCore import QObject, Slot
from PySide6.QtGui import QGuiApplication
from PySide6.QtQml import QQmlApplicationEngine

from umai.database import Database
from umai.table_model import TableModel


class Logic(QObject):
    database: Database

    def __init__(self, database: Database):
        self.database = database
        super().__init__()

    @Slot()  # pyright: ignore[reportAny]
    def hello_world(self) -> None:
        print("hello world")


def main() -> int:
    app = QGuiApplication()
    engine = QQmlApplicationEngine()
    engine.addImportPath(sys.path[0])

    database = Database()
    table_model = TableModel(database)
    logic = Logic(database)

    root_context = engine.rootContext()
    root_context.setContextProperty("logic", logic)
    root_context.setContextProperty("tableModel", table_model)

    engine.loadFromModule("umai", "umai")

    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
