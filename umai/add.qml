import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    title: "Add a new dish"

    GridLayout {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        columns: 2

        Label {
            text: "Name"
        }

        TextField {
            id: name
            Layout.fillWidth: true
        }

        Label {
            text: "Energy"
        }

        TextField {
            id: energy
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhDigitsOnly
            validator: DoubleValidator {
                bottom: 0
                top: 1000
            }
        }
    }

    Row {
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        Button {
            text: "Add"
            onClicked: {
                tableModel.add_dish(name.text, parseFloat(energy.text));
                root.close();
            }
        }

        Button {
            text: "Cancel"
            onClicked: root.close()
        }
    }
}
