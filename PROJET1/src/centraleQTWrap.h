#ifndef TURBINEQT_H
#define TURBINEQT_H


#include <iostream>
#include <ostream>
#include<QObject>
#include<QAbstractListModel>
#include "Centrale.cpp"

class TurbineQTWrapper : public QObject {
    Q_OBJECT
    // Ces macros créent le pont entre C++ et QML
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(double min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(double max READ max WRITE setMax NOTIFY maxChanged)

public:
    explicit TurbineQTWrapper(Turbine * org_turb, QObject *parent = nullptr)
        : QObject(parent), real_turbine(org_turb) {}

    bool active() const { return !real_turbine->locked; }
    double min() const { return real_turbine->minDebit; }
    double max() const { return real_turbine->maxDebit; }

public slots: // Les slots sont appelables depuis QML
    void setActive(bool a) { if (real_turbine->locked == a)
        { real_turbine->locked = !a; emit activeChanged(); } }
    void setMin(double m) { if (real_turbine->minDebit != m)
        { real_turbine->minDebit = m; emit minChanged(); } }
    void setMax(double m) { if (real_turbine->maxDebit != m)
        { real_turbine->maxDebit = m; emit maxChanged(); } }

signals:
    void activeChanged();
    void minChanged();
    void maxChanged();

private :
    Turbine * real_turbine;
};


class CentraleQTWrapper : public QAbstractListModel {
    Q_OBJECT
public:

    // Ajout du NameRole
    enum Roles {
        ActiveRole = Qt::UserRole + 1,
        MinRole,
        MaxRole,
        NameRole
    };

    Q_INVOKABLE void calculerSynthese(double debitTotal, double hauteur) {
        std::cout <<"Hauteur amont :" << hauteur << std::endl;
        real_centrale->H_amont = static_cast<float>(hauteur);

        VecDebitPower distribution = real_centrale->CalculateDistributionAndPower(static_cast<float>(debitTotal));
        std::cout << "Distribution des débits et puissances :\n";
        for (size_t i = 0; i < distribution.size(); ++i) {
            std::cout << "Turbine " << (i + 1) << ": Debit = " << distribution[i].first << " m3/s, Puissance = " << distribution[i].second << " MW\n";
        }
        std::cout << std::endl;
    }

    explicit CentraleQTWrapper(QObject *parent = nullptr, Centrale *realCentrale = nullptr) : QAbstractListModel(parent) {
        real_centrale = realCentrale;
        for(auto& turbine : real_centrale->turbines) {
            m_turbines.append(new TurbineQTWrapper(&turbine, this));
        }
    }

    // Indispensable : fait le pont entre les noms QML et les enums C++
    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[ActiveRole] = "active";
        roles[MinRole] = "min";
        roles[MaxRole] = "max";
        roles[NameRole] = "name";
        return roles;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_turbines.size();
    }

    QVariant data(const QModelIndex &index, int role) const override {
        if (!index.isValid() || index.row() >= m_turbines.size()) return {};

        auto *t = m_turbines[index.row()];
        switch (role) {
        case ActiveRole: return t->active();
        case MinRole:    return t->min();
        case MaxRole:    return t->max();
        case NameRole:   return QString("Turbine %1").arg(index.row() + 1);
        }
        return {};
    }

    // Indispensable pour l'écriture (quand on change une valeur dans l'UI)
    bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if (!index.isValid() || index.row() >= m_turbines.size()) return false;

        auto *t = m_turbines[index.row()];
        bool changed = false;

        switch (role) {
        case ActiveRole: t->setActive(value.toBool()); changed = true; break;
        case MinRole:    t->setMin(value.toDouble());    changed = true; break;
        case MaxRole:    t->setMax(value.toDouble());    changed = true; break;
        }

        if (changed) {
            // On prévient l'UI que la donnée a changé
            emit dataChanged(index, index, {role});
            return true;
        }
        return false;
    }

private:
    QList<TurbineQTWrapper*> m_turbines;
    Centrale * real_centrale;
};

#endif // TURBINEQT_H
