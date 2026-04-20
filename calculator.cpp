#include "calculator.h"

int BigNumber::precision = 25;
int Calculator::m_precision = 25;

void BigNumber::fromString(const QString &value)
{
    QString v = value.simplified();
    isNegative = false;
    if (v.startsWith('-')) {
        isNegative = true;
        v.remove(0, 1);
    }
    v.replace(',', '.');
    if (!v.contains('.')) v += ".0";
    num = v;
}

QString BigNumber::toString() const
{
    QString s = num;
    if (isNegative && s != "0") s.prepend('-');
    return s;
}

static void alignFrac(const BigNumber &a, const BigNumber &b,
                      QString &n1, QString &n2, int &common)
{
    int da = a.num.contains('.') ? a.num.size() - a.num.indexOf('.') - 1 : 0;
    int db = b.num.contains('.') ? b.num.size() - b.num.indexOf('.') - 1 : 0;
    common = qMax(da, db);
    n1 = a.num; n1.remove('.'); n1 += QString(common - da, '0');
    n2 = b.num; n2.remove('.'); n2 += QString(common - db, '0');
}

// сложение
BigNumber BigNumber::add(const BigNumber &a, const BigNumber &b)
{
    QString n1, n2; int common;
    alignFrac(a, b, n1, n2, common);
    QString sum = addStrings(n1, n2);
    if (common) sum.insert(sum.size() - common, '.');
    return BigNumber(sum);
}

// вычитание
BigNumber BigNumber::sub(const BigNumber &a, const BigNumber &b)
{
    // если знаки разные, это сложение: a - (-b) = a + b
    if (a.isNegative != b.isNegative) {
        BigNumber tempB = b;
        tempB.isNegative = !tempB.isNegative; // меняем знак второго числа
        return add(a, tempB);
    }

    int cmp = compare(a.num, b.num);

    // определяем, какое число по модулю больше
    bool isResultNegative = false;
    QString n1, n2;

    if (cmp >= 0) {
        // |a| >= |b|. вычитаем из большего меньшее.
        n1 = a.num;
        n2 = b.num;
        // знак результата совпадает со знаком "a"
        isResultNegative = a.isNegative;
    } else {
        // |a| < |b|. вычитаем из большего меньшее и ставим минус.
        n1 = b.num;
        n2 = a.num;
        // знак результата противоположен знаку "a"
        isResultNegative = !a.isNegative;
    }

    // выравниваем дробные части для вычитания
    int common;
    alignFrac(BigNumber(n1), BigNumber(n2), n1, n2, common); // создаем временные объекты для выравнивания

    // выполняем вычитание строк (уже без учета знаков)
    QString diff = subStrings(n1, n2);

    BigNumber r;
    r.num = diff;

    // вставляем десятичную точку обратно, если она была
    if (common) {
        r.num.insert(r.num.size() - common, '.');
    }

    // устанавливаем итоговый знак
    r.isNegative = isResultNegative;

    return r;
}

// умножение
BigNumber BigNumber::mul(const BigNumber &a, const BigNumber &b)
{
    // сколько цифр после запятой у каждого множителя
    int da = a.num.contains('.') ? a.num.size() - a.num.indexOf('.') - 1 : 0;
    int db = b.num.contains('.') ? b.num.size() - b.num.indexOf('.') - 1 : 0;

    // убираем точки, но НЕ трогаем ведущие нули
    QString n1 = a.num;
    QString n2 = b.num;
    n1.remove('.');
    n2.remove('.');

    // перемножаем столбиком (внутри никаких обрезок)
    QString prod = mulStrings(n1, n2);   // вернёт строку с возможными ведущими нулями

    // ставим точку
    int dotPos = prod.size() - (da + db);
    if (da + db > 0) {
        if (dotPos <= 0) {               // целой части не хватило
            prod = QString(-dotPos + 1, '0') + prod;
            dotPos = 1;
        }
        prod.insert(dotPos, '.');
    }

    // теперь можно убрать ведущие и хвостовые нули
    prod = prod.replace(QRegularExpression("^0+(?!\\.)"), "")
               .replace(QRegularExpression("(\\.0+)?$"), "");
    if (prod.isEmpty() || prod == ".") prod = "0";
    if (prod.startsWith('.')) prod.prepend('0');

    BigNumber r;
    r.num  = prod;
    r.isNegative = a.isNegative ^ b.isNegative;
    return r;
}

// деление
BigNumber BigNumber::div(const BigNumber &a, const BigNumber &b)
{
    if (b.num == "0" || b.num == "0.0" || b.num == ".0")
        throw "Деление на ноль";

    // количество знаков после запятой у исходных чисел
    int da = a.num.contains('.') ? a.num.size() - a.num.indexOf('.') - 1 : 0;
    int db = b.num.contains('.') ? b.num.size() - b.num.indexOf('.') - 1 : 0;

    // убираем точки и ведущие нули
    QString na = a.num;
    QString nb = b.num;
    na.remove('.');
    nb.remove('.');
    na = na.replace(QRegularExpression("^0+(?!$)"), "");
    nb = nb.replace(QRegularExpression("^0+(?!$)"), "");
    if (na.isEmpty()) na = "0";
    if (nb.isEmpty()) nb = "0";

    // делим с нужной точностью
    const int scale = BigNumber::precision;
    QString dividend = na + QString(scale, '0');
    QString divisor  = nb;
    QString quot     = divStrings(dividend, divisor);

    // расставляем точку
    int pointPos = quot.size() - scale - (da - db);
    BigNumber r;
    if (pointPos <= 0) {
        int zerosToPrepend = qAbs(pointPos) + 1;
        quot.prepend(QString(zerosToPrepend, '0'));
        pointPos = 0;
        quot.insert(1, '.');
        r.num = quot;
        r.num.replace(QRegularExpression("^0+(?!\\.)"), "");
        if (r.num.isEmpty() || r.num == ".") r.num = "0";
        else if (r.num.startsWith('.')) r.num.prepend('0');
    } else {
        quot.insert(pointPos, '.');
        r.num = quot;
        r.num.replace(QRegularExpression("^0+(?!\\.)"), "");
        if (r.num.startsWith('.')) r.num.prepend('0');
    }

    // знак и выход
    r.isNegative = a.isNegative ^ b.isNegative;
    return r;
}






// строковый комбайн

QString BigNumber::addStrings(const QString &a, const QString &b)
{
    QString r; int carry = 0;
    for (int i = 0; i < qMax(a.size(), b.size()) || carry; ++i) {
        int da = i < a.size() ? a[a.size() - 1 - i].digitValue() : 0;
        int db = i < b.size() ? b[b.size() - 1 - i].digitValue() : 0;
        int s = da + db + carry;
        carry = s / 10;
        r.prepend(QString::number(s % 10));
    }
    return r;
}

QString BigNumber::subStrings(const QString &a, const QString &b)
{
    QString r; int borrow = 0;
    for (int i = 0; i < a.size(); ++i) {
        int da = a[a.size() - 1 - i].digitValue() - borrow;
        int db = i < b.size() ? b[b.size() - 1 - i].digitValue() : 0;
        if (da < db) { da += 10; borrow = 1; } else borrow = 0;
        r.prepend(QString::number(da - db));
    }
    return r.replace(QRegularExpression("^0+"), "");
}

QString BigNumber::mulStrings(const QString &a, const QString &b)
{
    // быстрая проверка на ноль
    QString a_ = a;
    QString b_ = b;
    a_.remove('.');
    b_.remove('.');
    if (a_.replace(QRegularExpression("^0+"), "").isEmpty()) a_ = "0";
    if (b_.replace(QRegularExpression("^0+"), "").isEmpty()) b_ = "0";
    if (a_ == "0" || b_ == "0") return "0";

    // классическое умножение столбиком
    QString res(a.size() + b.size(), '0');
    for (int i = a.size() - 1; i >= 0; --i) {
        if (a[i] == '.') continue;
        int carry = 0;
        for (int j = b.size() - 1; j >= 0; --j) {
            if (b[j] == '.') continue;
            int idx = i + j + 1;
            int v   = a[i].digitValue() * b[j].digitValue()
                    + res[idx].digitValue() + carry;
            carry = v / 10;
            res[idx] = QChar('0' + v % 10);
        }
        if (carry) res[i] = QChar('0' + carry);
    }
    // возвращаем БЕЗ обрезки ведущих нулей
    return res;
}

QString BigNumber::divStrings(const QString &a, const QString &b)
{
    if (b == "0") throw "Деление на ноль";

    const int scale = BigNumber::precision;   // сколько знаков после запятой

    // домножаем делимое сразу на 10^scale
    QString dividend = a + QString(scale, '0');
    QString divisor  = b;

    QString result;
    QString remainder;

    // целая часть
    for (QChar ch : dividend) // теперь в dividend уже scale «лишних» нулей
    {
        remainder += ch;
        remainder = remainder.replace(QRegularExpression("^0+(?!$)"), "");
        if (remainder.isEmpty()) remainder = "0";

        int digit = 0;
        for (int d = 9; d >= 0; --d)
        {
            QString prod = mulStrings(divisor, QString::number(d));
            if (compare(prod, remainder) <= 0)
            {
                digit = d;
                break;
            }
        }
        result += QString::number(digit);
        remainder = subStrings(remainder,
                               mulStrings(divisor, QString::number(digit)));
        if (remainder == "0") remainder.clear();
    }

    // устанавливаем точку
    int pointPos = result.size() - scale;
    if (pointPos < 0)                       // целой части не хватило
    {
        result = QString(-pointPos, '0') + result;
        pointPos = 0;
    }
    result.insert(pointPos, '.');

    // убираем лишнее
    result = result.replace(QRegularExpression("\\.0+$"), "");
    if (result.endsWith('.')) result.chop(1);
    if (result.isEmpty() || result == "-") result = "0";
    if (result.startsWith('.')) result = '0' + result;

    return result;
}

int BigNumber::compare(const QString &a, const QString &b)
{
    // создаем копии строк, чтобы не менять оригиналы
    QString a_trim = a;
    QString b_trim = b;

    // убираем ведущие нули
    a_trim = a_trim.replace(QRegularExpression("^0+"), "");
    b_trim = b_trim.replace(QRegularExpression("^0+"), "");

    // если строка стала пустой после удаления нулей, это значит "0"
    if (a_trim.isEmpty()) a_trim = "0";
    if (b_trim.isEmpty()) b_trim = "0";

    // сравниваем длину строк (число с большим кол-вом цифр — больше)
    if (a_trim.length() > b_trim.length()) return 1;
    if (a_trim.length() < b_trim.length()) return -1;

    // если длина одинаковая, сравниваем посимвольно
    return a_trim.compare(b_trim);
}

BigNumber BigNumber::percent(const BigNumber &a)
{
    static const BigNumber HUNDRED("100");
    return div(a, HUNDRED);
}

BigNumber BigNumber::negate(const BigNumber &a)
{
    BigNumber r(a);
    r.isNegative = !r.isNegative;
    return r;
}






// калькулятор

Calculator::Calculator(QObject *parent) : QObject(parent) {}

QString Calculator::calculate(const QString &expr) {
    try {
        auto tokens = tokenize(expr);
        auto postfix = toPostfix(tokens);
        BigNumber res = evaluatePostfix(postfix);
        return formatResult(res);
    } catch (const QString &e) {
        return e;
    } catch (const char *e) {
        return QString(e);
    }
}

// токенайзер
QList<QString> Calculator::tokenize(const QString &expr) {
    QList<QString> tok;
    int i = 0;
    bool mayBeUnary = true; // следующий минус может быть унарным

    while (i < expr.length()) {
        QChar ch = expr[i];

        if (ch == QChar(0x2212)) ch = '-';
        else if (ch == QChar(0x00F7)) ch = '/';
        else if (ch == QChar(0x00D7)) ch = '*';
        else if (ch == QChar(0x00B1)) ch = QChar(0x00B1);

        // пропускаем пробелы
        if (ch.isSpace()) {
            i++;
            continue;
        }

        // числа
        if (ch.isDigit() || ch == '.') {
            QString num;
            int dotCount = 0; // счётчик точек для проверки корректности

            // собираем число (цифры и одна точка)
            while (i < expr.length() && (expr[i].isDigit() || expr[i] == '.')) {
                if (expr[i] == '.') {
                    dotCount++;
                    if (dotCount > 1) {
                        throw QString("Неверный ввод дробной части: в '%1'").arg(expr.mid(i - num.size(), num.size() + 1));
                    }
                }
                num.append(expr[i++]);
            }

            // проверка: не более 25 знаков после запятой
            int dotPos = num.indexOf('.');
            if (dotPos != -1) {
                int fracLength = num.size() - dotPos - 1;
                if (fracLength > BigNumber::precision) {
                    throw QString("Превышен лимит знаков: макс %1").arg(BigNumber::precision).arg(num);
                }
            }

            tok.append(num);
            mayBeUnary = false;
            continue;
        }

        // скобки
        if (ch == '(' || ch == ')') {
            tok.append(QString(ch));
            i++;
            mayBeUnary = (ch == '('); // после '(' может быть унарный минус
            continue;
        }

        // операторы +, -, *, /, %, ±
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == QChar(0x00B1)) {
            // унарные операторы: %, ±, и минус в начале или после '('/оператора
            if ((ch == '%' || ch == QChar(0x00B1)) || (ch == '-' && mayBeUnary)) {
                tok.append(ch == '-' ? "u-" : QString(ch));
                i++;
                mayBeUnary = true; // после унарного снова может быть унарный
                continue;
            }

            // бинарные операторы
            tok.append(QString(ch));
            i++;
            mayBeUnary = true;
            continue;
        }

        // "+/-" -> "±"
        if (ch == QChar(0x207A) && i + 2 < expr.length()) {
            if (expr[i + 1] == QChar(0x2215) && expr[i + 2] == QChar(0x208B)) {
                // найдено "⁺+/-", заменяем на "±"
                tok.append(QString(QChar(0x00B1)));
                i += 3; // пропускаем три символа
                mayBeUnary = true;
                continue;
            }
        }

        //throw QString("Unknown symbol: '%1'").arg(ch);
        throw QString("Недопустимый символ в выражении: '%1'").arg(ch);
    }

    return tok;
}

QQueue<QString> Calculator::toPostfix(const QList<QString> &tok)
{
    QQueue<QString> out; QStack<QString> st;
    QHash<QString,int> pr{{"+",1},{"-",1},{"*",2},{"/",2},{"u-",3}};
    for (auto &t : tok) {
        if (t.contains(QRegularExpression("^\\d"))) out.enqueue(t);
        else if (t == "(") st.push(t);
        else if (t == ")") {
            while (!st.isEmpty() && st.top() != "(") out.enqueue(st.pop());
            if (!st.isEmpty()) st.pop();
        } else {
            while (!st.isEmpty() && st.top() != "(" && pr[st.top()] >= pr[t])
                out.enqueue(st.pop());
            st.push(t);
        }
    }
    while (!st.isEmpty()) out.enqueue(st.pop());
    return out;
}

BigNumber Calculator::evaluatePostfix(const QQueue<QString> &post)
{
    QStack<BigNumber> st;
    QQueue<QString> q(post);

    while (!q.isEmpty()) {
        QString t = q.dequeue();

        // число (включая отрицательные)
        if (t.contains(QRegularExpression("^-?\\d+\\.?\\d*"))) {
            st.push(BigNumber(t));
            continue;
        }

        // унарные операторы
        if (t == "u-" || t == "%" || t == QString(QChar(0x00B1))) {
            if (st.isEmpty()) throw "Мало операндов для вычисления";
            BigNumber a = st.pop();
            if (t == "u-" || t == QString(QChar(0x00B1))) {
                st.push(BigNumber::negate(a)); // смена знака
            } else if (t == "%") {
                st.push(BigNumber::percent(a)); // умножить на 0,01
            }
            continue;
        }

        // бинарные операторы
        if (st.size() < 2) throw "Мало операндов";
        BigNumber b = st.pop();
        BigNumber a = st.pop();

        BigNumber r;
        if      (t == "+") r = BigNumber::add(a, b);
        else if (t == "-") r = BigNumber::sub(a, b);
        else if (t == "*") r = BigNumber::mul(a, b);
        else if (t == "/") r = BigNumber::div(a, b);
        else throw QString("Unknown operator: %1").arg(t);

        st.push(r);
    }
    return st.pop();
}

QString Calculator::formatResult(const BigNumber &num) const
{
    QString s = num.toString();

    // если это целое число, просто убираем ведущие нули и возвращаем
    if (!s.contains('.')) {
        return s.replace(QRegularExpression("^0+(?!$)"), "");
    }

    // разбиваем на целую и дробную части для обработки
    QStringList parts = s.split('.');
    QString integerPart = parts[0];
    QString fractionPart = parts[1];

    // обрабатываем целую часть (удаляем ведущие нули)
    integerPart = integerPart.replace(QRegularExpression("^0+(?!$)"), "");
    if (integerPart.isEmpty()) integerPart = "0";

    // обрезаем дробную часть до максимальной точности (25 знаков)
    // и удаляем хвостовые нули.
    fractionPart = fractionPart.left(BigNumber::precision); // оставляем максимум 25 знаков

    // удаляем нули в конце дробной части
    fractionPart = fractionPart.replace(QRegularExpression("0+$"), "");

    // собираем число обратно
    s = integerPart;
    if (!fractionPart.isEmpty()) {
        s += '.' + fractionPart;
    }

    // красивый минус для qml
    s.replace('-', QChar(0x2212));

    return s;
}

