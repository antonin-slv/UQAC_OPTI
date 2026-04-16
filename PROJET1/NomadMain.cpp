#include <iostream>
#include <omp.h>
#include <string>
#include <vector>
#include "src/Centrale.cpp"
#include "NomadSolver.h"
#include "DPResourceAllocationFast.cpp"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Erreur : Arguments manquants. Usage: QTapp.exe <H_amont> <Debit>" << std::endl;
        return 1;
    }
    std::streambuf* orig_buf = std::cout.rdbuf();
    //cut off std::cout
    std::cout.rdbuf(nullptr);

    // 1. Récupération des arguments envoyés par PowerShell
    // Remplacement des éventuelles virgules par des points (problème classique sous Windows)
    std::string arg1 = argv[1]; std::replace(arg1.begin(), arg1.end(), ',', '.');
    std::string arg2 = argv[2]; std::replace(arg2.begin(), arg2.end(), ',', '.');

    float H_amont_test = std::stof(arg1);
    float debitTotTest = std::stof(arg2);

    // 2. Initialisation
    Centrale centrale;
    centrale.load5DefaultTurb();
    centrale.H_amont = H_amont_test;

    /*
    NomadRessourceAlloc nomadSolver;
    nomadSolver.display_degree = 0;
    nomadSolver.use_XTRM_barrier = true;
    */
    DPResourceAllocationFast nomadSolver(0.05   );
    centrale.solver = &nomadSolver;

    // 3. Exécution et Chronométrage
    float sttime = omp_get_wtime();
    auto rslt = centrale.CalculateDistributionAndPower(debitTotTest);
    float elapsed_time = omp_get_wtime() - sttime;

    // 4. Récupération des résultats
    float tot_power = 0;
    for (const auto& [debit, power] : rslt) {
        tot_power += power;
    }

    // Récupération du nombre d'évaluations (itérations) de NOMAD
    // (À adapter selon l'implémentation exacte de ton wrapper)
    //int nb_evals = nomadSolver.max_eval_count;
int nb_evals = 0;
    // 5. Affichage propre pour que PowerShell puisse lire (Séparé par des espaces)
    // Format : Temps(s) Puissance(MW) Iterations
    //enable stdout
    std::cout.rdbuf(orig_buf);
    std::cout << elapsed_time << " " << tot_power << " " << nb_evals << std::endl;

    return 0;
}