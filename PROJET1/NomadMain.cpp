#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "src/Centrale.cpp"

int main(int argc, char* argv[]) {
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // 1. NOMAD passe un fichier en argument contenant les variables
    // Supposons ici ./blackbox randomfile.txt
    std::vector<float> q(5); // 5 turbines
    std::ifstream in ( argv[1] );

    if (!in.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << argv[1] << std::endl;
        return 3;
    }
    for (int i = 0; i < 5; ++i) {
        if (!(in >> q[i]))
        {
            std::cerr << "Erreur : Impossible de lire la valeur pour la turbine " << i+1 << std::endl;
            return 2;
        }
    }

    in.close();

    // 2. Paramètres fixe pour le test
    float H_amont_test = 137.88f;
    float debitTotTest = 578.0f;

    // 3. Initialisation de la centrale
    Centrale centrale;
    centrale.load5DefaultTurb();
    centrale.H_amont = H_amont_test;


    VecDebitPower powers = centrale.CalculatePowerGivenDistribution(q); // on calcule la distribution pour le débit total réel

    float p_tot = 0.0f;
    float d_tot = 0.0f;
    for (auto & [fst, snd] : powers)
    {
        p_tot += snd;
        d_tot += fst;
    }

    // 5. SORTIE POUR NOMAD
    // NOMAD minimise, donc on renvoie l'opposé de la puissance comme rslt
    // on a une contrainte PEB en 2nd : rslt validé si < 0
    std::cout << -p_tot << " " << d_tot - debitTotTest << std::endl;

    return 0;
}