#include <iostream>
#include <vector>
#include <OpenXLSX.hpp>

#include "src/Centrale.cpp"
#include "src/DPResourceAllocation.cpp"
#include "src/DPResourceAllocationFast.cpp"
#include <fstream>
#include <iomanip>
#include <sstream>


std::string PrintNumber(double val)
{
    // Si la valeur est entière (ex: 135.0), on la convertit en long pour supprimer le .0
    if (std::floor(val) == val)
    {
        return std::to_string((long)val);
    }
    // Sinon, on formate avec une décimale
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << val;
    return ss.str();
}

void print_rslt_to_output(std::ofstream& logFile, int firstLine, int nbLine, int numline, VecDebitPower turbines,
                          float QTurb, float TotalRealPower, float totalCalcP, float totalCalcQ,
                          VecDebitPower CalculatedPowers)
{
    std::string entete_latex = R"(\begin{table}[h!]
\centering
\caption{Comparaison des débits ($m^3/s$) et puissances ($MW$)}
\label{tab:comparaison_103}
\resizebox{\textwidth}{!}{%
\begin{tabular}{|c|c|ccccc|cc|ccccc|}
\hline
\textbf{Ligne} & \textbf{Type} & \textbf{$Q_1$} & \textbf{$Q_2$} & \textbf{$Q_3$} & \textbf{$Q_4$} & \textbf{$Q_5$} & \textbf{$Q_t_o_t$} & \textbf{$P_t_o_t$} &  \textbf{$P_1$} & \textbf{$P_2$} & \textbf{$P_3$} & \textbf{$P_4$} & \textbf{$P_5$} \\ \hline)";

    std::string pied_latex = R"(\end{tabular}%
}
\end{table})";


    int sectionNumbers = 1;
    int linesPerSection = nbLine / sectionNumbers;
    int currentIdx = numline - firstLine;

    if (numline == firstLine)
    {
        logFile << entete_latex << std::endl;
    }

    logFile << numline << " & \\textbf{Reel}";
    // Débits unitaires (Q1-Q5)
    for (int i = 0; i < 5; ++i) logFile << " & " << turbines[i].first;
    // Totaux (Qtot, Ptot)
    logFile << " & " << PrintNumber(QTurb) << " & " << PrintNumber(TotalRealPower);
    // Puissances unitaires (P1-P5)
    for (int i = 0; i < 5; ++i) logFile << " & " << PrintNumber(turbines[i].second);
    logFile << R"( \\ \cline{2-14})" << std::endl;

    // --- LIGNE CALCULÉ ---
    logFile << "  & \\textbf{Calcul}";
    // Débits unitaires (Q1-Q5)
    for (int i = 0; i < 5; ++i) logFile << " & " << CalculatedPowers[i].first;
    // Totaux (Qtot, Ptot)
    logFile << " & " << PrintNumber(totalCalcQ) << " & " << PrintNumber(totalCalcP);
    // Puissances unitaires (P1-P5)
    for (int i = 0; i < 5; ++i) logFile << " & " << PrintNumber(CalculatedPowers[i].second);
    logFile << R"( \\ \hline)" << std::endl;

    bool isEndOfSection = (currentIdx + 1) % linesPerSection == 0;
    bool isNotLastTotalLine = (numline != firstLine + nbLine - 1);
    if (isEndOfSection && isNotLastTotalLine)
    {
        // On ferme le tableau actuel, on ajoute un espace ou un saut de page,
        // et on ré-ouvre un nouveau tableau avec l'en-tête
        logFile << pied_latex << "\n\n\\newpage\n\n" << entete_latex << std::endl;
    }

    if (numline == firstLine + nbLine - 1)
    {
        logFile << pied_latex << std::endl;
    }
}

int main()
{
    // centrale.lockTurbine(2);

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

    int firstLine = 4;
    int nbLine = 1;

    //ON CREE LA CENTRALE
    Centrale centrale;
    centrale.load5DefaultTurb();

    // ON AJOUTE LES CONTRAINTES (par defaut maxDebit = 160, minDebit = 0)

    // centrale.turbines[1].maxDebit = 130;
    // centrale.lockTurbine(2);

    // ON CHOISIT LE SOLVEUR (ici, la programmation dynamique)
    // DPResourceAllocation solver(1.0f); // pas de 1 unité de débit
    DPResourceAllocationFast solver(20.0f); // pas de 1 unité de débit
    centrale.setSolver(&solver);

    std::ofstream logFile("resultats_validation.tex", std::ios::app);

    std::cout << "Launch the prog" << std::endl;

    for (int numline = firstLine; numline < nbLine + firstLine; ++numline)
    {
        //auto Elav = wks.cell("B" + std::to_string(numline)).value().get<float>();
        //auto QTot = wks.cell("C" + std::to_string(numline)).value().get<float>();
        auto QTurb = wks.cell("D" + std::to_string(numline)).value().get<float>();
        //auto QVan = wks.cell("E" + std::to_string(numline)).value().get<float>();
        auto N_Amont = wks.cell("F" + std::to_string(numline)).value().get<float>();

        float TotalRealPower = 0.0f;
        char firstTurb = 'G';
        for (int i = 0; i < 5; ++i)
        {
            auto Qtrubi = wks.cell(std::string(1, firstTurb++) + std::to_string(numline)).value().get<float>();
            auto PTurbi = wks.cell(std::string(1, firstTurb++) + std::to_string(numline)).value().get<float>();
            turbines[i].first = Qtrubi;
            turbines[i].second = PTurbi;
            TotalRealPower += turbines[i].second;
        }

        // ON paramètre la centrale.
        centrale.H_amont = N_Amont;
        float totalCalcP = 0, totalCalcQ = 0;

        auto CalculatedPowers = centrale.CalculateDistributionAndPower(QTurb);

        for (auto& [fst, snd] : CalculatedPowers)
        {
            totalCalcP += snd;
            totalCalcQ += fst;
        }

        if (logFile.is_open())
        {
            print_rslt_to_output(logFile, firstLine, nbLine, numline, turbines, QTurb, TotalRealPower,
                                 totalCalcP, totalCalcQ, CalculatedPowers);
        }

        // std::cout << " CALC_H_CHUTE_NETTE : " << CALC_H_CHUTE_NETTE(N_Amont - CALC_LEVEL_AVAL(600), 140) << std::endl;
    }
}
