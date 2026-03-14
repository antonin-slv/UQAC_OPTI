#include <cassert>
#include <functional>
#include <vector>

class DPResourceAllocation {
    std::function<std::pair<float, float>(std::vector<std::pair<float, float> >)> maxFloat = [
            ](const std::vector<std::pair<float, float> > &gains) {
        std::pair<float, float> bestGain = {0.0f, 0.0f};
        for (auto gain: gains) {
            if (gain.second > bestGain.second) {
                bestGain = gain;
            }
        }
        return bestGain;
    };

public:
    using GainFunction = std::function<float(float)>;

private:
    /* fonction permettant de calculer le gain en fonction de la ressource allouée. */
    std::vector<GainFunction> m_functions;
    /* les limites inférieure et supérieure de la ressource allouée pour chaque fonction de gain. */
    std::vector<std::pair<float, float> > m_bounds;
    /* la quantité totale de ressource disponible pour l'allocation. */
    float m_totalResource;
    /* Pas de discrétisation pour l'allocation de ressources. */
    float m_step;

public:
    DPResourceAllocation(
        std::vector<GainFunction> functions,
        float totalResource,
        std::vector<std::pair<float, float> > bounds,
        float step)
        : m_functions(std::move(functions)),
          m_totalResource(totalResource),
          m_bounds(std::move(bounds)),
          m_step(step) {
        assert(m_functions.size() == m_bounds.size());
    }


    /**
     *fonction d'allocation de ressources avec programmation dynamique.
     * La fonction retourner un vecteur de paires (décision, gain) pour chaque fonction de gain, où la décision est la quantité de ressource allouée à cette fonction et le gain est le gain correspondant.
     */
    std::vector<std::pair<float, float> > allocateResources() {
        struct DPEntry {
            float allocatedResource; // la quantité de ressource allouée à cette fonction
            float gain; // le gain correspondant à cette allocation
        };
        // table de programmation dynamique pour stocker les décisions et les gains
        std::vector< //1 : chaque étape
            std::vector< //2 : ligne du tableau de l'étape
                std::pair<float, //ressources restantes (Sn)
                    std::vector< // 3 :  choix
                        std::pair<float, // choix à cette étape
                            float> // 4 : gain total avec les meilleurs choix précédents
                    > >
            >
        > dpTable(m_functions.size());
        //TODO(tester de limiter les états initiaux max)


        for (int i = 0; i < m_functions.size() -1; ++i) {
            float Conso = m_bounds[i].first;
            while (Conso <= m_totalResource) {
                dpTable[i].emplace_back(m_totalResource - Conso, std::vector<std::pair<float, float> >());
                Conso += m_step;
            }
        }
        dpTable[m_functions.size() - 1].emplace_back(m_totalResource, std::vector<std::pair<float, float> >()); // pour la dernière fonction, on commence avec la ressource totale disponible

        for (int i = 0; i < m_functions.size(); ++i) {
            for (auto& jsp: dpTable[i]) {
                float AllocationChoice = m_bounds[i].first;
                assert(AllocationChoice <= m_bounds[i].second);
                assert(AllocationChoice <= m_totalResource);
                while (AllocationChoice <= m_bounds[i].second && AllocationChoice <= jsp.first) {
                    float gain = m_functions[i](AllocationChoice);

                    float remaining = jsp.first - AllocationChoice;
                    float bestGainForPreviousFunctions = 0.0f;
                    if (i != 0) {
                        for (const auto& entry: dpTable[i-1]) {
                            if (entry.first < remaining + 0.01f && entry.first > remaining - 0.01f) {
                                // si les ressources restantes sont suffisantes pour l'allocation actuelle
                                bestGainForPreviousFunctions = entry.second.empty() ? 0.0f : maxFloat(entry.second).second;
                                break;
                            }
                        }
                    }
                    gain += bestGainForPreviousFunctions; // on ajoute le meilleur gain des fonctions précédentes

                    // stocker la décision et le gain pour cette allocation
                    jsp.second.emplace_back(AllocationChoice, gain);
                    // choix et gain pour cette allocation
                    AllocationChoice += m_step;
                }
            }
        }

        auto bestOverallGain = maxFloat(dpTable.back().front().second); // on prend le meilleur gain global à la fin de la table

        auto resultat = std::vector<std::pair<float, float>>(m_functions.size());

        float remainingResource = m_totalResource;

        for (int i = m_functions.size() - 1; i >= 0; --i) {
            for (auto& entry: dpTable[i]) {
                if (entry.first < remainingResource + 0.01f && entry.first > remainingResource - 0.01f) {
                    resultat[i] = maxFloat(entry.second);
                    if (i < m_functions.size() - 1) {
                        resultat[i+1].second -= resultat[i].second;
                    }
                    remainingResource -= resultat[i].first;
                    break;
                }
            }
        }

        return resultat;
    }
};
