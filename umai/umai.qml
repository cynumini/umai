import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    title: "umai"
    width: 1280
    height: 720
    property int currentFood: -1

    Component.onCompleted: {
        // logic.hello_world()
        // console.log(tableView.height, tableView.width)
    }

    function toggle_has_ingredient(state) {
        if (state) {
            energy.enabled = false;
        } else {
            energy.enabled = true;
        }
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
                    onCurrentChanged: function (current, _) {
                        for (var i = 0; i < tableView.columns; i++) {
                            select(tableModel.index(current.row, i), ItemSelectionModel.Select);
                        }
                        if (ism.hasSelection) {
                            name.text = tableModel.data(tableModel.index(current.row, 0));
                            var has_ingredient_value = tableModel.data(tableModel.index(current.row, 1));
                            toggle_has_ingredient(has_ingredient_value);
                            if (has_ingredient_value == true) {
                                has_ingredient.checkState = Qt.Checked;
                            } else {
                                has_ingredient.checkState = Qt.Unchecked;
                            }
                            energy.text = tableModel.data(tableModel.index(current.row, 2));
                            var id = tableModel.get_id(current.row);
                            root.currentFood = id;
                            ingredientTableModel.set_food(id);
                            foodListModel.set_food(id);
                            cb.currentIndex = 0;
                        } else {
                            root.currentFood = -1;
                        }
                    }
                    onSelectionChanged: function (selected, _) {
                        if (selected.length == 0) {
                            edit.visible = false;
                            clearCurrentIndex();
                        } else {
                            edit.visible = true;
                        }
                    }
                }
                delegate: TableViewDelegate {
                    padding: 2
                }
            }
        }
        ColumnLayout {
            id: edit
            visible: false
            SplitView.minimumWidth: 50
            Label {
                text: "Food"
                Layout.fillWidth: true
                font.pixelSize: 24
            }
            SplitView.preferredWidth: 200
            Label {
                text: "Name"
                Layout.fillWidth: true
            }
            TextField {
                id: name
                Layout.fillWidth: true
                onEditingFinished: {
                    var index = ism.selectedIndexes[0];
                    if (ism.hasSelection) {
                        tableModel.setData(index, name.text, Qt.EditRole);
                    }
                }
            }
            Label {
                text: "Energy"
                Layout.fillWidth: true
            }
            TextField {
                id: energy
                Layout.fillWidth: true
                onEditingFinished: {
                    var index = ism.selectedIndexes[2];
                    if (ism.hasSelection) {
                        tableModel.setData(index, energy.text, Qt.EditRole);
                    }
                }
            }
            CheckBox {
                id: has_ingredient
                checked: false
                text: "Has ingredient"
                nextCheckState: function () {
                    var index = ism.selectedIndexes[1];
                    if (ism.hasSelection) {
                        var newState = checkState === Qt.Checked ? Qt.Unchecked : Qt.Checked;
                        var has_ingredient_value = checkState === Qt.Unchecked;
                        tableModel.setData(index, has_ingredient_value, Qt.EditRole);
                        toggle_has_ingredient(has_ingredient_value);
                        return newState;
                    }
                }
            }
            Label {
                text: "Ingredients"
                Layout.fillWidth: true
                font.pixelSize: 24
            }
            GridLayout {
                columns: 2

                Label {
                    text: "Ingredient"
                }

                ComboBox {
                    id: cb
                    Layout.fillWidth: true
                    editable: true
                    model: foodListModel
                    textRole: "Text"
                    valueRole: "Value"
                }

                Label {
                    text: "Amount"
                }

                TextField {
                    id: amount
                    Layout.fillWidth: true
                    text: "100"
                    inputMethodHints: Qt.ImhDigitsOnly
                    validator: DoubleValidator {
                        bottom: 0
                        top: 1000
                    }
                }
            }
            Button {
                Layout.fillWidth: true
                text: "Calc"
                onClicked: function () {
                    var index = ism.selectedIndexes[2];
                    if (ism.hasSelection) {
                        energy.text = ingredientTableModel.calc_energy();
                        tableModel.setData(index, energy.text, Qt.EditRole);
                    }
                }
            }
            Button {
                Layout.fillWidth: true
                text: "Add"
                onClicked: function () {
                    ingredientTableModel.add_ingredient(root.currentFood, cb.currentValue, amount.text);
                }
            }
            Button {
                Layout.fillWidth: true
                text: "Delete selected"
                onClicked: function () {
                    var id = ingredientTableModel.get_id(itv.currentRow);
                    ingredientTableModel.delete_ingredient(id);
                }
            }
            TableView {
                id: itv
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 100
                model: ingredientTableModel
                clip: true
                selectionModel: ItemSelectionModel {
                    model: itv.model
                    onCurrentChanged: function (current, _) {
                        for (var i = 0; i < tableView.columns; i++) {
                            select(ingredientTableModel.index(current.row, i), ItemSelectionModel.Select);
                        }
                    }
                    onSelectionChanged: function (selected, _) {
                        if (selected.length == 0) {
                            clearCurrentIndex();
                        }
                    }
                }
                delegate: TableViewDelegate {
                    padding: 2
                }
            }
        }
    }

    Component {
        id: add
        Add {}
    }
}
