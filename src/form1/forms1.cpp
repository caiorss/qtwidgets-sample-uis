#include <iostream>
#include <iomanip>
#include <functional>
#include <cassert>

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>

#define DISP_EXPR(expr) \
  std::cout << " [INFO] " << #expr << " = " << (expr) << std::endl

#define DISP_VALUE(msg, expr) \
  std::cout << " [INFO] " << (msg) << " = " << (expr) << std::endl


/** Load from from *.ui file */
auto LoadForm(QString filePath) -> QWidget*
{
    QUiLoader loader;
    QFile file(filePath);
    // file.open(QFile::ReadOnly);
    QWidget* form = loader.load(&file, nullptr);
    file.close();
    return form;
}

/** Makes QString printable */
auto operator<<(std::ostream& os, QString const& str) -> std::ostream&
{
    return os << str.toStdString();
}

double normal_pdf(double x);
double normal_cdf(double x);


bool CreateLinuxDesktopShortcut(
        QString const& strName,
        QString const& applicationPath,
        QString const& destDirectory,
        QString const& icon,
        QString const& comment = ""
        )
{
    QFile link(destDirectory + QDir::separator() + strName + ".desktop");
    if (link.open(QFile::WriteOnly | QFile::Truncate))
    {
    QTextStream out(&link);
    out.setCodec("UTF-8");
    out << "[Desktop Entry]" << endl
        << "Encoding = UTF-8" << endl
        << "Version  = 1.0" << endl
        << "Type     = Application" << endl
        << "Name     = " << strName << endl
        << "Comment  = " << comment << endl
        << "Icon     = " << icon    << endl
        << "Exec     = " << applicationPath << endl;
        return true;
    }
    return false;
}

class FormLoader: public QMainWindow
{
private:
    QString  formFile;
    QWidget* form;
public:
    FormLoader(QString path)
    {
        this->LoadForm(path);
        this->setCentralWidget(form);
        this->setWindowTitle(form->windowTitle());

        // Set Width and height
        this->resize(form->width(), form->height());

        // Center Window in the screen
        this->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                this->size(),
                qApp->desktop()->availableGeometry()
            )
        );
    }

    virtual ~FormLoader() = default;

    // Forbid copy, aka Deep Copy
    FormLoader(FormLoader const&) = delete;
    FormLoader& operator=(FormLoader const&) = delete;

    void LoadForm(QString filePath)
    {
        QUiLoader loader;
        formFile = filePath;
        QFile file(filePath);
       // file.open(QFile::ReadOnly);
        form = loader.load(&file, nullptr);
        assert(form != nullptr);
        file.close();
    }

    QWidget* GetForm() { return form;  }
};

class EuropeanOptionsForm: public FormLoader
{
private:
    // Extract children widgets from from file
    QLineEdit*   entryK;
    QLineEdit*   entryS;
    QLineEdit*   entryT;
    QLineEdit*   entrySigma;
    QLineEdit*   entryR;
    QPushButton* btnClose;
    QPushButton* btnReset;
    QPushButton* btnShortcut;
    QTextEdit*   display;
public:

    EuropeanOptionsForm()
        : FormLoader(":/assets/form1.ui")
        //FormLoader(QCoreApplication::applicationDirPath() + "/form1.ui")
    {
        QWidget* form = this->FormLoader::GetForm();
        entryK     = form->findChild<QLineEdit*>("entryK");
        entryS     = form->findChild<QLineEdit*>("entryS");
        entryT     = form->findChild<QLineEdit*>("entryT");
        entrySigma = form->findChild<QLineEdit*>("entrySigma");
        entryR     = form->findChild<QLineEdit*>("entryR");
        btnClose   = form->findChild<QPushButton*>("btnClose");
        btnReset   = form->findChild<QPushButton*>("btnReset");
        btnShortcut = form->findChild<QPushButton*>("btnShortcut");
        display    = form->findChild<QTextEdit*>("OutputDisplay");

        QObject::connect(btnClose, &QPushButton::clicked, []{
            // std::exit(0);
            QApplication::exit(0);
        });

        QObject::connect(btnReset, &QPushButton::clicked, [this]{
            ///QMessageBox::warning(this, "WARNIG", "Error: not implemented");
            this->Reset();
            this->Recalculate();
        });

        // Set initial GUI State
        this->Reset();
        // Update UI caculations showing option price
        this->Recalculate();

        auto update = [&]{
            // Update GUI calculations and results display
           this->Recalculate();
           // Set UI focus on next form
           this->focusNextChild();
        };

        // Restore window size and position
        this->RestoreAppState();

        // Save application state when the main Window is destroyed
        QObject::connect(this, &QMainWindow::destroyed, [this]{
           this->SaveAppState();
        });

        QObject::connect(entryK, &QLineEdit::returnPressed, update);
        QObject::connect(entryS, &QLineEdit::returnPressed, update);
        QObject::connect(entryT, &QLineEdit::returnPressed, update);
        QObject::connect(entryR, &QLineEdit::returnPressed, update);
        QObject::connect(entrySigma, &QLineEdit::returnPressed, update);

        QObject::connect(btnShortcut, &QPushButton::clicked, [&]{
            QString imagePath = QCoreApplication::applicationDirPath() + "/icon.png";
            // Extract resource file to disk to the application's directory.
            QFile::copy(":/images/appicon.png", imagePath);

            QString path = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0];

            // Split variables for better debugging
            auto appName = QCoreApplication::applicationName();
            auto appPath = QCoreApplication::applicationFilePath();
            ::CreateLinuxDesktopShortcut(
                        appName,
                        appPath,
                        path,
                        imagePath,
                        "Application for computing European options"
                        );
        });

    }

    // Set strike price
    void SetK(double value)
    {
        qDebug() << " [INFO] Set strike price";
        this->entryK->setText(QString::number(value));
        this->Recalculate();
    }

    void SetS(double value)
    {
        this->entryS->setText(QString::number(value));
        this->Recalculate();
    }

    void Recalculate()
    {
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
                  "<h2>European Call Option Price Parameters</h2>"
                  " <table>"
                  "<caption>Results of Black-Scholes formulas for European options</caption>"
                  " <tr>"
                  "  <th>Variable</th>"
                  "  <th>Value</th>"
                  "  <th>Description</th>"
                  " </tr>"
                  " <tr>"
                  "  <td>K</td>"
                  "  <td>%1</td>"
                  "  <td>Strike Price</td>"
                  " </tr>"
                  " <tr>"
                  "  <td>V</td>"
                  "  <td>%2</td>"
                  "  <td>Option Price (BLS)</td>"
                  " </tr>"
                  " </table>"
                  ).arg(K, 5, 'F', 3).arg(V, 5, 'F', 3); //.arg(d1).arg(d2).arg(V);

      display->setText(result);

      // form->nextInFocusChain()->setFocus();
      // QApplication::focusWidget()->nextInFocusChain()->setFocus();
      // Set focus on next child widget

    };


    /** Reseut the UI State to default value from
     * Test case: Book - John. C. Hull - Options, Futures and Other Derivatives
     * Call European option price
     * K = 50,
     * S0 = 50,
     * T = 0.5 ;
     * sigma (volatility) = 30% ;
     * r = 5%.
     * Expected option price V = 4.8174
     * computed with BlackScholes formula
     */
    void Reset()
    {
        entryK->setText("50");
        entryS->setText("50");
        entryT->setText("0.5");
        entrySigma->setText("30");
        entryR->setText("5");

    }

    void SaveAppState() const
    {
        QSettings conf;
        // Save current window position
        conf.setValue("pos", this->pos());
        // Save current window size before closing
        conf.setValue("size", this->size());
        // Save current state
        conf.setValue("state", this->saveState());

        auto saveEntryState = [&](QString name, QLineEdit* entry){
            conf.setValue(name, entry->text().toDouble());
        };

        saveEntryState("entryK", this->entryK);
        saveEntryState("entryS", this->entryS);
        saveEntryState("entryT", this->entryT);
        saveEntryState("entrySigma", this->entrySigma);
        saveEntryState("entryR", this->entryR);

        qDebug() << " [INFO] Application state saved OK.";
    }

    void RestoreAppState()
    {
        QSettings conf;
        // Default window position
        constexpr int top = 200, left = 200;
        // Defailt window size
        constexpr int width = 600, height = 500;

        // Note: QPoint(top, left) is the default value for the setting
        // if the configuration is not defined.
        QPoint pos  = conf.value("pos", QPoint(top, left)).toPoint();

        QSize  size = conf.value("size",  QSize(width, height)).toSize();
        QByteArray state = conf.value("state", QByteArray()).toByteArray();
        this->restoreState(state);
        this->resize(size);
        this->move(pos);

        auto loadEntryState = [&]( QString    name,
                                   QLineEdit* entry,
                                   double     default_value )
        {
            double value = conf.value(name, default_value).toDouble();
            entry->setText(QString::number(value));
        };

        loadEntryState("entryK",     this->entryK,     50.0);
        loadEntryState("entryS",     this->entryS,     50.0);
        loadEntryState("entryT",     this->entryT,     30.0);
        loadEntryState("entrySigma", this->entrySigma, 30.0);
        loadEntryState("entryR",     this->entryR,     5.0);
        this->Recalculate();
    }

};


int main(int argc, char** argv)
{
    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setOrganizationName("UniqueOrganizatioName");
    app.setOrganizationDomain("myorganization.net.domain");
    app.setApplicationName("BLSCalculator");

    std::cout << " [INFO] Starting Application" << std::endl;

    // Useful for debugging
    std::cout << " [INFO] QTVersion = " << ::qVersion() << std::endl;

    DISP_VALUE("Process Identifier - PID",   QCoreApplication::applicationPid());
    DISP_VALUE("Application Name",           QCoreApplication::applicationName());
    DISP_VALUE("Application Directory Path", QCoreApplication::applicationDirPath());
    DISP_VALUE("Current Executable Path",    QCoreApplication::applicationFilePath());

    DISP_EXPR(QSysInfo::buildAbi());
    DISP_EXPR(QSysInfo::kernelType());
    DISP_EXPR(QSysInfo::kernelVersion());
    // DISP_EXPR(QSysInfo::machineUniqueId().toStdString());
    DISP_EXPR(QSysInfo::productType());
    DISP_EXPR(QSysInfo::productVersion());
    DISP_EXPR(QSysInfo::prettyProductName());


#if 1
    EuropeanOptionsForm form;
    form.setWindowIcon(QIcon(":/images/appicon.png"));
    form.showNormal();
#endif

    return app.exec();
}

// ======= Utility Functions ===========//

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

