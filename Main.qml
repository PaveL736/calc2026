//Main.qml
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Calc 1.0
import "qrc:/buttons/" as Buttons
import "qrc:/menu/" as Menu
import "qrc:/code/" as Logic

Window {
    id: mainWindow
    width: 415; height: 640
    visible: true
    flags: Qt.FramelessWindowHint

    Calculator { id: calcEngine }


    Component.onCompleted: {
        calcController.handleKey("0"); // "0" на дисплее
    }

    FontLoader { id: openSans; source: "qrc:/fonts/OpenSans.ttf" }

    property bool isSecretArmed: secretCodeLogic.armed

    Logic.SecretCodeLogic { id: secretCodeLogic }
    Menu.SecretMenu { id: secretMenu }

    Connections {
        target: secretCodeLogic
        function onShowSecretMenu() { secretMenu.openMenu() }
    }

    QtObject {
        id: calcController
        property bool isOpenBracketInserted: false

        function handleParentheses() {
            if (isOpenBracketInserted) {
                handleKey(")");
                isOpenBracketInserted = false;
            }
            else {
                handleKey("(");
                isOpenBracketInserted = true;
            }
        }

        function handleKey(text) {
            if (text === "C") {
                expressionText.text = ""
                resultText.text  = "0"
                console.log("display-cleared")
                return
            }

            if (text === "=") {
                if (resultText.text === "0") return      // нечего считать
                try {
                    // выводим в консоль тот результат, который отправляем в движок
                    console.log("TOKENIZE: " + resultText.text)
                    const res = calcEngine.calculate(resultText.text)
                    expressionText.text = resultText.text
                    resultText.text     = res.toString()
                } catch (e) {
                    resultText.text = "Ошибка"
                    expressionText.text = ""
                }
                console.log("Result: " + resultText.text)
                return
            }

            // сброс после результата – теперь выполняется только
            // если НЕ нажали «=» и НЕ нажали "C"
            if (expressionText.text !== "") {
                expressionText.text = ""
                resultText.text     = text
                return
            }

            // обычный ввод
            if (resultText.text === "0" && text !== ".") {
                resultText.text = text
            } else {
                resultText.text += text
            }

            const symbolMap = { "\u207A\u2215\u208B":"+/-",
                                "\u00F7":"/",
                                "\u2212":"-",
                                "×":"*"
            }
            const changeText = symbolMap[text] || text
            console.log("on display:", changeText)

            // секретный код
            secretCodeLogic.handleKeyPress(text)
        }

    }

    // перетаскивание окна
    MouseArea {
        id: windowDragArea
        anchors.fill: parent
        cursorShape: Qt.OpenHandCursor

        property point pressWindowPos: Qt.point(0, 0)   // позиция окна при нажатии
        property point pressMousePos: Qt.point(0, 0)    // позиция мыши внутри окна при нажатии

        onPressed: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
                pressWindowPos = Qt.point(mainWindow.x, mainWindow.y)
                pressMousePos = Qt.point(mouse.x, mouse.y)
            }
        }

        onPositionChanged: (mouse) => {
            if (!(mouse.buttons & Qt.LeftButton)) return

            // текущая позиция мыши на экране = позиция окна + позиция мыши внутри окна
            var currentScreenX = pressWindowPos.x + (mouse.x - pressMousePos.x)
            var currentScreenY = pressWindowPos.y + (mouse.y - pressMousePos.y)

            // устанавливаем новую позицию окна
            mainWindow.x = currentScreenX
            mainWindow.y = currentScreenY
        }

        onReleased: {
            pressWindowPos = Qt.point(0, 0)
            pressMousePos = Qt.point(0, 0)
        }
    }

    // подложка
    Rectangle { anchors.fill: parent; color: "#024873" }

    // верхняя панель
    Rectangle {
        width: parent.width
        height: 200 + 24
        anchors.top: parent.top
        anchors.topMargin: -24
        color: "#04BFAD"
        //color: "#111111" // тест
        radius: 24
    }

    // колонка с дисплеем и кнопками
    Column {
        // привязываем колонку к верхней части окна
        anchors.top: parent.top  // начало сверху
        anchors.horizontalCenter: parent.horizontalCenter
        topPadding: 50  // опускаем дисплей ниже
        spacing: 15  // интервал между дисплеем и кнопками

        // дисплей
        Rectangle {
            id: display
            width: 350
            height: 150
            radius: 16
            color: "#04BFAD"

            // верхняя строка - выражение: (мелкий шрифт)
            Text {
                id: expressionText
                anchors {
                    left:   parent.left
                    right:  parent.right
                    top:    parent.top          // привязали к верху
                    bottom: undefined          // не трогаем центр
                    margins: 5
                }
                height: parent.height * 0.25   // 25 % высоты дисплея
                font { family: "Open Sans Semibold"; pointSize: 20 }
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignRight
                verticalAlignment:   Text.AlignBottom
                clip: true
                elide: Text.ElideRight
            }

            // нижняя строка - результат: (крупный шрифт)
            Text {
                id: resultText
                anchors {
                    left: parent.left;
                    right: parent.right;
                    bottom: parent.bottom;
                    margins: 5
                }
                height: parent.height * 0.65
                font { family: "Open Sans Semibold"; pointSize: 60 }
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignBottom
                clip: true

                function fitText() {
                    const maxWidth = parent.width - anchors.margins * 2;
                    font.pointSize = 60; // сброс к базовому
                    fitTimer.restart(); // даём тик, пока движок обновит contentWidth
                }

                Timer {   // одноразовый таймер на 0 мс – сразу после текущего события
                    id: fitTimer
                    interval: 0
                    onTriggered: {
                        const minSize = 10;
                        while (resultText.contentWidth > maxWidth &&
                               resultText.font.pointSize > minSize) {
                            resultText.font.pointSize -= 1;
                        }
                    }
                    property real maxWidth: parent.width - parent.anchors.margins * 2
                }

                Component.onCompleted: fitText()
                onTextChanged:  fitText()
            }

        }

        // сетка с кнопками
        GridLayout {
            id: calcGrid
            columns: 4
            columnSpacing: 40
            rowSpacing: 25



            Buttons.CustomButton { id: btnPar;   text: "()";  normalBackground: "#0889A6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleParentheses()
            }
            Buttons.CustomButton { id: btnSign;  text: "\u207A\u2215\u208B"; normalBackground: "#0889A6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btnPct;   text: "%";   normalBackground: "#0889A6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btnDiv;   text: "\u00F7"; normalBackground: "#0889A6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }



            Buttons.CustomButton { id: btn7;     text: "7";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btn8;     text: "8";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btn9;     text: "9";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btnMul;   text: "×";   normalBackground: "#0889A6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }



            Buttons.CustomButton { id: btn4;     text: "4";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btn5;     text: "5";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btn6;     text: "6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btnSub;   text: "\u2212"; normalBackground: "#0889A6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }



            Buttons.CustomButton { id: btn1;     text: "1";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btn2;     text: "2";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btn3;     text: "3";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btnAdd;   text: "+";   normalBackground: "#0889A6";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }



            Buttons.CustomButton { id: btnClr;   text: "C";   normalBackground: "#FC7C7C";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btn0;     text: "0";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton { id: btnDot;   text: ".";
                secretArmed: mainWindow.isSecretArmed; onClicked: calcController.handleKey(text)
            }
            Buttons.CustomButton {
                id: btnEq
                text: "="
                normalBackground: "#0889A6"
                secretArmed: mainWindow.isSecretArmed

                onClicked: calcController.handleKey("=")

                onLongPressStarted: secretCodeLogic.handleLongPress()
                onLongPressFinished: secretCodeLogic.handleRelease()
            }
        }
    }
}
