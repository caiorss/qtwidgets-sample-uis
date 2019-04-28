#include <iostream>
#include <functional>

#include <QtWidgets>
#include <QApplication>

// ====== Simulate file: Counter.hpp ======= //

class Counter: public QObject
{
    Q_OBJECT
public:
    explicit Counter(int value);
    virtual ~Counter() = default;
public slots:
    void setValue(int value);
    void increment();
    void decrement();
    int  get() const;
signals:
    void valueChanged(int newValue);

private:
    int m_value;
};


// ====== Simulate file: Counter.cpp ======= //

Counter::Counter(int value)
    : m_value(value)
{
}

void Counter::increment()
{
    m_value++;
    emit valueChanged(m_value);
}

void Counter::decrement()
{
    m_value--;
    emit valueChanged(m_value);
}

void Counter::setValue(int value)
{
    if(value == m_value)
        return;
    m_value = value;
    // Trigger Event - call callback (SLOT)
    emit valueChanged(value);
}

int Counter::get() const
{
    return m_value;
}


class Observable: public QObject
{
    Q_OBJECT
public:
    Observable(){}

    virtual ~Observable() = default;

    void subscribe(std::function<void ()> observer)
    {
        QObject::connect(this, &Observable::notified, observer);
    }

    void notify()
    {
        emit this->notified();
    }

signals:
    void notified();
};

// Required to make "Qt5's MOC" happy when the class
// declaration and implementation are in the same
// file.
// See: https://www.qtcentre.org/threads/28580-Why-does-qmake-moc-only-process-header-files
#include "sigslots.moc"


// ========= Simulate file: main.cpp ============//

int main(int argc, char** argv)
{
    QApplication app(argc, argv);


    Counter     counter{0};
    QPushButton btnIncrement("Increment A");
    QPushButton btnDecrement("Decrement A");
    QPushButton btnIncrementB("Increment B");
    QLCDNumber  disp;

    // Old QT Syntax with macros
    //-------------------------------------------
    // Button Is Sender and the Counter is the receiver
    // Pseudocode: button.OnClikck.Subscribe(counter.increment)
    QObject::connect(&btnIncrement, SIGNAL(clicked()), &counter, SLOT(increment()));


    QObject::connect(&counter, &Counter::valueChanged, &disp,
                     // Solve overload ambiguity resolution
                    (void (QLCDNumber::*) (int)) &QLCDNumber::display );


    // New QT syntax using lambda function
    QObject::connect(&btnDecrement,
                     &QPushButton::clicked,
                     [&]{ counter.decrement(); });

    // New QT syntax using std::bind
#if 0
    QObject::connect(&btnIncrementB,
                     &QPushButton::clicked,
                     std::bind(&Counter::increment, counter));
#endif

    // Attach logging observer function
    QObject::connect(&counter, &Counter::valueChanged, [&]{
        std::cout << "Counter value = " << counter.get() << std::endl;
    });

    QVBoxLayout vbox;
    vbox.addWidget(&btnIncrement);
    vbox.addWidget(&btnIncrementB);
    vbox.addWidget(&btnDecrement);
    vbox.addWidget(&disp);
    QMainWindow window;
    QWidget wdg;
    window.setCentralWidget(&wdg);
    window.centralWidget()->setLayout(&vbox);
    window.resize(400, 500);
    window.show();


    return app.exec();
}


