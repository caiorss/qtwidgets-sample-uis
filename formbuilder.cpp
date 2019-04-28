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

    TableDisplay(size_t rows, size_t columns)
    {
        this->setRowCount(rows);
        this->setColumnCount(columns);
        this->setShowGrid(false);
        this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    void AddEntry(QString name, QString description = "")
    {
        entries[name] = Entry{name, description, currentEntry};
        this->setItem(currentEntry, 0, new QTableWidgetItem(name));
        this->setItem(currentEntry, 2, new QTableWidgetItem(description));
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

int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);

    FormBuilder form;
    form.addLabel("Option Data");
    form.addLineEntry("entryS", "S - Asset Price");
    form.addLineEntry("entryK", "K - Srike Price");
    form.addLineEntry("entryR", "r - Interest rate in %");
    form.addWidget("btnCalc", "", new QPushButton("Submit"));

    QTableModel<double>* m  = new QTableModel<double>;
    QTableView*  tbl = new QTableView;
    tbl->setModel(m);


    form.addWidget("table", tbl);


#if 0
    QPushButton* btnCalc = form.findWidget<QPushButton>("btnCalc");
    QObject::connect(btnCalc, &QPushButton::clicked, [&]
    {
    });
#endif
    form.onFocusChange([&]{
        double S = form.getInputAsDouble("entryS");
        double K = form.getInputAsInt("entryK");
        std::cout << "S = " << S << " - " << " K = " << K << std::endl;
    });

    form.show();

    qapp.exec();
    return 0;
}
