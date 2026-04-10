import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

ColumnLayout {
    property var solver // Injecté par le Loader
    spacing: 12
    width: parent.width

    // --- Budget d'itérations ---
    RowLayout {
        Label { text: "Budget d'itérations :"; Layout.fillWidth: true }
        Slider {
            id: evalSlider
            from: 50; to: 5000; stepSize: 50
            value: solver ? solver.maxEvalCount : 1000
            onMoved: if(solver) solver.maxEvalCount = value
        }
        Label {
            text: evalSlider.value.toString()
            font.bold: true
            Layout.preferredWidth: 40
        }
    }

    // --- Type de Barrière ---
    RowLayout {
        spacing: 10
        Label { text: "Type de barrière :"; Layout.preferredWidth: 150 }
        ComboBox {
            id: barrierCombo
            model: ["EB (Strict)", "PB (Souple)"]
            currentIndex: solver && solver.useXtrmBarrier ? 0 : 1
            Layout.fillWidth: true
            onActivated: (index) => {
                if(solver) solver.useXtrmBarrier = (index === 0)
            }
        }
    }

    // --- Précision d'arrêt (Saisie manuelle) ---
    RowLayout {
        spacing: 10
        Label { text: "Précision d'arrêt (m³/s) :"; Layout.fillWidth: true }
        TextField {
            id: precisionInput
            placeholderText: "0.001"
            Layout.preferredWidth: 100
            // On affiche la valeur actuelle du solver
            text: solver ? solver.minMeshSize.toString() : "0.001"

            validator: DoubleValidator {
                bottom: 0.000001
                top: 1.0
                decimals: 7
                locale: "C" // Force le point décimal
            }

            onEditingFinished: {
                if(solver) solver.minMeshSize = Number(text)
            }
        }
    }

    // --- Pas de recherche initial (Saisie manuelle) ---
    RowLayout {
        spacing: 10
        Label { text: "Pas de recherche initial :"; Layout.fillWidth: true }
        TextField {
            id: initialStepInput
            placeholderText: "1.0"
            Layout.preferredWidth: 100
            text: solver ? solver.initialMeshSize.toString() : "1.0"

            validator: DoubleValidator {
                bottom: 0.01
                top: 1000.0
                decimals: 3
                locale: "C"
            }

            onEditingFinished: {
                if(solver) solver.initialMeshSize = Number(text)
            }
        }
    }

    // Un petit espaceur en bas
    Item { Layout.fillHeight: true }
}