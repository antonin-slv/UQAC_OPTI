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

            RowLayout {
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
                            validator: DoubleValidator {
                                bottom: 0
                                decimals: 3 // On autorise jusqu'à 3 décimales
                                notation: DoubleValidator.StandardNotation
                                locale: "C"}
                        }

                        Label { text: "Hauteur en amont (m) :" }
                        TextField {
                            id: headHeight
                            placeholderText: "0.0"
                            validator: DoubleValidator { bottom: 0
                                decimals: 3 // On autorise jusqu'à 3 décimales
                                notation: DoubleValidator.StandardNotation
                                locale: "C"
                            }
                        }
                    }
                }
                GroupBox {
                    Layout.fillWidth: true
                    bottomPadding : 15
                    topPadding : 15

                    // On remplace le titre textuel par un composant complexe
                    label: RowLayout {
                        width: parent.width
                        spacing: 10
                        // On ajuste les marges pour que ça s'aligne bien avec le cadre
                        x: 10
                        y: -15

                        Label {
                            text: "Configuration via :"
                            font.bold: true
                            color: "#333"
                        }

                        ComboBox {
                            id: algoCombo
                            model: solverManager.availableSolvers
                            textRole: "name"
                            currentIndex: 0 // Par défaut sur le premier

                            // On réduit un peu la taille pour que ça rentre bien dans l'en-tête
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: 200

                            onCurrentIndexChanged: solverManager.setSolverIndex(currentIndex)

                            // Design plus discret pour l'en-tête
                            flat: true
                        }
                    }

                    // --- CONTENU DU GROUPE ---
                    Loader {

                        id: paramLoader
                        Layout.fillWidth: true
                        // On s'assure que le chemin est correct selon ton architecture module
                        source: solverManager.currentSolver ? solverManager.currentSolver.settingsComponent : ""

                        onLoaded: {
                            if (item) item.solver = solverManager.currentSolver
                        }
                    }
                }
            }


            // --- Section Turbines ---
            Label {
                text: "Configuration des Turbines"
                font.bold: true
                font.pointSize: 14
                color : "#333"
            }

            // On utilise un Repeater pour générer les 5 turbines
            Repeater {
                model: myModel // Liaison avec CentraleQTWrapper

                delegate: Frame {
                    Layout.fillWidth: true
                    padding: 0
                    bottomPadding : 15
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        color: "white"
                        radius: 5
                        border.color: "#dcdcdc"

                    }

                    RowLayout {
                        Layout.fillHeight: true
                        anchors.fill: parent
                        spacing: 0
                        // remove any top/bottom padding from the RowLayout
                        // to ensure the content is vertically centered
                        anchors.margins: 0


                        RowLayout {
                            // --- SECTION CONFIGURATION (ENTRÉES) ---
                            Label {
                                text: model.name
                                font.bold: true
                                font.pixelSize: 11 // Police plus petite
                                Layout.preferredWidth: 80
                            }

                            CheckBox {
                                id: activeCheck
                                text: "Active"
                                checked: model.active
                                scale: 0.9
                                onToggled: model.active = checked
                            }

                            // Groupement des bornes pour plus de clarté
                            RowLayout {
                                spacing: 5
                                enabled: activeCheck.checked
                                Label { text: "Min:"; font.pixelSize: 11; color: "gray" }
                                TextField {
                                    text: model.min
                                    Layout.preferredWidth: 50
                                    validator: DoubleValidator { bottom: 0; top: 1000 }
                                    onEditingFinished: model.min = parseFloat(text)
                                }
                                Label { text: "Max:"; font.pixelSize: 11; color: "gray" }
                                TextField {
                                    text: model.max
                                    Layout.preferredWidth: 50
                                    validator: DoubleValidator { bottom: 0; top: 1000 }
                                    onEditingFinished: model.max = parseFloat(text)
                                }
                            }
                        }


                        // Petit séparateur vertical visuel
                        Item { Layout.preferredWidth: 50 }

                        // --- SECTION RÉSULTATS OPTIMISÉS (SORTIES) ---
                        // On accède à opti_rslt via le nom du modèle global et l'index actuel
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 20
                            anchors.margins : 0

                            // Affichage du débit optimisé
                            ColumnLayout {
                                spacing: 0
                                Label { text: "Débit optimisé"; font.pixelSize: 7; color: "#666" }
                                Label {
                                    // Sécurité : on vérifie si la liste results est assez grande
                                    property var result: myModel.opti_rslt[index]
                                    text: result ? result.debit.toFixed(2) + " m³/s" : "--"
                                    font.bold: true
                                    Layout.preferredHeight: 10
                                    font.pixelSize: 13
                                    color: activeCheck.checked ? "#2196F3" :  "#999" // Bleu si actif, gris sinon
                                }
                            }

                            // Affichage de la puissance calculée
                            ColumnLayout {
                                spacing: 0
                                Label { text: "Puissance"; font.pixelSize: 7; color: "#666" }
                                Label {
                                    property var result: myModel.opti_rslt[index]
                                    text: result ? result.puissance.toFixed(2) + " MW" : "--"
                                    font.bold: true
                                    Layout.preferredHeight: 10
                                    font.pixelSize: 13
                                    color: activeCheck.checked ? "#4CAF50" : "#999" // Vert si actif, gris sinon
                                }
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true

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

                Item { Layout.fillWidth: true } // Espaceur flexible

                ColumnLayout {
                    spacing: 0
                    Label { text: "Débit consommé"; font.pixelSize: 10; color: "#666" }
                    Label {
                        // Sécurité : on vérifie si la liste results est assez grande
                        text:  {
                            if (myModel.opti_rslt.length <= 0) {
                                return "--";
                            }
                            let total = 0;

                            for(let i=0; i < myModel.opti_rslt.length; i++) {
                                total += myModel.opti_rslt[i].debit;
                            }
                            return total.toFixed(2) + " m³/s";
                        }
                        font.bold: true
                        Layout.preferredHeight: 10
                        font.pixelSize: 14
                        color:  "#2196F3"
                    }
                }

                // Affichage de la puissance calculée
                ColumnLayout {
                    spacing: 0
                    Label { text: "Puissance totale"; font.pixelSize: 10; color: "#666" }
                    Label {
                        property var result: myModel.opti_rslt.length > 0 ? myModel.opti_rslt[0] : null
                        text:  {
                            if (myModel.opti_rslt.length <= 0) {
                                return "--";
                            }
                            let total = 0;

                            for(let i=0; i < myModel.opti_rslt.length; i++) {
                                total += myModel.opti_rslt[i].puissance;
                            }
                            return total.toFixed(2) + " MW";
                        }
                        font.bold: true
                        Layout.preferredHeight: 10
                        font.pixelSize: 14
                        color: "#4CAF50"
                    }
                }

            }

        }
    }
}