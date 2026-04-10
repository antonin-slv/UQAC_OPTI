#include <QDirIterator>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlregistration.h>
#include <QQmlContext>

#include "src/centraleQTWrap.h"
#include "src/SolverManagerQt.h"
//#include "PROJET1/src/SolverNomadQTWrapper.h"

#include "SolverNomadQTWrapper.h"

int main(int argc, char *argv[])
{
    // Indispensable pour la haute densité de pixels (écrans 4K)
    QGuiApplication app(argc, argv);

    // 1. Instanciation du Backend
    // On le crée sur la pile (stack) ; il sera détruit proprement à la fin du main.

    //NomadSolverWrapper nomadSolver;
    SolverManager solverManager(nullptr);

    NomadSolverWrapper NomadSolverWrapper(&solverManager);

    Centrale power_centrale;
    power_centrale.load5DefaultTurb();
    power_centrale.H_amont = 135.0f;
    power_centrale.solver = NomadSolverWrapper.getSolver();
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << "Fichier QRC trouvé :" << it.next();
    }

    CentraleQTWrapper manager(nullptr, &power_centrale);
    manager.slvmanag = &solverManager;


    // 2. Configuration du moteur QML
    QQmlApplicationEngine engine;

    // 3. Injection du Manager
    // Nous rendons l'instance 'manager' accessible sous le nom 'myModel' dans le monde QML.
    // On passe un pointeur (&manager).
    engine.rootContext()->setContextProperty("myModel", &manager);
    engine.rootContext()->setContextProperty("solverManager", &solverManager);

    qmlRegisterUncreatableType<SolverWrapper>("CentraleLib", 1, 0, "SolverWrapper", "Interface");
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

