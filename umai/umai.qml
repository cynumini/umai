import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    title: "umai"

    Component.onCompleted: {
        // logic.hello_world()
        // console.log(tableView.height, tableView.width)
    }

    menuBar: MenuBar {
        Menu {
            title: "File"
            Action {
                text: "&New..."
                onTriggered: {
                    var window = add.createObject(root);
                    window.show();
                }
            }
            MenuSeparator {}
            Action {
                text: "&Quit"
                onTriggered: Qt.quit()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        HorizontalHeaderView {
            id: horizontalHeader
            anchors.left: tableView.left
            anchors.top: parent.top
            syncView: tableView
            clip: true
        }

        VerticalHeaderView {
            id: verticalHeader
            anchors.top: tableView.top
            anchors.left: parent.left
            syncView: tableView
            clip: true
        }

        TableView {
            id: tableView
            anchors.left: verticalHeader.right
            anchors.top: horizontalHeader.bottom
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            model: tableModel
            delegate: TableViewDelegate {
                padding: 4
            }
        }
    }
    Component {
        id: add
        Add {}
    }
}
