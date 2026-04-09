#pragma once
#include <QtQml/qqmlregistration.h>
#include "SolverQtWrappers.h"

//#include "NomadSolver.h"

#if true
class NomadRessourceAlloc : public ResourceAllocationSolver
{
    public:
    NomadRessourceAlloc() {};
    ~NomadRessourceAlloc() override = default;

    std::vector<std::pair<float, float>> allocateResources() override { return {}; }

};
#endif

class NomadSolverWrapper : public SolverWrapper
{
    Q_OBJECT

public:
    explicit NomadSolverWrapper(QObject* parent = nullptr)
    {
        m_solver = NomadRessourceAlloc();
    }

    QString name() const override { return "NOMAD"; }
    ResourceAllocationSolver* getSolver() override { return &m_solver; }
    QString settingsComponent() const override { return "ui/DPSettings.qml"; }

private :
    NomadRessourceAlloc m_solver;
};
