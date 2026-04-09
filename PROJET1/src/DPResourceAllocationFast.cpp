#pragma once

#include <functional>
#include <vector>
#include <cmath>
#include <algorithm>
#include <omp.h>

#include "ResourceAllocationSolver.cpp"

class DPResourceAllocationFast : public ResourceAllocationSolver {

public:
    using GainFunction = std::function<float(float)>;

private:

    float m_step;

public:
    [[nodiscard]] float get_step() const
    {
        return m_step;
    }

    void set_step(float step)
    {
        m_step = step;
    }

    explicit DPResourceAllocationFast(const float step): m_step(step) {}

    /**
     *fonction d'allocation de ressources avec programmation dynamique.
     * La fonction retourner un vecteur de paires (décision, gain) pour chaque fonction de gain, où la décision est la quantité de ressource allouée à cette fonction et le gain est le gain correspondant.
     */
    [[nodiscard]] std::vector<std::pair<float, float> > allocateResources() override {

        struct Cell {
            float bestGain = -1.0f;     // Le gain max pour cette ressource à cette étape
            float choiceMade = 0.0f;    // Le débit choisi pour la turbine actuelle
        };

        // La matrice : [Nombre de Turbines][Nombre de pas de discrétisation]
        const int nbSteps = static_cast<int>(m_totalResource / m_step) + 1;
        std::vector<std::vector<Cell>> dpTable(m_functions.size(), std::vector<Cell>(nbSteps));

        const float NEG_INF = -1e15f; // Plus propre qu'un chiffre au hasard
        float sumMin = 0;
        for (int i = 0; i < m_functions.size(); ++i) {  // i : etapes (0 à nombre de turbines)
            sumMin += m_bounds[i].first;
            #pragma omp parallel for
            for (int r = 0; r < nbSteps; ++r) {         // r : etat (ressource restante)
                float availableDebit = m_step * static_cast<float>(r);

                dpTable[i][r].bestGain = NEG_INF;
                // Si la ressource totale ne couvre même pas le min des turbines 0 à i
                if (availableDebit < sumMin) continue;

                // tous les choix
                const float minB = m_bounds[i].first;
                const float maxB = std::min(m_bounds[i].second, availableDebit);

                for (float choice = minB; choice <= maxB; choice += m_step) {
                    const float currentGain = m_functions[i](choice);
                    float totalGain = currentGain;

                    if (i > 0) {
                        const int remainingIdx = std::lround( (availableDebit - choice) / m_step);
                        float prevGain = dpTable[i-1][remainingIdx].bestGain;
                        if (prevGain <= NEG_INF) continue; // État précédent impossible
                        totalGain += prevGain;
                    }

                    if (totalGain > dpTable[i][r].bestGain) {
                        dpTable[i][r].bestGain = totalGain;
                        dpTable[i][r].choiceMade = choice;
                    }
                }
            }
        }

        // ----------------- Backtracking
        std::vector<std::pair<float, float>> results(m_functions.size());
        float currentRemainingRes = m_totalResource;

        currentRemainingRes = std::floor(currentRemainingRes / m_step) * m_step;

        for (int i = static_cast<int>(m_functions.size()) - 1; i >= 0; --i) {
            const int rIdx = std::lround(currentRemainingRes / m_step );

            float chosenDebit = dpTable[i][rIdx].choiceMade;
            float gainObtained = m_functions[i](chosenDebit);

            results[i] = {chosenDebit, gainObtained};

            currentRemainingRes -= chosenDebit;
        }

        return results;
    }
};
