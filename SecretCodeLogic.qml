//SecretCodeLogic.qml
import QtQuick 2.15

Item {
    id: logic

    signal showSecretMenu()

    property var requiredSequence: ["1","2","3"]
    property bool armed: false

    property var sequence: []
    property bool started: false

    Timer {
        id: longPressTimer
        interval: 4000
        onTriggered: {
            console.log("ready – armed")
            logic.armed = true
        }
    }

    Timer {
        id: sequenceTimer
        interval: 5000
        onTriggered: {
            console.log("time is up – checking")
            checkAndFinish()
        }
    }

    function handleLongPress() { longPressTimer.start() }

    function handleRelease() { longPressTimer.stop() }

    function handleKeyPress(key) {
        if (!armed) return
        if (!started) {
            started = true
            sequence.push(key)
            console.log("sequence started:", key)
            sequenceTimer.start()
            return
        }
        sequence.push(key)
        console.log("added:", key)
    }

    function checkAndFinish() {
        const ok = (sequence.length === requiredSequence.length) &&
                   sequence.every((v,i)=>v===requiredSequence[i])
        if (ok) {
            console.log("correct – open menu")
            showSecretMenu()
            longPressTimer.restart() // рестарт таймера при следующем длинном нажатии.
        }
        else {
            console.log("wrong / timeout")
        }
        reset()
    }

    function reset() {
        armed  = false
        started = false
        sequence = []
        longPressTimer.stop()
        sequenceTimer.stop()
    }
}

