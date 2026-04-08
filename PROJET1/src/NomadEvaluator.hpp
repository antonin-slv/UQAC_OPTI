#pragma once

#include "nomad.hpp"
#include "Centrale.cpp"

class NomadEvaluator : public NOMAD::Evaluator {
public:
    Centrale& centrale;
    float Q_cible;

    NomadEvaluator(const NOMAD::Parameters& p, Centrale& c, float q)
        : NOMAD::Evaluator(p), centrale(c), Q_cible(q)
    {
        _is_multi_obj = false; // On n'optimise qu'un seul objectif (maximiser la puissance)
    }

    // La méthode que NOMAD appelle à chaque itération
    bool eval_x(NOMAD::Eval_Point& x, const NOMAD::Double& display_stats, bool& count_eval) const override {
        count_eval = true;
        // 1. Récupérer les débits suggérés par NOMAD
        // x(0) est Q1, x(1) est Q2, etc.
        float totalQ = 0;
        float totalP = 0;

        std::vector<float> debitParTurbine(5);
        for (int i = 0; i < 5; ++i)
        {
            debitParTurbine[i] = static_cast<float>(x[i].value());
            totalQ += debitParTurbine[i];
        }

        VecDebitPower resultats = centrale.CalculatePowerGivenDistribution(debitParTurbine);

        for (int i = 0; i < 5; ++i) {
            totalP += resultats[i].second;
        }

        // 2. Définir l'objectif (NOMAD minimise f(x))
        x.set_bb_output(0, -totalP);

        // 3. Définir la contrainte (EB ou PB)
        // La somme des débits doit être égale à Q_cible (on utilise une marge)
        x.set_bb_output(1, std::abs(totalQ - Q_cible));
        return true; // Succès de l'évaluation
    }
};