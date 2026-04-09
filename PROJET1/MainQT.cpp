#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlregistration.h>
#include <QQmlContext>

#include "src/centraleQTWrap.h"
#include "src/DPResourceAllocationfast.cpp"

int main(int argc, char *argv[])
{
    // Indispensable pour la haute densité de pixels (écrans 4K)
    QGuiApplication app(argc, argv);

    // 1. Instanciation du Backend
    // On le crée sur la pile (stack) ; il sera détruit proprement à la fin du main.

    DPResourceAllocationFast solver = DPResourceAllocationFast(10.0f);

    Centrale power_centrale;
    power_centrale.load5DefaultTurb();
    power_centrale.H_amont = 135.0f;
    power_centrale.setSolver(&solver);
    CentraleQTWrapper manager(nullptr, &power_centrale);

    // 2. Configuration du moteur QML
    QQmlApplicationEngine engine;

    // 3. Injection du Manager
    // Nous rendons l'instance 'manager' accessible sous le nom 'myModel' dans le monde QML.
    // On passe un pointeur (&manager).
    engine.rootContext()->setContextProperty("myModel", &manager);
    // 4. Gestion de la sécurité au démarrage
    // On connecte le signal d'échec de création d'objet pour quitter proprement
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // 5. Chargement du module QML (Syntaxe Qt 6)
    engine.loadFromModule("UQAC_OPTI", "MainQT");

    // 6. Vérification critique
    // Si le moteur n'a chargé aucun objet racine (ex: erreur de syntaxe QML), on quitte.
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

