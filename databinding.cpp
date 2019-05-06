#include <iostream>
#include <iomanip>
#include <functional>
#include <cassert>
#include <sstream>
#include <map>

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


using PropertyChangedHandler = std::function<void (QString)>;


template <typename T>
class TProperty;


class IProperty
{
public:
  virtual QString          Name() const  = 0;
  virtual QMetaType::Type  Type() const  = 0;
  virtual QVariant         Get()  const  = 0;
  virtual void             Set(QVariant) = 0 ;

  virtual ~IProperty() = default;
};

class PropertyValue: public IProperty
{
    const QString          m_name;
    const QMetaType::Type   m_type;
    QVariant               m_value;
    PropertyChangedHandler m_callback;
public:

    PropertyValue(QString                name,
                  QVariant               value,
                  PropertyChangedHandler callback):
        m_name(name)
      , m_type(static_cast<QMetaType::Type>(value.type()))
      , m_value(value)
      , m_callback(callback)
    { }

    QString   Name() const { return m_name;  }
    QMetaType::Type  Type() const  { return m_type; }

    QVariant  Get()  const
    {
        return m_value;
    }
    void Set(QVariant value)
    {
        m_value  = value;
        m_callback(m_name);
    }
};

using FGetter = std::function<QVariant ()>;
using FSetter = std::function<void (QVariant )>;


class PropertyComputed: public IProperty
{
public:
    PropertyComputed(QString name,
                     QMetaType::Type type,
                     FGetter getter,
                     FSetter setter
                     )
        : m_name(name), m_type(type), m_getter(getter), m_setter(setter)
    {
    }

    QString   Name() const  { return m_name;  }

    QMetaType::Type Type() const  { return m_type; }

    QVariant  Get()  const
    {
        return m_getter();
    }
    void Set(QVariant value)
    {
        m_setter(value);
    }
private:
    const QString   m_name;
    const QMetaType::Type m_type;
    FGetter m_getter;
    FSetter m_setter;
};



class InotifyPropertyChanged
{
public:
    virtual ~InotifyPropertyChanged() = default;

//    InotifyPropertyChanged(InotifyPropertyChanged const&) = delete;
//    InotifyPropertyChanged& operator=(InotifyPropertyChanged const&) = delete;

    virtual void     Subscribe(PropertyChangedHandler hnd) = 0;
    virtual void     NotifyObservers(QString propertyName) = 0;
    virtual void     Clear() = 0;
    virtual size_t   Count() const = 0;
    virtual QVariant GetProperty(QString name) = 0;
    virtual void     SetProperty(QString name, QVariant value) = 0;
};

class PropertyChangedObserver: public InotifyPropertyChanged
{
public:
    using PropertyMap = std::map<QString, std::unique_ptr<IProperty>>;

    virtual ~PropertyChangedObserver() = default;

    void Subscribe(PropertyChangedHandler hnd)
    {
        observers.push_back(hnd);
    }
    void NotifyObservers(QString propertyName)
    {
        for(auto&& hnd: observers)
            hnd(propertyName);
    }
    void Clear()
    {
        this->observers.clear();
    }

    size_t Count() const
    {
        return this->observers.size();
    }

    IProperty* property(QString name)
    {
        auto it = pmap.find(name);
        if(it == pmap.end())
            return nullptr;
        return it->second.get();
    }

    QVariant GetProperty(QString name)
    {
        auto it = pmap.find(name);
        if(it == pmap.end())
            return QVariant();
        return it->second->Get();
    }

    void SetProperty(QString name, QVariant value)
    {
        auto it = pmap.find(name);
        if(it == pmap.end())
            return;
        return it->second->Set(value);
    }


protected:
    PropertyMap pmap;

    IProperty* AddPropertyValue(QString name, QVariant value)
    {
        auto callback = [&](QString name){
            NotifyObservers(name);
        };
        auto ptr = std::make_unique<PropertyValue>(name, value, callback);
        IProperty* p = ptr.get();
        this->pmap[name] = std::move(ptr);
        return p;
    }

    // Add Computed property
    IProperty* AddProperty(QString name,
                           QVariant::Type type,
                           FGetter getter,
                           FSetter setter)
    {
        auto ptr = std::make_unique<PropertyComputed>(
                    name,
                    static_cast<QMetaType::Type>(type),
                    getter,
                    setter
                    );
        auto addr = ptr.get();
        this->pmap[name] = std::move(ptr);
        return addr;
    }


private:
    std::vector<PropertyChangedHandler> observers{};
};


class BLSFormula2: public PropertyChangedObserver
{
    using IProperty_p = IProperty*;
    IProperty_p m_K, m_S, m_T, m_sigma, m_r;
    double m_d1, m_d2, m_Vcall, m_Vput;
public:

    BLSFormula2()
    {
        m_K = AddPropertyValue("K", 50.0);
        m_S = AddPropertyValue("S", 50.0);
        m_T = AddPropertyValue("T", 0.5);
        m_r = AddPropertyValue("sigma", 0.30);
        m_r = AddPropertyValue("r", 0.05);
    }


#if 1
    IProperty* K()     { return m_K; }
    IProperty* S()     { return m_S; }
    IProperty* T()     { return m_T; }
    IProperty* r()     { return m_r; }
    IProperty* sigma() { return m_T; }

    void Recalculate()
    {
        double K     = m_K->Get().toDouble();
        double S     = m_S->Get().toDouble();
        double T     = m_K->Get().toDouble();
        double sigma = m_sigma->Get().toDouble();
        double r     = m_r->Get().toDouble();

        int a = 1;
        double q = 0.0;
        double b   = r;

        double d1 = (log(S/K) + (b + sigma * sigma / 2.0) * T) / (sigma * sqrt(T));
        double d2 = d1 - sigma * std::sqrt(T);
        // Helper parameter
        double exp_brt = std::exp((b - r) * T);
        double exp_rt = std::exp(-r * T);
        double sqrt_T  = sqrt(T);

        a = 1;
        // European option price at t = 0
        m_Vcall = a * S * exp_brt * normal_cdf(a * d1) - a * K * exp_rt * normal_cdf(a * d2);

        a = -1;
        m_Vput = a * S * exp_brt * normal_cdf(a * d1) - a * K * exp_rt * normal_cdf(a * d2);

        m_d1 = d1;
        m_d2 = d2;
    }

#endif

};


/**
 *   Source (Often an observable Object)
 *   Target (Often a QT Widget)
 *   Source property
 *   Target Property
 *   Target Event
 *---------------------------------------------*/

enum class BindingMode: std::uint32_t
{
    OneWay, TwoWays
};

struct Binding
{
  InotifyPropertyChanged* source;
  QString                 path;
  BindingMode             mode;
  Binding(InotifyPropertyChanged* source, QString path, BindingMode mode)
      : source(source), path(path), mode(mode){ }
};

void SetLineEditBinding(Binding const& binding, QLineEdit* edit);


#if 0
void Bind2WData_prototype1(  InotifyPropertyChanged* source
                           , QLineEdit* target
                           , QString source_property_name
                           )
{
    QObject::connect(target, &QLineEdit::returnPressed, [&]
    {
       double value = target->text().toDouble();
       obs.SetK(value);
       std::cout << " [INFO] User press return target " << std::endl;
    });

    source->Subscribe([=](QString name)
    {
        if(name == "K") target->setText(QString::number(obs.K));
    });
}
#endif


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

#if 2
    BLSFormula2 bls;
    bls.K()->Set(50.0);
    bls.S()->Set(50.0);
    bls.T()->Set(0.5);
    bls.T()->Set(0.30);
    bls.r()->Set(0.05);

    QObject::connect(entryK1, &QLineEdit::returnPressed, [&]
    {
       double value = entryK1->text().toDouble();
       bls.property("K")->Set(value);
       std::cout << " [INFO] User press return entryK1 " << std::endl;
    });

    bls.Subscribe([&](QString name)
    {
        if(name == "K")
            entryK1->setText(QString::number(bls.GetProperty("K").toDouble()));
    });
#endif

    // Observable object
    //BLSFormula obs;
    //Binding b1 = {&obs, "K", BindingMode::TwoWays};

#if 0
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
#endif

    return app.exec();
}
