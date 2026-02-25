#include <iostream>
#include <vector>
#include <math.h>



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
        } else {
            printf("high flow\n");
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

    void CalculatePower(float debitTotal)
    {
        H_aval = CALC_LEVEL_AVAL(debitTotal);
        
        printf("Hauteur d'eau en aval: %f m\n", H_aval);
        float HauteurDeChute = H_amont - H_aval;


        
        float totalPower = 0.0f;
        for (int i = 0; i < turbines.size() -1; ++i)
        {
            float power = turbines[i].calculate_power(HauteurDeChute, debitTotal/ (turbines.size() -1));
            std::cout << "Puissance de la turbine: " << power << " MW" << std::endl;
            totalPower += power;
        }
        std::cout << "Puissance totale de la centrale: " << totalPower << " MW" << std::endl;
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

}