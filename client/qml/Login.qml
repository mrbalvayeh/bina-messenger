import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    // width: 400
    // height: 550
    signal loginSuccess()

    // A light-to-dark blue gradient background
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#6DD5FA" }   // Light blue
        GradientStop { position: 1.0; color: "#2980B9" }   // Darker blue
    }

    // A semi-transparent overlay to soften the gradient, optional
    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"
        opacity: 0.15
    }

    // Main vertical layout
    Column {
        id: layoutColumn
        anchors.centerIn: parent
        spacing: 20
        height: 500
        width: 500
        // width: parent.width * 0.8


        // LOGO
        // Rectangle
        // {
        //     color: "Black"
        //     width: 300
        //     height: 100
        // }

        Image {
            id: logoImage
            source: "qrc:/images/images/Bina_Logo_rad.png"   // Place your logo file or resources path
            fillMode: Image.PreserveAspectFit
            // anchors.horizontalCenter: parent.horizontalCenter
            width: 500
            height: 200
            opacity: 0.8
            // width: parent.width * 0.4
            // height: parent.width * 0.4
        }

        // Input fields container
        Rectangle {
            id: inputContainer
            color: "#B0BEC5"
            // opacity: 0.8
            radius: 12
            width: parent.width
            height: 160
            anchors.horizontalCenter: parent.horizontalCenter
            Row{
                anchors.centerIn:  parent
                spacing: 15
                Column {
                    // anchors.centerIn: parent
                    // anchors.horizontalCenter:  parent.horizontalCenter
                    spacing: 13
                    // width: parent.width * 0.8

                    // USERNAME
                    TextField {
                        id: usernameField
                        placeholderText: "Username"
                        font.pointSize: 15
                        height: 50
                        width: 200
                        // You can style this further if needed
                    }
                    // PASSWORD
                    TextField {
                        id: passwordField
                        placeholderText: "Password"
                        echoMode: TextInput.Password
                        font.pointSize: 15
                        height: 50
                        width: 200
                    }
                }
                Column
                {
                    // anchors.verticalCenter: parent.verticalCenter
                    spacing: 10
                    // LOGIN BUTTON
                    Button
                    {
                        id: loginButton
                        text: "Login"
                        width: 100
                        height: 50
                        font.bold: true
                        font.pointSize:  10

                        // A bluish shade for the button
                        background: Rectangle {
                            radius: 8
                            color: "#3C8CE7"   // A mid-range blue
                        }
                        // Text color
                        // contentItem: Text {
                        //     text: loginButton.text
                        //     color: "white"
                        //     font.bold: true
                        //     anchors.horizontalCenter:  parent.horizontalCenter
                        // }

                        // Example of QML->C++ call:
                        onClicked: {
                            var authResult = clientAPI.authenticate(usernameField.text, passwordField.text)

                            console.log("Login result:", authResult)
                            console.log("Is admin:", authResult.isAdmin)
                            if (authResult.success) {
                                statusLabel.text = "Login Successful"

                                if (authResult.isAdmin) {
                                    loginSuccess() // ✅ Trigger it here
                                }
                            }
                            if (!authResult.success) {
                                statusLabel.text = authResult.error || "Invalid credentials"
                                return
                            }

                        }
                    }
                    Button
                    {
                        id:resetPassButton
                        text:"Forget Pass?"
                        width: 100
                        height: 50
                        font.bold: true
                        font.pointSize:  10
                        background: Rectangle {
                            radius: 8
                            color: "#FE4F2D"   // A mid-range blue
                        }
                    }

                }
            }
        }



        // STATUS LABEL
        Text {
            id: statusLabel
            text: ""
            color: "white"
            font.bold: true
            font.pointSize: 12
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            // Example usage: statusLabel.text = "Login failed, please try again."
        }
    }
}
