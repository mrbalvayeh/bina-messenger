import QtQuick
import QtQuick.Window
import QtQuick.Controls
Window {
    id: root
    width: 1920
    height: 1080
    visible: true
    title: qsTr("Bina Messenger")
    property string mainState: "login"

    Login {
        id: loginPage
        anchors.fill: parent
        visible: mainState === "login"
        onLoginSuccess: {
            mainState = "admin"
        }
    }

    Admin {
        id: adminPage
        visible: mainState === "admin"
    }


}
