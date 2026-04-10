import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.2

Dialog {
    id: finddialog
    title: "Find"
    standardButtons: Dialog.NoButton
    width: 360
    height: 180

    signal doSearch(string text, bool caseSensitive)
    signal clearSearch()

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            
            Label {
                text: "Find: "
            }
            
            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: "Type to search..."
                
                onTextChanged: {
                    if (text.length > 0) {
                        Qt.callLater(function() {
                            finddialog.doSearch(text, caseCheck.checked)
                        })
                    } else {
                        // Clear search when field is empty
                        finddialog.clearSearch()
                        resultLabel.visible = false
                    }
                }
            }
        }

        Label {
            id: resultLabel
            Layout.fillWidth: true
            visible: false
            font.pixelSize: 12
            font.bold: true
        }

        CheckBox {
            id: caseCheck
            text: "Case Sensitive"
            
            onCheckedChanged: {
                if (searchField.text.length > 0) {
                    finddialog.doSearch(searchField.text, checked)
                }
            }
        }

        Item { Layout.fillHeight: true }

        Button {
            text: "Close"
            implicitHeight: 30
            Layout.alignment: Qt.AlignRight
            onClicked: {
                finddialog.close()
            }
        }
    }

    function showResults(total) {
        if (total === 0) {
            resultLabel.text = "No matches found"
            resultLabel.color = "red"
        } else {
            resultLabel.text = "Showing " + total + " matching row" + (total > 1 ? "s" : "")
            resultLabel.color = "#2196F3"
        }
        resultLabel.visible = true
    }

    onVisibleChanged: {
        if (visible) {
            searchField.forceActiveFocus()
        }
    }
}
