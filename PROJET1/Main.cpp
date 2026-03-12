#include <iostream>
#include <vector>
#include <OpenXLSX.hpp>

#include "src/centrale.cpp"

int main() {

    Centrale centrale;
    centrale.load5DefaultTurb();
    centrale.lockTurbine(2);

    // read the B3 cell of the "Data" sheet in the "data.xlsx" file
    OpenXLSX::XLDocument doc;
    std::cout << "Opening data.xlsx..." << std::endl;
    doc.open("data/data.xlsx");
    std::cout << "Data file opened successfully." << std::endl;
    auto wks = doc.workbook().worksheet("Sheet1");
    std::cout << "Worksheet 'Sheet1' accessed successfully." << std::endl;
    auto cell = wks.cell("B3");
    std::cout << "Cell B3 accessed successfully." << std::endl;
    auto cellValue = cell.value().get<std::string>();
    std::cout << "Value in B3: " << cellValue << std::endl;

    VecDebitPower turbines(5, {0.0f, 0.0f}); // vector pour stocker les débits et puissances des turbines

    for (int numline = 4; numline <= 7; ++numline) {
        auto Elav = wks.cell("B" + std::to_string(numline)).value().get<float>();
        auto QTot = wks.cell("C" + std::to_string(numline)).value().get<float>();
        auto QTurb = wks.cell("D" + std::to_string(numline)).value().get<float>();
        auto QVan = wks.cell("E" + std::to_string(numline)).value().get<float>();
        auto N_Amont = wks.cell("F" + std::to_string(numline)).value().get<float>();

        std::cout << "\n\n--- Line " << numline << " ---\n";
        std::cout << "Elav: " << Elav << ", QTot: " << QTot << ", QTurb: " << QTurb << ", QVan: " << QVan <<
                ", N_Amont: " << N_Amont << std::endl;

        float TotalPower = 0.0f;
        char firstTurb = 'G';
        for (int i = 0; i < 5; ++i) {
            auto Qtrubi = wks.cell(std::string(1, firstTurb++) + std::to_string(numline)).value().get<float>();
            auto PTurbi = wks.cell(std::string(1, firstTurb++) + std::to_string(numline)).value().get<float>();
            turbines[i].first = Qtrubi;
            turbines[i].second = PTurbi;
            TotalPower += turbines[i].first;
        }

        // on essais de reproduire les calculs avec notre centrale simulée.
        auto CalculatedPowers = centrale.CalculateAveragePower(QTot, QVan, N_Amont);

        // on compare les résultats
        std::cout << "centrale stats : " << centrale.H_amont << " m amont, " << centrale.H_aval << " m aval, chute : "
                << centrale.H_amont - centrale.H_aval << " m\n";

        std::cout << "nv aval : " << Elav << ", vs calcul : " << centrale.H_aval;
        std::cout << "\nReal :\t";
        for (int i = 0; i < 5; ++i) {
            std::cout << "T" << i + 1 << ": " << turbines[i].second << " kW, ";
        }
        std::cout << "\nCalc :\t";
        for (int i = 0; i < 5; ++i) {
            std::cout << "T" << i + 1 << ": " << CalculatedPowers[i].second << "kW, ";
        }
    }

    std::cout << std::endl;
}
