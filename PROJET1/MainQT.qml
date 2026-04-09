import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 800
    height: 600
    visible: true
    title: qsTr("Gestion Centrale Hydraulique")

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width - 40
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 5

            // --- Section Paramètres Généraux ---
            GroupBox {
                title: "Paramètres Généraux"
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    Layout.fillWidth: true

                    Label { text: "Débit total (m³/s) :" }
                    TextField {
                        id: totalFlow
                        placeholderText: "0.0"
                        validator: DoubleValidator { bottom: 0 }
                    }

                    Label { text: "Hauteur en amont (m) :" }
                    TextField {
                        id: headHeight
                        placeholderText: "0.0"
                        validator: DoubleValidator { bottom: 0 }
                    }
                }
            }

            // --- Section Turbines ---
            Label {
                text: "Configuration des Turbines"
                font.bold: true
                font.pointSize: 14
            }

            // On utilise un Repeater pour générer les 5 turbines
            Repeater {
                model: myModel // Liaison avec TurbineManager

                delegate: Frame {
                    Layout.fillWidth: true

                    // On utilise un background personnalisé pour le contraste
                    background: Rectangle {
                        color: "white"
                        radius: 5
                        border.color: "#dcdcdc"
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 20

                        // Nom de la turbine (fourni par NameRole)
                        Label {
                            text: model.name
                            font.bold: true
                            Layout.preferredWidth: 80
                        }

                        // État Active/Inactive
                        CheckBox {
                            id: activeCheck
                            text: "Active"
                            checked: model.active // Lit ActiveRole
                            onToggled: model.active = checked // Appelle setData(ActiveRole)
                        }

                        // Champ Limite Minimale
                        RowLayout {
                            Label { text: "Min:" }
                            TextField {
                                text: model.min // Lit MinRole
                                placeholderText: "0"
                                enabled: activeCheck.checked
                                Layout.preferredWidth: 60

                                // Validation : n'accepte que des nombres
                                validator: DoubleValidator { bottom: 0; top: 1000 }

                                // On attend que l'utilisateur finisse de taper pour mettre à jour
                                onEditingFinished: model.min = parseFloat(text)
                            }
                        }

                        // Champ Limite Maximale
                        RowLayout {
                            Label { text: "Max:" }
                            TextField {
                                text: model.max // Lit MaxRole
                                placeholderText: "160"
                                enabled: activeCheck.checked
                                Layout.preferredWidth: 60

                                validator: DoubleValidator { bottom: 0; top: 1000 }

                                onEditingFinished: model.max = parseFloat(text)
                            }
                        }
                    }
                }
            }

            Button {
                text: "Calculer"
                // Forçage local du contraste si la palette globale ne suffit pas
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 14
                    font.bold: true
                    color: "white" // Contraste maximal
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.down ? "#1a252f" : (parent.hovered ? "#34495e" : "#2c3e50")
                    radius: 4
                }

                onClicked: {
                    console.log("Calcul en cours...");
                    let d = parseFloat(totalFlow.text);
                    let h = parseFloat(headHeight.text);

                    if (isNaN(d) || isNaN(h)) {
                        console.error("Erreur : Valeurs non numériques !");
                        return;
                    }

                    console.log("Appel C++ avec :", d, h);
                    // On récupère les valeurs des TextField par leur ID
                    // et on appelle la fonction C++
                    myModel.calculerSynthese(d,h)
                }
            }
        }
    }
}