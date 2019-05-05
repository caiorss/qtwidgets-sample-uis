#include <iostream>
#include <iomanip>
#include <functional>
#include <cassert>
#include <sstream>

#include <QtWidgets>
#include <QApplication>
#include <QSysInfo>


/** @brief Normal Probability Density Function (mean = 0) and standard deviation = 1  */
double normal_pdf(double x)
{
    return 1/(std::sqrt(2.0 * M_PI)) * std::exp(- x * x / 2);
}

/** @brief Cumulative normal distribution approximation  (with mean = 0 and standard deviation = 1)
 */
double normal_cdf(double d)
{
    constexpr double A [] =
    {
        0.31938153,
       -0.356563782,
        1.781477937,
       -1.821255978,
        1.330274429
    };
    constexpr double RSQRT2PI = 0.39894228040143267793994605993438;
    double K = 1.0 / (1.0 + 0.2316419 * std::fabs(d));
    double c = RSQRT2PI * std::exp(- 0.5 * d * d) *
            (K * (A[0] + K * (A[1] + K * (A[2] + K * (A[3] + K * A[4])))));
    if(d > 0) return 1.0 - c;
    return c;
}



class BLSFormula
{
public:
    using PropertyChangedHandler = std::function<void (QString)>;

    double K = 50.0, S = 50.0, T = 0.5, sigma = 0.30, r = 0.05;
    double Vcall, Vput, d1, d2;

    BLSFormula()
    {
        this->Recalculate();
    }

    std::vector<PropertyChangedHandler> observers{};

    void Subscribe(PropertyChangedHandler hnd)
    {
        observers.push_back(hnd);
    }

    void NotifyObservers(QString propertyName)
    {
        for(auto&& hnd: observers)
            hnd(propertyName);
    }

    void SetK(double value)
    {
        K = value;
        this->Recalculate();
        this->NotifyObservers("K");
    }
    void SetS(double value)
    {
        if(value == S) return;
        S = value;
        this->Recalculate();
        this->NotifyObservers("S");
    }
    void SetT(double value)
    {
        if(value == T) return;
        T = value;
        this->Recalculate();
        this->NotifyObservers("T");
    }
    void SetSigma(double value)
    {
        sigma = value;
        this->Recalculate();
        this->NotifyObservers("sigma");
    }
    void SetR(double value)
    {
        if(r == value) return;
        r =  value;
        this->Recalculate();
        this->NotifyObservers("R");
    }

    double GetD1()    const { return d1; }
    double GetD2()    const { return d2; }
    double GetVcall() const { return Vcall; }
    double GetVput()  const { return Vput; }

    void Recalculate()
    {
        int a = 1;
        double q = 0.0;
        double b   = r;

        d1 = (log(S/K) + (b + sigma * sigma / 2.0) * T) / (sigma * sqrt(T));
        d2 = d1 - sigma * std::sqrt(T);
        // Helper parameter
        double exp_brt = std::exp((b - r) * T);
        double exp_rt = std::exp(-r * T);
        double sqrt_T  = sqrt(T);

        a = 1;
        // European option price at t = 0
        Vcall = a * S * exp_brt * normal_cdf(a * d1) - a * K * exp_rt * normal_cdf(a * d2);

        a = -1;
        Vput = a * S * exp_brt * normal_cdf(a * d1) - a * K * exp_rt * normal_cdf(a * d2);
    }
};


int main(int argc, char** argv)
{
    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);

    QMainWindow wnd;
    QFormLayout* form = new QFormLayout(&wnd);
    wnd.resize(400, 500);
    wnd.show();
    wnd.setCentralWidget(new QWidget);
    wnd.centralWidget()->setLayout(form);

    QLineEdit* entryK1 = new QLineEdit();
    form->addRow("k Strike Price 1", entryK1);

    QLineEdit* entryK2 = new QLineEdit();
    form->addRow("k Strike Price 2", entryK2);


    QLabel* labelVcall = new QLabel("0.0");
    form->addRow("Vcall - Call Option Price = ", labelVcall);

    // Observable object
    BLSFormula obs;

    QObject::connect(entryK1, &QLineEdit::returnPressed, [&]
    {
       double value = entryK1->text().toDouble();
       obs.SetK(value);
       std::cout << " [INFO] User press return entryK1 " << std::endl;
    });

    obs.Subscribe([&](QString name)
    {
        if(name == "K") entryK1->setText(QString::number(obs.K));
    });

    obs.Subscribe([&](QString name)
    {
        if(name == "K") entryK2->setText(QString::number(obs.K));
    });

    QObject::connect(entryK2, &QLineEdit::returnPressed, [&]
    {
       double value = entryK2->text().toDouble();
       std::cout << " [INFO] User press return entryK1 " << std::endl;
       obs.SetK(value);
    });


    obs.Subscribe([&](QString name)
    {
        labelVcall->setText(QString::number(obs.Vcall));
    });


    return app.exec();
}
