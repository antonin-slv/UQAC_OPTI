#include <cmath>
#include <memory>
#include <vector>
#include "Calculator.cpp"

#define CALC_LEVEL_AVAL(DebitTotal) (2.805f * std::logf(DebitTotal) + 85.76f)
#define PERTES_DE_CHARGE 0.000005f // soit 0.5 x 10^-5
#define CALC_H_CHUTE_NETTE(HauteurDeChute, Debit) (HauteurDeChute - PERTES_DE_CHARGE * Debit * Debit)


using VecDebitPower = std::vector<std::pair<float, float> >; // pour stocker les débits et puissances des turbines


class Turbine {
    std::unique_ptr<Calculator> claculator;

public:
    bool locked = false; // pour simuler une turbine en maintenance
    float rendement = 1.0f; // 100% d'efficacité

    Turbine() = default;

    Turbine(std::unique_ptr<Calculator> polyLow)
        : claculator(std::move(polyLow)) {
    }

    float calculate_power(float HauteurDeChute, float debit) {
        // calcul de la hauteur de chute nette en tenant compte des pertes de charge
        float Hnet = CALC_H_CHUTE_NETTE(HauteurDeChute, debit);
        // calcul de la puissance en fonction de la hauteur de chute nette et du débit
        return claculator->calculate(debit, Hnet) * rendement;
    }
};


class Centrale {
public:
    std::vector<Turbine> turbines;

    float H_amont = 0.0f; // hauteur d'eau en amont
    float H_aval = 0.0f; // hauteur d'eau en aval

    Centrale() = default;

    void AddTurbine(Turbine turbine) {
        turbines.push_back(std::move(turbine));
    }

    VecDebitPower CalculatePower(float debitTotal, float debitVan = 0.0f, float N_Amont = 0.0f) {
        H_amont = N_Amont;
        H_aval = CALC_LEVEL_AVAL(debitTotal);
        float HauteurDeChute = H_amont - H_aval;

        float debitTurbines = debitTotal - debitVan;

        int active_turbines = 0;

        VecDebitPower powers(0);
        for (int i = 0; i < turbines.size(); ++i) {
            active_turbines += !turbines[i].locked;
        }

        for (auto &turb: turbines) {
            if (!turb.locked) {
                float debitParTurbine = debitTurbines / active_turbines;
                // on répartit le débit total entre les turbines actives
                float debit = debitParTurbine; // débit pour la turbine courante
                float power = turb.calculate_power(HauteurDeChute, debit);
                powers.push_back({debit, power});
                // on calcule la puissance de chaque turbine et on l'ajoute à la liste des puissances
            } else {
                powers.push_back({0.0f, 0.0f}); // turbine en maintenance, pas de puissance produite
            }
        }
        return powers;
    }

    bool lockTurbine(int index) {
        if (index >= 0 && index < turbines.size()) {
            turbines[index].locked = true;
            return true;
        }

        return false;
    }

    bool unlockTurbine(int index) {
        if (index >= 0 && index < turbines.size()) {
            turbines[index].locked = false;
            return true;
        }

        return false;
    }


    void load5DefaultTurb() {
        PolyDeg3 polynomeT1 = {};
        polynomeT1.p00 = -2.624f;
        polynomeT1.p10 = -1.381f;
        polynomeT1.p01 = 0.2794f;
        polynomeT1.p20 = 0.00602f;
        polynomeT1.p11 = 0.06592f;
        polynomeT1.p02 = -0.00915f;
        polynomeT1.p30 = -1.597e-05f;
        polynomeT1.p21 = -4.901e-05f;
        polynomeT1.p12 = -0.0007456f;
        polynomeT1.p03 = 9.436e-05f;
        Turbine t1(std::make_unique<PolyDeg3>(polynomeT1));
        AddTurbine(std::move(t1));

        // --- Turbine 2 ---
        PolyDeg3 polynomeT2 = {};
        polynomeT2.p00 = -1.699f;
        polynomeT2.p10 = -1.954f;
        polynomeT2.p01 = 0.1789f;
        polynomeT2.p20 = 0.007711f;
        polynomeT2.p11 = 0.09284f;
        polynomeT2.p02 = -0.005728f;
        polynomeT2.p30 = -1.933e-05f;
        polynomeT2.p21 = -6.597e-05f;
        polynomeT2.p12 = -0.001106f;
        polynomeT2.p03 = 5.715e-05f;
        Turbine t2(std::make_unique<PolyDeg3>(polynomeT2));
        AddTurbine(std::move(t2));

        // --- Turbine 3 ---
        PolyDeg3 polynomeT3 = {};
        polynomeT3.p00 = -0.758f;
        polynomeT3.p10 = -1.215f;
        polynomeT3.p01 = 0.06992f;
        polynomeT3.p20 = 0.004243f;
        polynomeT3.p11 = 0.06423f;
        polynomeT3.p02 = -0.001757f;
        polynomeT3.p30 = -1.197e-05f;
        polynomeT3.p21 = -3.24e-05f;
        polynomeT3.p12 = -0.0007572f;
        polynomeT3.p03 = 1.064e-05f;
        Turbine t3(std::make_unique<PolyDeg3>(polynomeT3));
        AddTurbine(std::move(t3));

        // --- Turbine 4 ---
        PolyDeg3 polynomeT4 = {};
        polynomeT4.p00 = 0.3147f;
        polynomeT4.p10 = -1.263f;
        polynomeT4.p01 = -0.05313f;
        polynomeT4.p20 = 0.007074f;
        polynomeT4.p11 = 0.0543f;
        polynomeT4.p02 = 0.002703f;
        polynomeT4.p30 = -2.047e-05f;
        polynomeT4.p21 = -3.824e-05f;
        polynomeT4.p12 = -0.0005816f;
        polynomeT4.p03 = -4.155e-05f;
        Turbine t4(std::make_unique<PolyDeg3>(polynomeT4));
        AddTurbine(std::move(t4));

        // --- Turbine 5 ---
        PolyDeg3 polynomeT5 = {};
        polynomeT5.p00 = -0.3513f;
        polynomeT5.p10 = -1.366f;
        polynomeT5.p01 = 0.01318f;
        polynomeT5.p20 = 0.005064f;
        polynomeT5.p11 = 0.06935f;
        polynomeT5.p02 = 0.0007486f;
        polynomeT5.p30 = -1.186e-05f;
        polynomeT5.p21 = -5.668e-05f;
        polynomeT5.p12 = -0.0007641f;
        polynomeT5.p03 = -2.412e-05f;
        Turbine t5(std::make_unique<PolyDeg3>(polynomeT5));
        AddTurbine(std::move(t5));
    }
};
