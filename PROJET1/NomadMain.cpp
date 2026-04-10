#include <iostream>
#include <string>
#include <vector>
#include "src/Centrale.cpp"
#include "NomadSolver.h"

int main(int argc, char* argv[]) {
    // 2. Paramètres fixe pour le test
    float H_amont_test = 137.88f;
    float debitTotTest = 578.0f;

    // 3. Initialisation de la centrale
    Centrale centrale;
    centrale.load5DefaultTurb();
    centrale.H_amont = H_amont_test;

    NomadRessourceAlloc nomadSolver;
    centrale.solver = &nomadSolver;

        // 4. Exécution de l'allocation
    auto rslt  =  centrale.CalculateDistributionAndPower(debitTotTest);

    for (const auto& [debit, power] : rslt) {
        std::cout << "Débit: " << debit << " m3/s, Puissance: " << power << " MW\n";
    }


    return 0;
}
