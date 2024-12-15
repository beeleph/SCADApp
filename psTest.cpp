#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <QCoreApplication>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QVariant>
#include <QTimer>
//#include <QThread>

#include <QModbusTcpServer>

#include <array>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include <iostream>

using namespace std;
using namespace prometheus;

// разделить на хедер и спп.

void print(std::string str){
  cout << str << endl;
}
void print(float str){
  cout << std::to_string(str) << endl;
}

class Panel : public QObject {
    Q_OBJECT

  private:
    int modbusSlaveID = 0;
    string ip;
    float gammaDoze = 0.0;
    float neutronDoze = 0.0;
    //QString ip;
    QModbusTcpClient *modbus = nullptr;
    QModbusDataUnit *gammaNeutron = nullptr;
    //QModbusDataUnit *neutron = nullptr;
    QTimer *readLoopTimer;
    //QThread mainThread;

    Gauge *doze_gauge_neutron;
    Gauge *doze_gauge_gamma;
    double test_doze = 0;
    double test_doze2 = 0;
    

  private slots:
    void onReadReady(QModbusReply* reply);
    void loop_goBabe();;

  signals:
    void readFinished(QModbusReply* reply);

  public:
     float getGammadoze();

  Panel(int modbusSlaveID, string ip, string name, std::shared_ptr<prometheus::Registry> reg) {
      // modbus stuff
    readLoopTimer = new QTimer(this);
    this->modbusSlaveID = modbusSlaveID;
    this->ip = ip;
    modbus = new QModbusTcpClient();
    QObject::connect(modbus, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        cout << modbus->errorString().toStdString() << std::endl;
    });
    if (!modbus) {
        cout << "Could not create Modbus master." << std::endl;
    }
    modbus->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QString::fromStdString(ip));
    modbus->setConnectionParameter(QModbusDevice::NetworkPortParameter, "502");
    modbus->setTimeout(3000);
    if (!modbus->connectDevice()) {
      std::cout << "Connect failed: " << modbus->errorString().toStdString() << std::endl;
    }
    gammaNeutron = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 4); // CONFIRM THAT!
    //neutron = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 2, 2);// CONFIRM THAT!
    QObject::connect(this, SIGNAL(readFinished(QModbusReply*)), this, SLOT(onReadReady(QModbusReply*)));
    std::cout << "Versioning, name, contacts, date" << std::endl;
    
    QObject::connect(readLoopTimer, SIGNAL(timeout()), this, SLOT(loop_goBabe()));
    //moveToThread(&mainThread);
    //readLoopTimer->moveToThread(&mainThread);
    //QObject::connect(&mainThread, SIGNAL(started()), this, SLOT(threadStarted()));
    readLoopTimer->start(1000);
      // prometheus stuff
    auto& doze_gauge = BuildGauge() 
                            .Name("Doze_post_" + name)
                            .Help("Doze from post from neutron and gamma detector")
                            .Register(*reg);

    auto& nd = doze_gauge.Add({{"Neutron_doze", "mZvt"}});
    auto& gd = doze_gauge.Add({{"Gamma_doze", "mZvt"}});
    doze_gauge_neutron = &nd;
    doze_gauge_gamma = &gd;
  }
};

void Panel::loop_goBabe(){
  std::cout << " loop go babe go!! " << std::endl;
  if (auto *replyOne = modbus->sendReadRequest(*gammaNeutron, modbusSlaveID)) {
        if (!replyOne->isFinished())
            connect(replyOne, &QModbusReply::finished, this, [this, replyOne](){
                emit readFinished(replyOne);  // read fiinished connects to ReadReady()
            });
        else
            delete replyOne; // broadcast replies return immediately
    } else {
        std::cout << "Read error: " + modbus->errorString().toStdString() << std::endl;
  }
  const auto random_value = std::rand() / (double)RAND_MAX;
  if (random_value < 0.5){
    test_doze -= random_value;
    test_doze2 -= random_value;
  }
  else{
    test_doze += random_value;
    test_doze2 += random_value * 2;
  }
  doze_gauge_neutron->Set(test_doze);
  doze_gauge_gamma->Set(test_doze2);
}

void Panel::onReadReady(QModbusReply* reply){
  std::cout << " we reading smth!" << std::endl;
  if (!reply)
      return;
  if (reply->error() == QModbusDevice::NoError) {
    const QModbusDataUnit unit = reply->result();
    std::cout << "u0 = " << unit.value(0) << " u1 = " << unit.value(1) << " u2 = " << unit.value(2) << " u3= " << unit.value(3) << std::endl;
    uint16_t data[2];
    data[0] = unit.value(0);
    data[1] = unit.value(1);
    memcpy(&gammaDoze, data, 4); 
    print(gammaDoze);
    if (gammaDoze > 0 & gammaDoze < 0.1)
        gammaDoze = 0.1;
    data[0] = unit.value(2);
    data[1] = unit.value(3);
    memcpy(&neutronDoze, data, 4);
    print(neutronDoze);
    if (neutronDoze > 0 & neutronDoze < 0.1)
        neutronDoze = 0.1;
  } else if (reply->error() == QModbusDevice::ProtocolError) {
      std::cout << "Read response error: %1 (Mobus exception: 0x%2)";
  } else {
      std::cout << "Read response error: %1 (code: 0x%2)";
  }
  reply->deleteLater();
}

float Panel::getGammadoze(){
  return gammaDoze;
}

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  print("begin");
  // create an http server running on port 8080
  Exposer exposer{"127.0.0.1:8080"};
  std::cout << "After exposer " << std::endl;
  // create a metrics registry
  // @note it's the users responsibility to keep the object alive
  auto registry = std::make_shared<Registry>();
  Panel *testPanel = new Panel(1, "192.168.0.10", "one", registry);
  Panel *testPanel2 = new Panel(1, "192.168.0.11", "two", registry);
  Panel *testPanel3 = new Panel(1, "192.168.0.12", "three", registry);
  Panel *testPanel4 = new Panel(1, "192.168.0.13", "four", registry);
  Panel *testPanel5 = new Panel(1, "192.168.0.14", "five", registry);
  print("panel done");
  exposer.RegisterCollectable(registry);
  
  return a.exec();
}

#include "psTest.moc"
