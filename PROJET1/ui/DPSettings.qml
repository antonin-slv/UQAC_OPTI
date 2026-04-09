import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

RowLayout {
    property var solver // Injecté par le Loader
    spacing: 10

    Label {
        text: "Pas de discrétisation :"
        font.pixelSize: 12
    }

    SpinBox {
        id: stepSpinBox
        // On travaille en "dixièmes" pour simuler le float (0.1 -> 1, 100.0 -> 1000)
        from: 1
        to: 10000
        stepSize: 1
        editable: true

        Layout.preferredWidth: 120

        // Liaison avec le solver : on multiplie par 10 pour l'affichage
        value: solver ? Math.round(solver.step * 100) : 100

        // Mise à jour du solver : on divise par 10
        onValueModified: {
            if (solver) solver.step = value / 100
        }

        // --- Logique d'affichage (Integer -> String "X.X") ---
        textFromValue: function(value, locale) {
            return Number(value / 100).toLocaleString(locale, 'f', 2);
        }

        // --- Logique de saisie (String "X.X" -> Integer) ---
        valueFromText: function(text, locale) {
            return Math.round(Number.fromLocaleString(locale, text) * 100);
        }

        // Validateur pour empêcher la saisie de n'importe quoi
        validator: DoubleValidator {
            bottom: 0.01
            top: 100.0
            decimals: 2
            notation: DoubleValidator.StandardNotation
        }
    }

    Label {
        text: "m³/s"
        font.italic: true
        color: "#666"
    }
}