#pragma once

#include <utility>

#include "nomad.hpp"

class NomadEvaluator : public NOMAD::Evaluator {
public:
    float Q_cible;
    std::vector<std::function<float(float)>> _gainFunctions;

    NomadEvaluator(const NOMAD::Parameters& p, float q, std::vector<std::function<float(float)>> gainFunctions)
        : NOMAD::Evaluator(p), Q_cible(q), _gainFunctions(std::move(gainFunctions))
    {
        _is_multi_obj = false; // On n'optimise qu'un seul objectif (maximiser la puissance)
    }

    // La méthode que NOMAD appelle à chaque itération
    bool eval_x(NOMAD::Eval_Point& x, const NOMAD::Double& display_stats, bool& count_eval) const override {

        if(x.size() != _gainFunctions.size()) return false;
        int nb_functions = x.size();

        count_eval = true;
        // 1. Récupérer les débits suggérés par NOMAD
        // x(0) est Q1, x(1) est Q2, etc.
        float totalQ = 0;
        float totalP = 0;

        std::vector<float> debitParTurbine(nb_functions);
        for (int i = 0; i < nb_functions; ++i)
        {
            float Ressource_consuption = static_cast<float>(x[i].value());
            totalQ += Ressource_consuption;
            totalP += _gainFunctions[i](Ressource_consuption);
        }

        // 2. Définir l'objectif (NOMAD minimise f(x))
        x.set_bb_output(0, -totalP);

        // 3. Définir la contrainte (EB ou PB)
        // La somme des débits doit être égale à Q_cible (on utilise une marge)
        x.set_bb_output(1, std::abs(totalQ - Q_cible));
        return true; // Succès de l'évaluation
    }
};