import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    title: "umai"
    width: 1280
    height: 720

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

    SplitView {
        anchors.fill: parent
        Rectangle {
            SplitView.minimumWidth: 50
            SplitView.preferredWidth: 200
            color: "red"
        }
        Rectangle {
            SplitView.fillWidth: true
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
                columnSpacing: 1
                rowSpacing: 1
                clip: true
                model: tableModel
                selectionModel: ItemSelectionModel {
                    id: ism
                    model: tableView.model
                    onCurrentRowChanged: {
                        for (var i = 0; i < tableView.columns; i++) {
                            select(tableModel.index(tableView.currentRow, i), ItemSelectionModel.Select);
                        }
                        name.text = tableModel.data(tableModel.index(tableView.currentRow, 0));
                        energy.text = tableModel.data(tableModel.index(tableView.currentRow, 1));
                    }
                }
                delegate: TableViewDelegate {
                    padding: 2
                }
            }
        }
        ColumnLayout {
            SplitView.minimumWidth: 50
            SplitView.preferredWidth: 200
            Label {
                text: "Name"
                Layout.fillWidth: true
            }
            TextField {
                id: name
                Layout.fillWidth: true
            }
            Label {
                text: "Energy"
                Layout.fillWidth: true
            }
            TextField {
                id: energy
                Layout.fillWidth: true
            }
            Item {
                Layout.fillHeight: true
            }
        }
    }

    Component {
        id: add
        Add {}
    }
}
