#pragma once
#include <cassert>
#include <vector>
#include <functional>
#include <algorithm>

class ResourceAllocationSolver {
public:
    virtual ~ResourceAllocationSolver() = default;
    using ProdFunction = std::function<float(float)>;
    /* fonction permettant de calculer le gain en fonction de la ressource allouée. */
    std::vector<ProdFunction> m_functions;

    /* les limites inférieure et supérieure de la ressource allouée pour chaque fonction de gain. */
    std::vector<std::pair<float, float>> m_bounds;

    /* la quantité totale de ressource disponible pour l'allocation. */
    float m_totalResource;

    void setParameters(
        std::vector<ProdFunction> functions,
        std::vector<std::pair<float, float>> bounds,
        float totalResource) {
        m_functions = std::move(functions);
        m_bounds = std::move(bounds);
        m_totalResource = totalResource;

        assert(m_functions.size() == m_bounds.size());
    }

    std::vector<std::pair<float, float>> virtual allocateResources() = 0;
};
