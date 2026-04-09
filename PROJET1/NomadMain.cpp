#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "src/Centrale.cpp"
#include "src/NomadEvaluator.hpp"
#include "nomad.hpp"

int main(int argc, char* argv[]) {
    // 2. Paramètres fixe pour le test
    float H_amont_test = 137.88f;
    float debitTotTest = 578.0f;

    // 3. Initialisation de la centrale
    Centrale centrale;
    centrale.load5DefaultTurb();
    centrale.H_amont = H_amont_test;

    NOMAD::Display out(std::cout);
    out.precision (NOMAD::DISPLAY_PRECISION_STD );

    try
    {
        NOMAD::begin(0, nullptr);
        NOMAD::Parameters params = NOMAD::Parameters(out);
        params.set_DIMENSION(5); // 5 turbines
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
        params.set_LOWER_BOUND(NOMAD::Point(5, 0.0));
        params.set_UPPER_BOUND(NOMAD::Point(5, 160.0));

        params.set_MAX_BB_EVAL(500);
        NOMAD::Point orig(5, 1.0);
        /*
        orig.set_coord(0, 0); // Débit initial pour la turbine 1
        orig.set_coord(1, 147.719); // Débit initial pour la turbine
        orig.set_coord(2, 138.006);
        orig.set_coord(3, 148.723);
        orig.set_coord(4, 143.551);
        */
        std::vector<float> debits({0.0f, 147.719f, 138.006f, 148.723f, 143.551f});
        VecDebitPower powers = centrale.CalculatePowerGivenDistribution(debits);
        float puissanceTotale = 0.0f;
        for (const auto& dp : powers) {
            puissanceTotale += dp.second;
        }
        std::cout << "Puissance totale initiale : " << puissanceTotale << " MW" << std::endl;
        params.set_X0(orig); // Point de départ initial
        try {
            params.check(); // Vérifie la validité des paramètres
        } catch (const std::exception& e) {
            std::cerr << "Erreur dans les paramètres : " << e.what() << std::endl;
            return 1;
        }

        for (auto obj : params.get_index_obj())
        {
            std::cout << "Objectif à optimiser : " << obj << std::endl;
        }

        NomadEvaluator ev(params, centrale, debitTotTest);

        NOMAD::Mads mads(params, &ev);

        mads.run();

        const NOMAD::Eval_Point * best = mads.get_best_feasible();

        if (best) {
            std::cout << "\n--- SOLUTION OPTIMALE TROUVÉE ---" << std::endl;

            auto Qturb = 0.0f;
            for (int i = 0; i < 5; ++i) {
                Qturb += best->value(i);
                std::cout << "Turbine " << (i+1) << " : " << best->value(i) << " m3/s" << std::endl;
            }

            std::cout << "Qturb / Qtot = " << Qturb << " / "<< debitTotTest <<" m3/s" << std::endl;
            double puissanceMax = -best->get_bb_outputs()[0].value();
            std::cout << "Puissance Totale : " << puissanceMax << " MW" << std::endl;

            // 3. Extraction de la contrainte (Optionnel)
            double ecartQ = best->get_bb_outputs()[1].value();
            std::cout << "Ecart de débit (Contrainte) : " << ecartQ << std::endl;
        } else {
            std::cout << "Aucune solution réalisable n'a été trouvée." << std::endl;
        }

        const NOMAD::Stats & stats = mads.get_stats();

        std::cout << "\n=== STATISTIQUES DÉTAILLÉES ===" << std::endl;
        std::cout << "Nombre d'itérations      : " << stats.get_iterations() << std::endl;
        std::cout << "Evaluations boîte noire  : " << stats.get_bb_eval() << std::endl;
        std::cout << "Evaluations simulées     : " << stats.get_sgte_eval() << std::endl; // Si surrogates activés
        std::cout << "Temps d'évaluation        :" << stats.get_real_time() << std::endl;
        // Pourcentage de succès des itérations
        if (stats.get_iterations() > 0) {
            std::cout << "Efficacité (BBE/Iter)    : " << static_cast<float>(stats.get_bb_eval()) / static_cast<float>(stats.get_iterations()) << std::endl;
        }

        NOMAD::end();
    } catch (const std::exception& e)
    {
        std::cerr << "Erreur lors de l'exécution de NOMAD : " << e.what() << std::endl;
    }


    return 0;
}
