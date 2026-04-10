import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

Item {
    anchors.fill: parent

    FileDialog {
        id: filePickerDialog
        title: "Select File"
        folder: shortcuts.home
        selectMultiple: false
        property string targetField: ""
        onAccepted: {
            var path = fileUrl.toString().replace(/^(file:\/{2})/, "")
            switch (targetField) {
                case "identityCertAuthority": identityCertAuthorityField.text = path; break
                case "certRevocation":        certRevocationField.text        = path; break
                case "privateKey":            privateKeyField.text            = path; break
                case "identityCert":          identityCertField.text          = path; break
                case "permsCertAuthority":    permsCertAuthorityField.text    = path; break
                case "governanceDoc":         governanceDocField.text         = path; break
                case "permissionsDoc":        permissionsDocField.text        = path; break
            }
        }
    }

    Rectangle {
        id: mainRectangle
        color: "#F2F2F2"
        anchors.fill: parent

        // secEnabled defined here — AFTER mainRectangle, avoids forward reference
        property bool secEnabled: enableSecurityCheck.checked

        // ── Header Bar ─────────────────────────────────────────────────
        Rectangle {
            id: upperRectangle
            width: parent.width
            height: 60
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            border.color: "lightgray"
            color: "#F2F2F2"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Text { text: "Security"; color: "darkgray"; bottomPadding: 15 }
                Item { Layout.fillWidth: true }

                Row {
                    spacing: 15
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    Image { source: "qrc:/resources/images/icons/refresh_arrow.png"; width: 20; height: 20; MouseArea { anchors.fill: parent } }
                    Image { source: "qrc:/resources/images/icons/left_arrow.png";    width: 20; height: 20; MouseArea { anchors.fill: parent } }
                    Image { source: "qrc:/resources/images/icons/down_arrow.png";    width: 20; height: 20; MouseArea { anchors.fill: parent } }
                    Image { source: "qrc:/resources/images/icons/dots.png";          width: 20; height: 20; MouseArea { anchors.fill: parent } }
                }
            }
        }

        // ── Bottom Bar (declared before ScrollView so anchors resolve) ─
        Rectangle {
            id: bottomBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "white"
            border.color: "#E8E9EB"; border.width: 1

            RowLayout {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 16; spacing: 8

                Button {
                    id: restoreDefaultsBtn; text: "Restore Defaults"
                    implicitWidth: 130; implicitHeight: 35
                    background: Rectangle { radius: 5; color: "white"; border.color: "darkgray"; border.width: 1 }
                    contentItem: Text { text: restoreDefaultsBtn.text; color: "black"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: {
                        domainFilterField.text = "*"
                        securityExpirationField.text = "3600"
                        keyRevisionDepthField.text = "8"
                        preSharedIdField.text = "0"
                        preSharedSeedField.text = ""
                        identityCertAuthorityField.text = ""
                        certRevocationField.text = ""
                        privateKeyField.text = ""
                        identityCertField.text = ""
                        permsCertAuthorityField.text = ""
                        governanceDocField.text = ""
                        permissionsDocField.text = ""
                        keyAlgorithmCombo.currentIndex = 0
                        encryptionAlgoCombo.currentIndex = 0
                        securityLoggingCombo.currentIndex = 0
                        shapesSecurityCombo.currentIndex = 0
                        privateKeyPasswordCheck.checked = false
                        keyRevisionCheck.checked = false
                        preSharedCheck.checked = false
                        enableSecurityCheck.checked = false
                    }
                }

                Button {
                    id: applyBtn; text: "Apply"
                    implicitWidth: 80; implicitHeight: 35
                    background: Rectangle { radius: 5; color: "white"; border.color: "darkgray"; border.width: 1 }
                    contentItem: Text { text: applyBtn.text; color: "black"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    onClicked: {}
                }
            }
        }

        // ── ScrollView ─────────────────────────────────────────────────
        ScrollView {
            id: scrollView
            anchors.top: upperRectangle.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomBar.top
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // ── Item wrapper gives ScrollView a concrete height ────────
            Item {
                id: scrollContent
                width: scrollView.width
                height: mainCol.implicitHeight + 40

                ColumnLayout {
                    id: mainCol
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 16
                    anchors.topMargin: 10
                    spacing: 12

                    // ── Enable Security Checkbox ───────────────────────
                    CheckBox {
                        id: enableSecurityCheck
                        Layout.fillWidth: true
                        checked: false
                        indicator: Rectangle {
                            implicitWidth: 16; implicitHeight: 16
                            x: enableSecurityCheck.leftPadding
                            y: parent.height / 2 - height / 2
                            border.color: enableSecurityCheck.checked ? "#DA3450" : "#34495e"
                            border.width: 2; radius: 2
                            color: enableSecurityCheck.checked ? "#DA3450" : "white"
                            Text { anchors.centerIn: parent; text: enableSecurityCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                        }
                        contentItem: Text {
                            text: "Enable Security for specified Domains"
                            leftPadding: enableSecurityCheck.indicator.width + enableSecurityCheck.spacing + 4
                            verticalAlignment: Text.AlignVCenter; color: "black"; font.pixelSize: 12
                        }
                    }

                    // ── Domain Filter ──────────────────────────────────
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Domain Filter:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: domainFilterField
                            Layout.fillWidth: true; height: 30; text: "*"; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (domainFilterField.activeFocus ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; radius: 2
                            }
                        }
                    }

                    // ── Divider: Authentication ────────────────────────
                    RowLayout { Layout.fillWidth: true; spacing: 8
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                        Text { text: "Authentication"; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12 }
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                    }

                    // Key Establishment Algorithm
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Key Establishment Algorithm:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        ComboBox {
                            id: keyAlgorithmCombo
                            Layout.fillWidth: true; height: 30; enabled: mainRectangle.secEnabled
                            model: ["AUTO", "ECDH+prime256v1-CEUM", "ECDH+X25519-CEUM", "RSA+2048"]; currentIndex: 0
                            background: Rectangle { radius: 3; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1 }
                            contentItem: Text { leftPadding: 8; text: keyAlgorithmCombo.displayText; verticalAlignment: Text.AlignVCenter; color: mainRectangle.secEnabled ? "black" : "gray" }
                            delegate: ItemDelegate {
                                width: keyAlgorithmCombo.width
                                contentItem: Text { text: modelData; color: "black"; verticalAlignment: Text.AlignVCenter; leftPadding: 8 }
                                highlighted: keyAlgorithmCombo.highlightedIndex === index
                                background: Rectangle { color: highlighted ? "#DA3450" : "white" }
                            }
                        }
                    }

                    // Identity Certificate Authority (red when empty + enabled)
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Identity Certificate Authority:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: identityCertAuthorityField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (identityCertAuthorityField.text === "" ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? (identityCertAuthorityField.text === "" ? "#DA3450" : "white") : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 30; height: 30; radius: 3
                            color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "📁"; font.pixelSize: 14; opacity: mainRectangle.secEnabled ? 1.0 : 0.4 }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { filePickerDialog.targetField = "identityCertAuthority"; filePickerDialog.open() } }
                        }
                    }

                    // Certificate Revocation List
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Certificate Revocation List:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: certRevocationField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (certRevocationField.activeFocus ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 30; height: 30; radius: 3
                            color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "📁"; font.pixelSize: 14; opacity: mainRectangle.secEnabled ? 1.0 : 0.4 }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { filePickerDialog.targetField = "certRevocation"; filePickerDialog.open() } }
                        }
                    }

                    // Private Key (red when empty + enabled)
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Private Key:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: privateKeyField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (privateKeyField.text === "" ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? (privateKeyField.text === "" ? "#DA3450" : "white") : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 30; height: 30; radius: 3
                            color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "📁"; font.pixelSize: 14; opacity: mainRectangle.secEnabled ? 1.0 : 0.4 }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { filePickerDialog.targetField = "privateKey"; filePickerDialog.open() } }
                        }
                    }

                    // Private Key Password
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        CheckBox {
                            id: privateKeyPasswordCheck
                            checked: false; enabled: mainRectangle.secEnabled
                            indicator: Rectangle {
                                implicitWidth: 16; implicitHeight: 16
                                x: privateKeyPasswordCheck.leftPadding; y: parent.height / 2 - height / 2
                                border.color: !mainRectangle.secEnabled ? "lightgray" : (privateKeyPasswordCheck.checked ? "#DA3450" : "#34495e")
                                border.width: 1; radius: 2
                                color: !mainRectangle.secEnabled ? "#F0F0F0" : (privateKeyPasswordCheck.checked ? "#DA3450" : "white")
                                Text { anchors.centerIn: parent; text: privateKeyPasswordCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                            }
                            contentItem: Text {
                                text: "Private Key Password:"
                                leftPadding: privateKeyPasswordCheck.indicator.width + privateKeyPasswordCheck.spacing + 4
                                verticalAlignment: Text.AlignVCenter; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12
                            }
                        }
                        TextField {
                            id: privateKeyPasswordField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            echoMode: TextInput.Password
                            enabled: mainRectangle.secEnabled && privateKeyPasswordCheck.checked
                            verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: (mainRectangle.secEnabled && privateKeyPasswordCheck.checked) ? "darkgray" : "lightgray"
                                border.width: 1; color: (mainRectangle.secEnabled && privateKeyPasswordCheck.checked) ? "white" : "#F0F0F0"; radius: 2
                            }
                        }
                    }

                    // Identity Certificate (red when empty + enabled)
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Identity Certificate:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: identityCertField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (identityCertField.text === "" ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? (identityCertField.text === "" ? "#DA3450" : "white") : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 30; height: 30; radius: 3
                            color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "📁"; font.pixelSize: 14; opacity: mainRectangle.secEnabled ? 1.0 : 0.4 }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { filePickerDialog.targetField = "identityCert"; filePickerDialog.open() } }
                        }
                    }

                    // Security Advance Notice Expiration
                    RowLayout {
                        Layout.fillWidth: true; spacing: 6
                        Text { text: "Security Advance Notice Expiration (seconds)"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; wrapMode: Text.WordWrap; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: securityExpirationField
                            Layout.fillWidth: true; height: 30; text: "3600"; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            validator: IntValidator { bottom: 0; top: 999999 }
                            background: Rectangle { border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; radius: 2 }
                        }
                        Rectangle {
                            width: 28; height: 30; color: "transparent"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 16; color: mainRectangle.secEnabled ? "black" : "gray" }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { var v = parseInt(securityExpirationField.text)||0; securityExpirationField.text = Math.max(v-1,0).toString() } }
                        }
                        Rectangle {
                            width: 28; height: 30; color: "transparent"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 16; color: mainRectangle.secEnabled ? "black" : "gray" }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { var v = parseInt(securityExpirationField.text)||0; securityExpirationField.text = (v+1).toString() } }
                        }
                    }

                    // ── Divider: Access Control ────────────────────────
                    RowLayout { Layout.fillWidth: true; spacing: 8
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                        Text { text: "Access Control"; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12 }
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                    }

                    // Permissions Certificate Authority (red when empty + enabled)
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Permissions Certificate Authority:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: permsCertAuthorityField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (permsCertAuthorityField.text === "" ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? (permsCertAuthorityField.text === "" ? "#DA3450" : "white") : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 30; height: 30; radius: 3
                            color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "📁"; font.pixelSize: 14; opacity: mainRectangle.secEnabled ? 1.0 : 0.4 }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { filePickerDialog.targetField = "permsCertAuthority"; filePickerDialog.open() } }
                        }
                    }

                    // Governance Document (red when empty + enabled)
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Governance Document:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: governanceDocField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (governanceDocField.text === "" ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? (governanceDocField.text === "" ? "#DA3450" : "white") : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 30; height: 30; radius: 3
                            color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "📁"; font.pixelSize: 14; opacity: mainRectangle.secEnabled ? 1.0 : 0.4 }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { filePickerDialog.targetField = "governanceDoc"; filePickerDialog.open() } }
                        }
                    }

                    // Permissions Document (red when empty + enabled)
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Permissions Document:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        TextField {
                            id: permissionsDocField
                            Layout.fillWidth: true; height: 30; leftPadding: 8
                            enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                            background: Rectangle {
                                border.color: mainRectangle.secEnabled ? (permissionsDocField.text === "" ? "#DA3450" : "darkgray") : "lightgray"
                                border.width: 1; color: mainRectangle.secEnabled ? (permissionsDocField.text === "" ? "#DA3450" : "white") : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 30; height: 30; radius: 3
                            color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "📁"; font.pixelSize: 14; opacity: mainRectangle.secEnabled ? 1.0 : 0.4 }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { filePickerDialog.targetField = "permissionsDoc"; filePickerDialog.open() } }
                        }
                    }

                    // ── Divider: Cryptography ──────────────────────────
                    RowLayout { Layout.fillWidth: true; spacing: 8
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                        Text { text: "Cryptography"; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12 }
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                    }

                    // Encryption Algorithm
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Encryption Algorithm:"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        ComboBox {
                            id: encryptionAlgoCombo
                            Layout.fillWidth: true; height: 30; enabled: mainRectangle.secEnabled
                            model: ["Auto", "AES-128-GCM", "AES-256-GCM"]; currentIndex: 0
                            background: Rectangle { radius: 3; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1 }
                            contentItem: Text { leftPadding: 8; text: encryptionAlgoCombo.displayText; verticalAlignment: Text.AlignVCenter; color: mainRectangle.secEnabled ? "black" : "gray" }
                            delegate: ItemDelegate {
                                width: encryptionAlgoCombo.width
                                contentItem: Text { text: modelData; color: "black"; verticalAlignment: Text.AlignVCenter; leftPadding: 8 }
                                highlighted: encryptionAlgoCombo.highlightedIndex === index
                                background: Rectangle { color: highlighted ? "#DA3450" : "white" }
                            }
                        }
                    }

                    // Key Revision Max History Depth
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        CheckBox {
                            id: keyRevisionCheck
                            checked: false; enabled: mainRectangle.secEnabled
                            indicator: Rectangle {
                                implicitWidth: 16; implicitHeight: 16
                                x: keyRevisionCheck.leftPadding; y: parent.height / 2 - height / 2
                                border.color: !mainRectangle.secEnabled ? "lightgray" : (keyRevisionCheck.checked ? "#DA3450" : "#34495e")
                                border.width: 1; radius: 2
                                color: !mainRectangle.secEnabled ? "#F0F0F0" : (keyRevisionCheck.checked ? "#DA3450" : "white")
                                Text { anchors.centerIn: parent; text: keyRevisionCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                            }
                            contentItem: Text {
                                text: "Key Revision Max History Depth:"
                                leftPadding: keyRevisionCheck.indicator.width + keyRevisionCheck.spacing + 4
                                verticalAlignment: Text.AlignVCenter; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12
                            }
                        }
                        TextField {
                            id: keyRevisionDepthField
                            Layout.fillWidth: true; height: 30; text: "8"; leftPadding: 8
                            enabled: mainRectangle.secEnabled && keyRevisionCheck.checked
                            verticalAlignment: TextInput.AlignVCenter
                            validator: IntValidator { bottom: 0; top: 9999 }
                            background: Rectangle {
                                border.color: (mainRectangle.secEnabled && keyRevisionCheck.checked) ? "darkgray" : "lightgray"
                                border.width: 1; color: (mainRectangle.secEnabled && keyRevisionCheck.checked) ? "white" : "#F0F0F0"; radius: 2
                            }
                        }
                        Rectangle {
                            width: 28; height: 30; color: "transparent"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 16; color: mainRectangle.secEnabled ? "black" : "gray" }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled && keyRevisionCheck.checked; onClicked: { var v = parseInt(keyRevisionDepthField.text)||0; keyRevisionDepthField.text = Math.max(v-1,0).toString() } }
                        }
                        Rectangle {
                            width: 28; height: 30; color: "transparent"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 16; color: mainRectangle.secEnabled ? "black" : "gray" }
                            MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled && keyRevisionCheck.checked; onClicked: { var v = parseInt(keyRevisionDepthField.text)||0; keyRevisionDepthField.text = (v+1).toString() } }
                        }
                    }

                    // Pre-shared secret passphrase block
                    Rectangle {
                        Layout.fillWidth: true
                        height: preSharedCol.implicitHeight + 20
                        border.color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0"; border.width: 1
                        color: mainRectangle.secEnabled ? "white" : "#F8F8F8"; radius: 2

                        ColumnLayout {
                            id: preSharedCol
                            anchors.fill: parent; anchors.margins: 10; spacing: 8

                            Rectangle {
                                Layout.fillWidth: true; height: 50
                                border.color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0"; border.width: 1
                                color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; radius: 2
                            }

                            RowLayout {
                                Layout.fillWidth: true; spacing: 8
                                CheckBox {
                                    id: preSharedCheck
                                    checked: false; enabled: mainRectangle.secEnabled
                                    indicator: Rectangle {
                                        implicitWidth: 16; implicitHeight: 16
                                        x: preSharedCheck.leftPadding; y: parent.height / 2 - height / 2
                                        border.color: !mainRectangle.secEnabled ? "lightgray" : (preSharedCheck.checked ? "#DA3450" : "#34495e")
                                        border.width: 1; radius: 2
                                        color: !mainRectangle.secEnabled ? "#F0F0F0" : (preSharedCheck.checked ? "#DA3450" : "white")
                                        Text { anchors.centerIn: parent; text: preSharedCheck.checked ? "✓" : ""; color: "white"; font.bold: true; font.pixelSize: 11 }
                                    }
                                    contentItem: Text {
                                        text: "Pre-shared secret passphrase:"
                                        leftPadding: preSharedCheck.indicator.width + preSharedCheck.spacing + 4
                                        verticalAlignment: Text.AlignVCenter; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12
                                    }
                                }
                                Text { text: "ID:"; color: mainRectangle.secEnabled ? "black" : "gray"; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                                TextField {
                                    id: preSharedIdField
                                    width: 60; height: 30; text: "0"; leftPadding: 8
                                    enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                                    validator: IntValidator { bottom: 0 }
                                    background: Rectangle { border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; radius: 2 }
                                }
                                Rectangle {
                                    width: 24; height: 30; color: "transparent"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 14; color: mainRectangle.secEnabled ? "black" : "gray" }
                                    MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { var v = parseInt(preSharedIdField.text)||0; preSharedIdField.text = Math.max(v-1,0).toString() } }
                                }
                                Rectangle {
                                    width: 24; height: 30; color: "transparent"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1
                                    Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 14; color: mainRectangle.secEnabled ? "black" : "gray" }
                                    MouseArea { anchors.fill: parent; enabled: mainRectangle.secEnabled; onClicked: { var v = parseInt(preSharedIdField.text)||0; preSharedIdField.text = (v+1).toString() } }
                                }
                                Text { text: "Seed:"; color: mainRectangle.secEnabled ? "black" : "gray"; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                                TextField {
                                    id: preSharedSeedField
                                    Layout.fillWidth: true; height: 30; leftPadding: 8
                                    enabled: mainRectangle.secEnabled; verticalAlignment: TextInput.AlignVCenter
                                    background: Rectangle { border.color: mainRectangle.secEnabled ? (preSharedSeedField.activeFocus ? "#DA3450" : "darkgray") : "lightgray"; border.width: 1; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; radius: 2 }
                                }
                            }

                            RowLayout { spacing: 6
                                Text { text: "Property Value:"; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12 }
                                Text { text: "data:,0:"; color: "gray"; font.pixelSize: 12 }
                            }
                        }
                    }

                    // ── Divider: Logging ───────────────────────────────
                    RowLayout { Layout.fillWidth: true; spacing: 8
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                        Text { text: "Logging"; color: mainRectangle.secEnabled ? "black" : "gray"; font.pixelSize: 12 }
                        Rectangle { height: 1; Layout.fillWidth: true; color: mainRectangle.secEnabled ? "lightgray" : "#E0E0E0" }
                    }

                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "Security Logging Verbosity (local):"; color: mainRectangle.secEnabled ? "black" : "gray"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        ComboBox {
                            id: securityLoggingCombo
                            Layout.fillWidth: true; height: 30; enabled: mainRectangle.secEnabled
                            model: ["Warning", "Debug", "Info", "Error", "Silent"]; currentIndex: 0
                            background: Rectangle { radius: 3; color: mainRectangle.secEnabled ? "white" : "#F0F0F0"; border.color: mainRectangle.secEnabled ? "darkgray" : "lightgray"; border.width: 1 }
                            contentItem: Text { leftPadding: 8; text: securityLoggingCombo.displayText; verticalAlignment: Text.AlignVCenter; color: mainRectangle.secEnabled ? "black" : "gray" }
                            delegate: ItemDelegate {
                                width: securityLoggingCombo.width
                                contentItem: Text { text: modelData; color: "black"; verticalAlignment: Text.AlignVCenter; leftPadding: 8 }
                                highlighted: securityLoggingCombo.highlightedIndex === index
                                background: Rectangle { color: highlighted ? "#DA3450" : "white" }
                            }
                        }
                    }

                    // ── Divider: Need help? ────────────────────────────
                    RowLayout { Layout.fillWidth: true; spacing: 8
                        Rectangle { height: 1; Layout.fillWidth: true; color: "lightgray" }
                        Text { text: "Need help?"; color: "black"; font.pixelSize: 12 }
                        Rectangle { height: 1; Layout.fillWidth: true; color: "lightgray" }
                    }

                    Text { Layout.fillWidth: true; text: "For more information about how to configure Admin Console to use Security, please check the following links"; color: "black"; wrapMode: Text.WordWrap; font.pixelSize: 12 }

                    Text {
                        text: "DDS Security Data Visualization with RTI Administration Console"
                        color: "#E67E22"; font.pixelSize: 12; font.underline: true
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Qt.openUrlExternally("https://community.rti.com") }
                    }

                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Text { text: "RTI Shapes Demo Security Profiles:"; color: "black"; Layout.preferredWidth: 200; verticalAlignment: Text.AlignVCenter; font.pixelSize: 12 }
                        ComboBox {
                            id: shapesSecurityCombo
                            Layout.fillWidth: true; height: 30
                            model: ["-", "Profile 1", "Profile 2"]; currentIndex: 0
                            background: Rectangle { radius: 3; color: "white"; border.color: "darkgray"; border.width: 1 }
                            contentItem: Text { leftPadding: 8; text: shapesSecurityCombo.displayText; verticalAlignment: Text.AlignVCenter; color: "black" }
                            delegate: ItemDelegate {
                                width: shapesSecurityCombo.width
                                contentItem: Text { text: modelData; color: "black"; verticalAlignment: Text.AlignVCenter; leftPadding: 8 }
                                highlighted: shapesSecurityCombo.highlightedIndex === index
                                background: Rectangle { color: highlighted ? "#DA3450" : "white" }
                            }
                        }
                    }

                    Item { height: 10 }
                }
            }
        }
    }
}