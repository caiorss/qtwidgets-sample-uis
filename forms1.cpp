#include <iostream>
#include <iomanip>
#include <functional>

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>

/** Load from from *.ui file */
auto LoadForm(QString filePath) -> QWidget*
{
    QUiLoader loader;
    QFile file(filePath);
    file.open(QFile::ReadOnly);
    QWidget* form = loader.load(&file, nullptr);
    file.close();
    return form;
}


double normal_pdf(double x);
double normal_cdf(double x);

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QString formPath = QCoreApplication::applicationDirPath() + "/form1.ui";
    std::cout << "FormPath = " << formPath.toStdString() << std::endl;

    QWidget* form = LoadForm(formPath);
    assert(form != nullptr);
    form->show();

    QLineEdit* entryK     = form->findChild<QLineEdit*>("entryK");
    QLineEdit* entryS     = form->findChild<QLineEdit*>("entryS");
    QLineEdit* entryT     = form->findChild<QLineEdit*>("entryT");
    QLineEdit* entrySigma = form->findChild<QLineEdit*>("entrySigma");
    QLineEdit* entryR     = form->findChild<QLineEdit*>("entryR");

    QPushButton* btnClose = form->findChild<QPushButton*>("btnClose");
    QPushButton* btnReset = form->findChild<QPushButton*>("btnReset");

    QObject::connect(btnClose, &QPushButton::clicked, []{ std::exit(0); });
    QObject::connect(btnReset, &QPushButton::clicked, []{
        QMessageBox::warning(nullptr, "WARNIG", "Error: not implemented");
    });

    QTextEdit* display    = form->findChild<QTextEdit*>("OutputDisplay");
    assert(display != nullptr);

    auto computePrice = [&]{
      int a = 1;
      double q = 0.0;

      double K   = entryK->text().toDouble();
      double S   = entryS->text().toDouble();
      double T   = entryT->text().toDouble();
      double sigma = entrySigma->text().toDouble() / 100.0;
      double r   = entryR->text().toDouble() / 100.0;
      double b   = r;

      double d1 = (log(S/K) + (b + sigma * sigma / 2.0) * T) / (sigma * sqrt(T));
      double d2 = d1 - sigma * std::sqrt(T);
      // Helper parameter
      double exp_brt = std::exp((b - r) * T);
      double exp_rt = std::exp(-r * T);
      double sqrt_T  = sqrt(T);
      // European option price at t = 0
      double V = a * S * exp_brt * normal_cdf(a * d1) - a * K * exp_rt * normal_cdf(a * d2);

      auto result = QString(
                  "<h1>European Call Option Price Parameters</h1>"
                  "<p> K - Strike Price = %1 </p>"
                  "<p> d1 = %2 </p>"
                  "<p> d2 = %3 </p>"
                  "<p> V  = %4 - Option Price or fair value </p>"
                  ).arg(K).arg(d1).arg(d2).arg(V);

      display->setText(result);

    };

    /** Test case: John. C. Hull
     * Call European option price
     * K = 50,
     * S0 = 50,
     * T = 0.5 ;
     * sigma (volatility) = 30% ;
     * r = 5%.
     * Expected option price V = 4.8174
     * computed with BlackScholes formula
     */
    entryK->setText("50");
    entryS->setText("50");
    entryT->setText("0.5");
    entrySigma->setText("30");
    entryR->setText("5");
    computePrice();

    QObject::connect(entryK, &QLineEdit::returnPressed, computePrice);
    QObject::connect(entryS, &QLineEdit::returnPressed, computePrice);
    QObject::connect(entryT, &QLineEdit::returnPressed, computePrice);
    QObject::connect(entryR, &QLineEdit::returnPressed, computePrice);
    QObject::connect(entrySigma, &QLineEdit::returnPressed, computePrice);

    #if 0
    display->append("<h1>Enter Strike Price</h1>");
    display->append("<p>Computing Greeks<p>");
    #endif

    return app.exec();
}


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

