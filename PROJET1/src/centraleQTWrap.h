#ifndef TURBINEQT_H
#define TURBINEQT_H


#include<QObject>
#include<QAbstractListModel>

class TurbineQTWrapper : public QObject {
    Q_OBJECT
    // Ces macros créent le pont entre C++ et QML
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(double min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(double max READ max WRITE setMax NOTIFY maxChanged)

public:
    explicit TurbineQTWrapper(double min = 0, double max = 160, QObject *parent = nullptr)
        : QObject(parent), m_active(true), m_min(min), m_max(max) {}

    bool active() const { return m_active; }
    double min() const { return m_min; }
    double max() const { return m_max; }

public slots: // Les slots sont appelables depuis QML
    void setActive(bool a) { if (m_active != a) { m_active = a; emit activeChanged(); } }
    void setMin(double m) { if (m_min != m) { m_min = m; emit minChanged(); } }
    void setMax(double m) { if (m_max != m) { m_max = m; emit maxChanged(); } }

signals:
    void activeChanged();
    void minChanged();
    void maxChanged();

private:
    bool m_active = true;
    double m_min = 0;
    double m_max = 160;
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
        qDebug() << "Calcul en cours...";
        qDebug() << "Débit:" << debitTotal << "m3/s | Hauteur:" << hauteur << "m";

        // C'est ici que vous mettriez votre algorithme complexe
        double puissanceTheorique = debitTotal * hauteur * 9.81 * 0.85; // rho * g * h * rendement

        qDebug() << "Puissance estimée :" << puissanceTheorique << "kW";
    }

    explicit CentraleQTWrapper(QObject *parent = nullptr) : QAbstractListModel(parent) {
        for(int i=0; i<5; ++i) m_turbines.append(new TurbineQTWrapper(0.0, 160.0, this));
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
};

#endif // TURBINEQT_H
