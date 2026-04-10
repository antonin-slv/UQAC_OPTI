#pragma once
#include <QList>
#include <QObject>

#include "SolverNomadQTWrapper.h"
#include "SolverQtWrappers.h"

class SolverManager : public QObject
{
    Q_OBJECT
    // Liste pour le ComboBox
    Q_PROPERTY(QList<QObject*> availableSolvers READ availableSolvers CONSTANT)
    // Solveur actuellement choisi
    Q_PROPERTY(SolverWrapper* currentSolver READ currentSolver WRITE setCurrentSolver NOTIFY currentSolverChanged)

public:
    explicit SolverManager(QObject* parent = nullptr);

    QList<QObject*> availableSolvers() const
    {
        QList<QObject*> list;
        for (auto* s : m_solvers) list.append(s);
        return list;
    }

    SolverWrapper* currentSolver() const { return m_current; }

    void addSolver(SolverWrapper* solver)
    {
        if (!m_solvers.contains(solver))
            m_solvers.append(solver);
    }

public slots:

    void setCurrentSolver(SolverWrapper* solver)
    {
        if (m_current != solver)
        {
            m_current = solver;
            emit currentSolverChanged();
        }
    }

    // Méthode utilitaire pour changer par index (pratique pour ComboBox)
    Q_INVOKABLE void setSolverIndex(int index)
    {
        if (index >= 0 && index < m_solvers.size())
        {
            setCurrentSolver(m_solvers[index]);
        }
    }

    signals:
        void currentSolverChanged();

private:
    QList<SolverWrapper*> m_solvers;
    SolverWrapper* m_current = nullptr;
};

inline SolverManager::SolverManager(QObject *parent) : QObject(parent) {
    // 1. Instanciation des solveurs disponibles
    auto* dp = new DPSolverWrapper(this);
    auto* nomad = new NomadSolverWrapper(this);

    m_solvers.append(dp);
    m_solvers.append(nomad);

    // 2. Sélection par défaut
    m_current = dp;
}