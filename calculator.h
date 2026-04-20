#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QObject>
#include <QString>
#include <QStack>
#include <QQueue>
#include <QRegularExpression>
#include <QDebug>
#include <cmath>
#include <QLocale>

class BigNumber {
public:
    QString num;
    bool isNegative = false;
    static int precision;

    explicit BigNumber(const QString &value = "0") { fromString(value); }

    void fromString(const QString &value);

    static BigNumber add(const BigNumber &a, const BigNumber &b);
    static BigNumber sub(const BigNumber &a, const BigNumber &b);
    static BigNumber mul(const BigNumber &a, const BigNumber &b);
    static BigNumber div(const BigNumber &a, const BigNumber &b);

    QString toString() const;

    BigNumber abs() const { BigNumber r(*this); r.isNegative = false; return r; }

    static BigNumber percent(const BigNumber &a);
    static BigNumber negate (const BigNumber &a);

private:
    static QString addStrings(const QString &a, const QString &b);
    static QString subStrings(const QString &a, const QString &b);
    static QString mulStrings(const QString &a, const QString &b);
    static QString divStrings(const QString &a, const QString &b);
    static int compare(const QString &a, const QString &b);
};

class Calculator : public QObject {
    Q_OBJECT
public:
    explicit Calculator(QObject *parent = nullptr);
    static int precision() { return m_precision; }

public slots:
    QString calculate(const QString &expression);

private:
    static int m_precision;
    QList<QString> tokenize(const QString &expr);
    QQueue<QString> toPostfix(const QList<QString> &tokens);
    BigNumber evaluatePostfix(const QQueue<QString> &postfix);

    QString formatResult(const BigNumber &num) const;

};

#endif // CALCULATOR_H
