#include <iostream>
#include <vector>
#include <math.h>

#include <OpenXLSX.hpp>

#define CALC_LEVEL_AVAL(DebitTotal) (2.9449f * std::logf(DebitTotal) + 84.867f)
#define PERTES_DE_CHARGE 0.000005f // soit 0.5 x 10^-5

// polynome interface
class Polynome
{
public:
    virtual float calculate(float x, float y) = 0;
};

class PolynomeAmont : public Polynome
{
public:
    constexpr static float a = 0.6368f;
    constexpr static float b = 6.3523f;
    constexpr static float c = 0.0068f;
    constexpr static float d = -0.0021f;
    constexpr static float e = -0.0878f;
    constexpr static float f = -152.19f;

    virtual float calculate(float x, float y) override
    {
        return a * x + b * y + c * x * y + d * x * x + e * y * y + f;
    }
};

class PolynomeAval : public Polynome
{
public:
    constexpr static float a = 0.3916f;
    constexpr static float b = 1.0177f;
    constexpr static float f = -45.427f;

    virtual float calculate(float x, float y) override
    {
        return a * x + b * y + f;
    }
};

class Turbine
{

    std::unique_ptr<Polynome> polynomeHighFlow;
    std::unique_ptr<Polynome> polynomeLowFlow;

public:
    Turbine() = default;

    Turbine(std::unique_ptr<Polynome> polyLow, std::unique_ptr<Polynome> polyHigh, float treashHold)
        : polynomeLowFlow(std::move(polyLow)),
          polynomeHighFlow(std::move(polyHigh)),
          flowThreshold(treashHold) {}

    float flowThreshold = 125.0f; // seuil de débit pour différencier les faibles et forts débits
    float rendement = 1.0f;       // 100% d'efficacité

    float calculate_power(float HauteurDeChute, float debit)
    {

        float Hnet = HauteurDeChute - PERTES_DE_CHARGE * debit * debit; // calcul de la hauteur de chute nette en tenant compte des pertes de charge
        if (debit <= flowThreshold)
        {
            printf("low flow\n");
            return polynomeLowFlow->calculate(debit, Hnet) * rendement;
        }
        else
        {
            return polynomeHighFlow->calculate(debit, Hnet) * rendement;
        }
    }
};

class Centrale
{
public:
    std::vector<Turbine> turbines;

    float H_amont = 0.0f; // hauteur d'eau en amont
    float H_aval = 0.0f;  // hauteur d'eau en aval

    Centrale() = default;

    void AddTurbine(Turbine turbine)
    {
        turbines.push_back(std::move(turbine));
    }

    std::vector<float> CalculatePower(float debitTotal)
    {
        H_aval = CALC_LEVEL_AVAL(debitTotal);
        float HauteurDeChute = H_amont - H_aval;

        std::vector<float> powers (5, 0.0f);
        for (int i = 0; i < turbines.size() - 1; ++i)
        {
            float power = turbines[i].calculate_power(HauteurDeChute, debitTotal / (turbines.size() - 1));
            powers[i] = power;
        }
        return powers;
    }
};

int main()
{
    // Création de la centrale et ajout de turbines avec des polynômes différents pour les faibles et forts débits

    Centrale centrale;

    centrale.H_amont = 137.89f; // exemple de hauteur d'eau en amont
    for (int i = 0; i < 5; ++i)
    {
        auto pLow = std::make_unique<PolynomeAval>();
        auto pHigh = std::make_unique<PolynomeAmont>();

        // Create a fresh turbine directly inside the adding logic
        Turbine t(std::move(pLow), std::move(pHigh), 125.0f);
        centrale.AddTurbine(std::move(t));
    }

    centrale.CalculatePower(539.25f);

    // read the B3 cell of the "Data" sheet in the "data.xlsx" file
    OpenXLSX::XLDocument doc;
    std::cout << "Opening data.xlsx..." << std::endl;
    doc.open("data/data.xlsx");
    std::cout << "Data file opened successfully." << std::endl;
    auto wks = doc.workbook().worksheet("Sheet1");
    std::cout << "Worksheet 'Sheet1' accessed successfully." << std::endl;
    auto cell = wks.cell("B3");
    std::cout << "Cell B3 accessed successfully." << std::endl;
    std::string cellValue = cell.value().get<std::string>();
    std::cout << "Value in B3: " << cellValue << std::endl;

    std::vector<std::pair<float, float>> turbines(5, {0.0f, 0.0f}); // vector pour stocker les débits et puissances des turbines

    for (int numline = 4; numline <= 100; ++numline)
    {
        float Elav = wks.cell("B" + std::to_string(numline)).value().get<float>();
        float QTot = wks.cell("C" + std::to_string(numline)).value().get<float>();
        float QTurb = wks.cell("D" + std::to_string(numline)).value().get<float>();
        float QVan = wks.cell("E" + std::to_string(numline)).value().get<float>();
        float N_Amont = wks.cell("F" + std::to_string(numline)).value().get<float>();

        float TotalPower = 0.0f;
        char firstTurb = 'G';
        for (int i = 0; i < 5; ++i)
        {
            float Qtrubi = wks.cell(std::string(1, firstTurb++) + std::to_string(numline)).value().get<float>();
            float PTurbi = wks.cell(std::string(1, firstTurb++) + std::to_string(numline)).value().get<float>();
            turbines[i].first = Qtrubi;
            turbines[i].second = PTurbi;
            TotalPower += turbines[i].first;
        }
        // on essais de reproduire les calculs avec notre centrale simulée.
        auto CalculatedPowers = centrale.CalculatePower(QTurb);
        // on compare les résultats

        std::cout << std::endl <<"nv aval : " << Elav << ", vs calcul : " << centrale.H_aval;
        std::cout << "\nReal :\t";
        for (int i = 0; i < 5; ++i)
        {
            std::cout << "T" << i + 1 << ": " << turbines[i].second << " kW, ";
        }
        std::cout << "\nCalc :\t";
        for (int i = 0; i < 5; ++i)
        {
            std::cout << "T" << i + 1 << ": " << CalculatedPowers[i] << " kW, ";
        }
    }

    std::cout << std::endl;
}
