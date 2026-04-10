#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "SolverQtWrappers.h"
#include "NomadSolver.h"

class NomadSolverWrapper : public SolverWrapper
{
    Q_OBJECT
    QML_ELEMENT

    // Déclaration des propriétés pour le QML
    Q_PROPERTY(int maxEvalCount READ maxEvalCount WRITE setMaxEvalCount NOTIFY maxEvalCountChanged)
    Q_PROPERTY(bool useXtrmBarrier READ useXtrmBarrier WRITE setUseXtrmBarrier NOTIFY useXtrmBarrierChanged)
    Q_PROPERTY(double minMeshSize READ minMeshSize WRITE setMinMeshSize NOTIFY minMeshSizeChanged)
    Q_PROPERTY(double initialMeshSize READ initialMeshSize WRITE setInitialMeshSize NOTIFY initialMeshSizeChanged)

public:
    explicit NomadSolverWrapper(QObject* parent = nullptr) : m_solver(){}

    [[nodiscard]] QString name() const override { return "MADS (NOMAD)"; }
    ResourceAllocationSolver* getSolver() override { return &m_solver; }
    [[nodiscard]] QString settingsComponent() const override { return "ui/NomadSettings.qml"; }

    // --- Getters / Setters pour les propriétés ---

    [[nodiscard]] int maxEvalCount() const { return m_solver.max_eval_count; }

    void setMaxEvalCount(int val)
    {
        if (m_solver.max_eval_count != val)
        {
            m_solver.max_eval_count = val;
            emit maxEvalCountChanged();
        }
    }

    [[nodiscard]] bool useXtrmBarrier() const { return m_solver.use_XTRM_barrier; }

    void setUseXtrmBarrier(bool val)
    {
        if (m_solver.use_XTRM_barrier != val)
        {
            m_solver.use_XTRM_barrier = val;
            emit useXtrmBarrierChanged();
        }
    }

    [[nodiscard]] double minMeshSize() const { return m_solver.min_mesh_size; }
    void setMinMeshSize(double val) { m_solver.min_mesh_size = val; emit minMeshSizeChanged();}


    [[nodiscard]] double initialMeshSize() const { return m_solver.initial_mesh_size; }
    void setInitialMeshSize(double val) { m_solver.initial_mesh_size = val; emit initialMeshSizeChanged();}

signals:
    void maxEvalCountChanged();
    void useXtrmBarrierChanged();
    void minMeshSizeChanged();
    void initialMeshSizeChanged();

private:
    NomadRessourceAlloc m_solver;
};
