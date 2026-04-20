import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Popup {
    FontLoader { id: openSans; source: "qrc:/fonts/OpenSans.ttf" }

    id: popup

    width: 300
    height: 400
    modal: true                 // блокирует фон
    focus: true                 // можно управлять кнопкой
    anchors.centerIn: Overlay.overlay   // центрируем поверх всего окна

    // подложка
    background: Rectangle {
        color: "#04BFAD"
        radius: 24
    }

    // колонка текста и кнопки
    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Секретное меню"
            font.family: "Open Sans Semibold"
            font.pixelSize: 28
            color: "white"
        }

        // кнопка
        RowLayout {
            width: 120
            height: 40

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 12
                color: "#04BFAD"
                border.color: "#FFFFFF"
                border.width: 1.5

                // зона нажатия
                Rectangle {
                    //отступы
                    x: 5
                    y: 5
                    width: parent.width - 10  // Ширина с учётом отступов
                    height: parent.height - 10  // Высота с учётом отступов
                    radius: 10
                    color: "transparent"
                    clip: true  // Обрезаем содержимое по границе

                    //название кнопки
                    Text {
                        anchors.centerIn: parent
                        text: "Назад"
                        font.family: "Open Sans Semibold"
                        color: "#FFFFFF"
                        font.bold: true
                        font.weight: Font.ExtraBold
                    }

                    // MouseArea ограничен внутренней зоной
                    MouseArea {
                        anchors.fill: parent
                        onClicked: popup.close()
                    }
                }
            }
        }
    }

    // публичный слот, вызов из Main
    function openMenu() { popup.open() }
}
