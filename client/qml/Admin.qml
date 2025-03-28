import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    property bool editing: false

    // Gradient Background
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#6DD5FA" }
        GradientStop { position: 1.0; color: "#2980B9" }
    }

    // Overlay to soften gradient
    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"
        opacity: 0.15
    }

    Column {
        anchors.fill: parent
        spacing: 10
        padding: 20

        // ===== Add User Section =====
        Rectangle {
            color: "#B0BEC5"
            radius: 12
            height: 180
            width: parent.width
            Row {
                anchors.centerIn: parent
                spacing: 20

                Column {
                    spacing: 10
                    TextField {
                        id: usernameField
                        placeholderText: "Username"
                        width: 200
                        height: 50
                        font.pointSize: 15
                    }
                    TextField {
                        id: passwordField
                        placeholderText: "Password (10 digits)"
                        validator: RegularExpressionValidator { regularExpression: /[0-9]{10}/ }
                        width: 200
                        height: 50
                        font.pointSize: 15
                        echoMode: TextInput.Password
                    }
                    Text {
                            text: modelData.is_admin ? "Yes" : "No"
                            width: 100
                            font.pointSize: 14
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }
                }

                Column {
                    spacing: 10
                    Image {
                        source: "qrc:/images/images/adduser_icon.png"
                        width: 80
                        height: 80
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (clientAPI.createUser(usernameField.text, passwordField.text, false)) {
                                    statusLabel.text = "User added ✅"
                                    statusLabel.color = "green"
                                    usernameField.text = ""
                                    passwordField.text = ""
                                    userList.model = clientAPI.getAllUsers()
                                } else {
                                    statusLabel.text = "Add failed ❌"
                                    statusLabel.color = "red"
                                }
                            }
                        }
                    }
                    // Button {
                    //     text: "Add"
                    //     width: 100
                    //     height: 50
                    //     onClicked: {
                    //         if (clientAPI.createUser(usernameField.text, passwordField.text, false)) {
                    //             statusLabel.text = "User added ✅"
                    //             statusLabel.color = "green"
                    //             usernameField.text = ""
                    //             passwordField.text = ""
                    //             userList.model = clientAPI.getAllUsers()
                    //         } else {
                    //             statusLabel.text = "Add failed ❌"
                    //             statusLabel.color = "red"
                    //         }
                    //     }
                    // }
                }
            }
        }

        // Status message
        Text {
            id: statusLabel
            text: ""
            color: "white"
            font.bold: true
            font.pointSize: 14
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // ===== User List Section =====
        Rectangle {
            color: "#55454545"
            radius: 12
            width: parent.width
            height: parent.height - 250

            Column {
                anchors.fill: parent
                spacing: 5
                padding: 10

                // Header
                Row {
                    spacing: 50
                    Text { text: "Username"; width: 150; font.bold: true; font.pointSize: 16 }
                    Text { text: "Password"; width: 150; font.bold: true; font.pointSize: 16 }
                    Text { text: "Is Admin"; width: 100; font.bold: true; font.pointSize: 16 }
                    Text { text: "Actions"; width: 200; font.bold: true; font.pointSize: 16 }
                }

                ListView {
                    id: userList
                    width: parent.width
                    height: parent.height - 60
                    model: clientAPI.getAllUsers()
                    spacing: 5

                    delegate:  Rectangle {
                        width: parent.width
                        height: 50
                        color: "transparent"
                        property bool editing: false

                        Row {
                            spacing: 50
                            anchors.fill: parent
                            anchors.margins: 10

                            TextField {
                                id: usernameEditor
                                text: modelData.username
                                width: 150
                                font.pointSize: 14
                                readOnly: !editing || modelData.username === "admin"
                            }

                            TextField {
                                id: passwordEditor
                                text: ""
                                placeholderText: "New Password"
                                echoMode: TextInput.Password
                                width: 150
                                font.pointSize: 14
                                readOnly: !editing || modelData.username === "admin"
                            }
                            Text {
                                    text: modelData.is_admin ? "Yes" : "No"
                                    width: 100
                                    font.pointSize: 14
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                }

                            Row {
                                spacing: 10
                                visible: modelData.username !== "admin"

                                Image {
                                    source: editing ? "qrc:/images/images/save_icon.png" : "qrc:/images/images/edit_icon.png"
                                    width: 30
                                    height: 30
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            if (!editing) {
                                                editing = true
                                            } else {
                                                const success = clientAPI.updateUser(
                                                                  modelData.username,
                                                                  usernameEditor.text,
                                                                  passwordEditor.text
                                                                  )
                                                if (success) {
                                                    editing = false
                                                    statusLabel.text = "Updated ✅"
                                                    statusLabel.color = "green"
                                                    userList.model = clientAPI.getAllUsers()
                                                } else {
                                                    statusLabel.text = "Update failed ❌"
                                                    statusLabel.color = "red"
                                                }
                                            }
                                        }
                                    }
                                }

                                Image {
                                    source: "qrc:/images/images/delete_icon.png"
                                    width: 30
                                    height: 30
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            if (clientAPI.deleteUser(modelData.username)) {
                                                statusLabel.text = "User deleted ✅"
                                                statusLabel.color = "green"
                                                userList.model = clientAPI.getAllUsers()
                                            } else {
                                                statusLabel.text = "Delete failed ❌"
                                                statusLabel.color = "red"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                }
            }
        }
    }
}
