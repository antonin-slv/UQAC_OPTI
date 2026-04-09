#pragma once

#include <QObject>

#include "DPResourceAllocationFast.cpp"
// SolverWrapper.h

class SolverWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString settingsComponent READ settingsComponent CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    virtual ResourceAllocationSolver* getSolver() = 0;
    virtual QString settingsComponent() const = 0; // Ex: "DPSettings.qml"
    virtual QString name() const = 0;
};

// Exemple pour la DP
class DPSolverWrapper : public SolverWrapper
{
    Q_OBJECT
    Q_PROPERTY(double step READ step WRITE setStep NOTIFY stepChanged)

public:
    explicit DPSolverWrapper(QObject* parent = nullptr) : m_solver(5.0f) // Valeur par défaut du pas, peut être modifiée via l'UI
    { }

    ResourceAllocationSolver* getSolver() override { return &m_solver; }
    QString settingsComponent() const override { return "ui/DPSettings.qml"; }
    QString name() const override { return "Programmation Dynamique"; }

    double step() const { return m_solver.get_step(); }

    void setStep(double s)
    {
        m_solver.set_step(s);
        emit stepChanged();
    }

private:
    DPResourceAllocationFast m_solver; // Ta classe héritant de ResourceAllocationSolver
signals:
    void stepChanged();
};
