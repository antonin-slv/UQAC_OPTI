//
// Created by anton on 08/04/2026.
//

#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "NomadEvaluator.hpp"
#include "nomad.hpp"
#include "ResourceAllocationSolver.cpp"

class NomadRessourceAlloc : public ResourceAllocationSolver
{
public:
    NomadRessourceAlloc() = default;

    std::vector<std::pair<float, float>> allocateResources() override
    {
        NOMAD::Display out(std::cout);
        out.precision(NOMAD::DISPLAY_PRECISION_STD);

        int nb_fct_prod = static_cast<int>(m_functions.size());

        assert(nb_fct_prod == m_bounds.size());
        assert(nb_fct_prod > 0);
        try
        {
            NOMAD::begin(0, nullptr);
            NOMAD::Parameters params = NOMAD::Parameters(out);
            params.set_DIMENSION(nb_fct_prod);
            std::vector<NOMAD::bb_output_type> bout(2);
            bout[0] = NOMAD::OBJ;
            bout[1] = NOMAD::PB;
            params.set_SEED(0);
            params.set_BB_OUTPUT_TYPE(bout);
            params.set_DISPLAY_DEGREE(2);
            params.set_DISPLAY_STATS("BBE ( SOL ) OBJ");
            /*
            params.set_INITIAL_MESH_SIZE(NOMAD::Point(5, 0.5));
            params.set_INITIAL_POLL_SIZE(NOMAD::Point(5, 0.1));
            */
            NOMAD::Point lower_bound(0);
            NOMAD::Point upper_bound(0);
            int i = 0;
            for (auto& [fst, snd] : m_bounds)
            {
                lower_bound.set_coord(i, fst);
                upper_bound.set_coord(i, snd);
                i++;
            }

            params.set_LOWER_BOUND(lower_bound);
            params.set_UPPER_BOUND(upper_bound);

            params.set_MAX_BB_EVAL(500);
            NOMAD::Point orig(5, 1.0);
            /*
            orig.set_coord(0, 0); // Débit initial pour la turbine 1
            orig.set_coord(1, 147.719); // Débit initial pour la turbine
            orig.set_coord(2, 138.006);
            orig.set_coord(3, 148.723);
            orig.set_coord(4, 143.551);
            */
            /*
            std::vector<float> debits({0.0f, 147.719f, 138.006f, 148.723f, 143.551f});
            VecDebitPower powers = centrale.CalculatePowerGivenDistribution(debits);
            float puissanceTotale = 0.0f;
            for (const auto& dp : powers)
            {
                puissanceTotale += dp.second;
            }
            std::cout << "Puissance totale initiale : " << puissanceTotale << " MW" << std::endl;
            */
            params.set_X0(orig); // Point de départ initial
            try
            {
                params.check(); // Vérifie la validité des paramètres
            }
            catch (const std::exception& e)
            {
                std::cerr << "Erreur dans les paramètres : " << e.what() << std::endl;
                return {};
            }

            for (auto obj : params.get_index_obj())
            {
                std::cout << "Objectif à optimiser : " << obj << std::endl;
            }

            NomadEvaluator ev(params, m_totalResource, m_functions);

            NOMAD::Mads mads(params, &ev);

            mads.run();

            const NOMAD::Eval_Point* best = mads.get_best_feasible();
            if (best)
            {
                std::vector<std::pair<float, float>> results;
                for (i = 0; i < nb_fct_prod; ++i)
                {
                    float conso = best->value(i);
                    results.emplace_back(conso, m_functions[i](conso));
                }
            return results;
            };
            return {};
        }
        catch (const std::exception& e)
        {
            std::cerr << "Erreur lors de l'exécution de NOMAD : " << e.what() << std::endl;
            return {};
        }
    };
};

