#include <iostream>
#include <functional>
#include <optional>
#include <map>

#include <QtWidgets>
#include <QApplication>

class FormBuilder: public QMainWindow
{
private:
    std::map<QString, QWidget*> entries;
    QFormLayout* form = new QFormLayout;
    std::function<void ()> onFocusChange_hadler = [](){};
public:

    FormBuilder()
    {
        this->resize(400, 500);
        this->setWindowTitle("Form Builder APP");
        this->setCentralWidget(new QWidget);
        this->centralWidget()->setLayout(form);
    }
    auto addLabel(QString label) -> FormBuilder&
    {
        form->addRow(new QLabel(label));
        return *this;
    }
    auto addWidget(QString fieldName, QWidget* widget) -> FormBuilder&
    {
        entries[fieldName] = widget;
        form->addRow(widget);
        return *this;
    }

    auto addWidget(QString fieldName,
                   QString label,
                   QWidget* widget) -> FormBuilder&
    {
        entries[fieldName] = widget;
        form->addRow(label, widget);
        return *this;
    }

    auto addLineEntry(QString fieldName, QString label) -> FormBuilder&
    {
        QLineEdit* pEntry = new QLineEdit();
        static auto handler = [&]{
            this->focusNextChild();
            this->onFocusChange_hadler();
            std::cout << " Focus changed " << std::endl;
         };
        QObject::connect(pEntry, &QLineEdit::returnPressed, handler);
        return this->addWidget(fieldName, label, pEntry);
    }

    template<typename T>
    auto findWidget(QString fieldName) -> T*
    {
        auto it = entries.find(fieldName);
        if(it == entries.end())
            return nullptr;
        return qobject_cast<T*>(it->second);
    }

    auto getInputAsText(QString fieldName) -> QString
    {
        QLineEdit* entry = this->findWidget<QLineEdit>(fieldName);
        if(entry == nullptr)
            throw std::runtime_error("Error: line edit not found");
        return entry->text();
    }

    auto getInputAsDouble(QString name) -> double
    {
        return this->getInputAsText(name).toDouble();
    }

    auto getInputAsInt(QString name) -> int
    {
        return this->getInputAsText(name).toInt();
    }

    auto onFocusChange(std::function<void ()> handler) -> void
    {
        this->onFocusChange_hadler = handler;
    }
};

class TableDisplay: public QTableWidget
{
public:
    int currentEntry = 0;
    struct Entry
    {
      QString name;
      QString description;
      int row;
    };

    std::map<QString, Entry> entries;

    TableDisplay()
    {
        //this->setRowCount(rows);
        this->setColumnCount(3);
        this->setShowGrid(false);
        this->horizontalHeader()->hide();
        this->verticalHeader()->hide();
        // this->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
        this->setEditTriggers(QAbstractItemView::NoEditTriggers);

        //this->verticalHeader()->resizeSection(2, QHeaderView::AdjustToContents);
        //this->horizontalHeader()->resizeSection(2, QHeaderView::ExpandingState);
    }
    void AddEntry(QString name, QString description = "")
    {
        entries[name] = Entry{name, description, currentEntry};
        this->insertRow(currentEntry);
        this->setItem(currentEntry, 0, new QTableWidgetItem(name));
        this->setItem(currentEntry, 2, new QTableWidgetItem(description));
        this->resizeColumnToContents(2);
        currentEntry++;
    }
    void SetEntry(QString name, double value)
    {
        auto it = entries.find(name);
        if(it == entries.end())
            return;
        int row = it->second.row;
        this->setItem(row, 1, new QTableWidgetItem(tr("%1").arg(value)));
    }
};


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



int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);

    FormBuilder form;
    form.resize(500, 400);
    form.addLabel("Option Data");
    form.addLineEntry("entryK", "K - Srike Price");
    form.addLineEntry("entryS", "S - Asset Price");
    form.addLineEntry("entryT", "T - Time to maturity in years");
    form.addLineEntry("entrySigma", "Volatility (sigma) in %");
    form.addLineEntry("entryR", "r - Interest rate in %");
    form.addWidget("btnCalc", "", new QPushButton("Submit"));

    // QTableModel<double>* m  = new QTableModel<double>;
    //QTableView*  tbl = new QTableView;
    //tbl->setModel(m);
    TableDisplay* tbl = new TableDisplay(9, 5);
    tbl->AddEntry("Vcall", "Call European option price or fair value at t = 0");
    tbl->AddEntry("Vput",  "Call European option price or fair value at t = 0");
    tbl->AddEntry("d1");
    tbl->AddEntry("d2");

    form.addWidget("table", tbl);

#if 0
    QPushButton* btnCalc = form.findWidget<QPushButton>("btnCalc");
    QObject::connect(btnCalc, &QPushButton::clicked, [&]
    {
    });

#endif


    form.onFocusChange([&]{
        double K = form.getInputAsInt("entryK");
        double S = form.getInputAsDouble("entryS");
        double T = form.getInputAsDouble("entryT");
        double sigma = form.getInputAsDouble("entrySigma") / 100.0;
        double r = form.getInputAsDouble("entryR") / 100.0;

        int a = 1;
        double q = 0.0;
        double b = r;

        double d1 = (log(S/K) + (b + sigma * sigma / 2.0) * T) / (sigma * sqrt(T));
        double d2 = d1 - sigma * std::sqrt(T);
        // Helper parameter
        double exp_brt = std::exp((b - r) * T);
        double exp_rt = std::exp(-r * T);
        double sqrt_T  = sqrt(T);
        // European option price at t = 0
        a = 1;
        double Vcall = a * S * exp_brt * normal_cdf(a * d1)
                - a * K * exp_rt * normal_cdf(a * d2);
        a = -1;
        double Vput = a * S * exp_brt * normal_cdf(a * d1)
                - a * K * exp_rt * normal_cdf(a * d2);

        tbl->SetEntry("d1", d1);
        tbl->SetEntry("d2", d2);
        tbl->SetEntry("Vcall", Vcall);
        tbl->SetEntry("Vput",  Vput);
    });

    form.show();

    qapp.exec();
    return 0;
}
