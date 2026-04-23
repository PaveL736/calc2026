// CustomButton.qml
import QtQuick 2.15

Item {
    id: root
    property alias text: label.text
    property bool pressed: false
    property bool hovered: false
    property color normalBackground: "#B0D1D8"
    property color hoverBackground: "#04BFAD"
    property color specialHoverBackground: "#F7E425"
    property color deleteHoverBackground: "#F25E5E"
    property color pressBackground: "#04BFAD"
    property color textColor: "#024873"
    property color hoverTextColor: "#FFFFFF"

    signal clicked()
    signal longPressStarted()
    signal longPressFinished()

    property bool secretArmed: false // передача флага

    // размер кнопок
    width: 60; height: 60

    // форма кнопок
    Rectangle {
        id: buttonForm
        anchors.fill: parent
        radius: Math.min(width, height) / 2
        color: pressed ? root.pressBackground
                     : hovered ? (isSpecial(text) ? root.specialHoverBackground
                                                  : text === "C" ? root.deleteHoverBackground
                                                                : root.hoverBackground)
                     : root.normalBackground
    }

    // названия кнопок
    Text {
        id: label
        anchors.centerIn: buttonForm
        lineHeight: isBracket(text) ? 1.15 : 1.0
        font.family: "Open Sans Semibold"
        font.weight: isOperator(text) ? Font.Light : Font.Bold
        font.pointSize: 24
        color: isSpecial(text) ? "transparent" : (root.hovered ? root.hoverTextColor : root.textColor)
    }

    // настройка толщины и размера шрифта
    function isOperator(txt) {
        return txt === "\u00F7" || txt === "×" ||
               txt === "\u2212" || txt === "+" || txt === "=";
    }

    // выравнивание скобок
    function isBracket(txt) { return txt === "()"; }

    // настройка цвета кнопок действий
    function isSpecial(txt) {
        return txt === "+" || txt === "\u207A\u2215\u208B" ||
               txt === "×" || txt === "\u00F7" || txt === "=" ||
               txt === "%" || txt === "\u2212" || txt === "()";
    }

    // зона реакции на кнопку
    MouseArea {
        id: ma
        anchors.fill: parent
        hoverEnabled: true

        property bool longPressActive: false
        property bool shortDone: false

        onEntered: root.hovered = true
        onExited:  root.hovered = false

        // кнопка нажата
        onPressed: {
            root.pressed = true
            shortDone = false
            if (text === "=") {
                longPressActive = false
                longPressTimer.start()
            }
        }

        // кнопка отжата
        onReleased: {
            root.pressed = false
            longPressTimer.stop()
            if (text === "=") {
                if (longPressActive) {
                    longPressActive = false
                    root.longPressFinished()
                }
                else if (!shortDone) {
                    shortDone = true
                    root.clicked()
                }
            }
            else {
                root.clicked()
            }
        }

        // исключение паразитных нажатий
        Timer {
            id: longPressTimer
            interval: 150
            onTriggered: {
                ma.longPressActive = true
                root.longPressStarted()
            }
        }
    }
}

