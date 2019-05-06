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
  virtual QString               Name() const = 0;
  virtual std::type_info const& Type() const = 0 ;
  virtual void                  Print(std::ostream& os) const = 0;

  virtual ~IProperty() = default;

  // Note: Adding Non virtual methods does not causes break the base class ABI
  // or binary compatibility with derived classes (fragile-base class problem).
  template<typename T>
  T Get() const {
       if(this->Type() != typeid(T)) throw std::bad_cast();
       return static_cast<TProperty<T> const*>(this)->Get();
  }

  template<typename T>
  void Set(T const& value) {
       if(this->Type() != typeid(T)) throw std::bad_cast();
       static_cast<TProperty<T>*>(this)->Set(value);
  }

  template<typename T>
  TProperty<T>& As()
  {
       if(this->Type() != typeid(T)) throw std::bad_cast();
       return *static_cast<TProperty<T>*>(this);
  }

  // Make class printable
  friend std::ostream& operator<<(std::ostream& os, IProperty const& rhs)
  {
      rhs.Print(os);
      return os;
  }
};

/** Class that encapsulate get/set properties
 *  @tparam - Type default constructible, copiable and equality-comparable
 */

template <typename T>
class TProperty: public IProperty
{
  using Action = std::function<void ()>;
  QString                 m_name;
  T                       m_value;
  std::type_info const*   m_tinfo;
  Action m_callback;
public:
  TProperty(QString name, T const& init = {}, Action callback = []{})
          : m_name(std::move(name))
          , m_value(init)
          , m_tinfo(&typeid(T))
          , m_callback(callback)
  { }

  ~TProperty() = default;

  QString Name() const
  {
       return m_name;
  }

  const std::type_info& Type() const
  {
       return *m_tinfo;
  }

  T Get() const
  {
       return m_value;
  }

  void Set(T const& value)
  {
       m_value = value;
       m_callback();
  }

  void Print(std::ostream& os) const
  {
       os << m_value;
  }
};


class InotifyPropertyChanged
{
public:
    virtual ~InotifyPropertyChanged() = default;

//    InotifyPropertyChanged(InotifyPropertyChanged const&) = delete;
//    InotifyPropertyChanged& operator=(InotifyPropertyChanged const&) = delete;

    virtual void Subscribe(PropertyChangedHandler hnd) = 0;
    virtual void NotifyObservers(QString propertyName) = 0;
    virtual void Clear() = 0;
    virtual size_t Count() const = 0;
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

protected:
    PropertyMap pmap;

    template<typename T>
    TProperty<T>* AddProperty(QString name, T const& init)
    {
        auto callback = [=]{ this->NotifyObservers(name);  };
        auto ptr = std::make_unique<TProperty<T>>(name, init, callback);
        TProperty<T>* addr = ptr.get();
        this->pmap[name] = std::move(ptr);
        return addr;
    }
private:
    std::vector<PropertyChangedHandler> observers{};
};


class BLSFormula2: public PropertyChangedObserver
{
    using TProperty_ptr = TProperty<double>*;
    TProperty_ptr m_K, m_S, m_T, m_sigma, m_r;
    double d1, d2, Vcall, Vput;
public:

    BLSFormula2()
    {
        m_K     = this->AddProperty<double>("K", 0.0);
        m_S     = this->AddProperty<double>("S", 0.0);
        m_T     = this->AddProperty<double>("T", 0.0);
        m_sigma = this->AddProperty<double>("sigma", 0.0);
        m_r     = this->AddProperty<double>("r", 0.0);

        this->Subscribe([=](QString name){ this->Recalculate(); });
    }
    IProperty* K()     { return m_K; }
    IProperty* S()     { return m_S; }
    IProperty* T()     { return m_T; }
    IProperty* r()     { return m_r; }
    IProperty* sigma() { return m_T; }

    void Recalculate()
    {
        double K = m_K->Get();
        double S = m_S->Get();
        double T = m_K->Get();
        double sigma = m_sigma->Get();
        double r = m_r->Get();

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


    }


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
            entryK1->setText(QString::number(bls.property("K")->Get<double>()));
    });


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
