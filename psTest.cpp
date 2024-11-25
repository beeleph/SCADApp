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

// разделить на хедер и спп.

void print(std::string str){
  std::cout << str << std::endl;
}
void print(float str){
  std::cout << std::to_string(str) << std::endl;
}

class Panel : public QObject {
    Q_OBJECT
// bybre

  private:
    int modbusSlaveID = 0;
    std::string ip;
    float gammaDoze = 0.0;
    float netronDoze = 0.0;
    //QString ip;
    QModbusTcpClient *modbus = nullptr;
    QModbusDataUnit *gammaNeutron = nullptr;
    //QModbusDataUnit *neutron = nullptr;
    QTimer *readLoopTimer;
    //QThread mainThread;

  private slots:
    void onReadReady(QModbusReply* reply);
    void loop_goBabe();
    void threadStarted();

  signals:
    void readFinished(QModbusReply* reply);

  public:
     int var;     // data member
     float getGammadoze();
     void start_loop_goBabe();

  Panel(int modbusSlaveID, std::string ip) {
    readLoopTimer = new QTimer(this);
    this->modbusSlaveID = modbusSlaveID;
    this->ip = ip;
    modbus = new QModbusTcpClient();
    QObject::connect(modbus, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        std::cout << modbus->errorString().toStdString() << std::endl;
    });
    if (!modbus) {
        std::cout << "Could not create Modbus master." << std::endl;
    }
    const QString a("vasili4");
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
  }
};

void Panel::threadStarted(){
  readLoopTimer->start(1000);
}

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
}

void Panel::start_loop_goBabe(){
  loop_goBabe();
  //emit readLoopTimer->timeout();
  
}

void Panel::onReadReady(QModbusReply* reply){
  std::cout << " we reading smth!" << std::endl;
  if (!reply)
      return;
  if (reply->error() == QModbusDevice::NoError) {
    const QModbusDataUnit unit = reply->result();
    std::cout << "u0 = " << unit.value(0) << " u1 = " << unit.value(1) << " u2 = " << unit.value(2) << " u3= " << unit.value(3) << std::endl;
    //unsigned short data[2];
    uint16_t data[2];
    data[0] = unit.value(0);
    data[1] = unit.value(1);
    //data[0] = 19564;
    //data[1] = 32536;
    // after this working fine!
    memcpy(&gammaDoze, data, 4); 
    print(gammaDoze);
    if (gammaDoze > 0 & gammaDoze < 0.1)
        gammaDoze = 0.1;
    data[0] = unit.value(2);
    data[1] = unit.value(3);
    memcpy(&netronDoze, data, 4);
    print(netronDoze);
    if (netronDoze > 0 & netronDoze < 0.1)
        netronDoze = 0.1;
  } else if (reply->error() == QModbusDevice::ProtocolError) {
      std::cout << "Read response error: %1 (Mobus exception: 0x%2)";
  } else {
      std::cout << "Read response error: %1 (code: 0x%2)";
  }
  reply->deleteLater();
  //
  //print(netronDoze);
}

float Panel::getGammadoze(){
  return gammaDoze;
}

void exposerShit(){
  using namespace prometheus;

  // create an http server running on port 8080
  Exposer exposer{"127.0.0.1:8080"};
  std::cout << "after exposer there is no life! " << std::endl;
  // create a metrics registry
  // @note it's the users responsibility to keep the object alive
  auto registry = std::make_shared<Registry>();

  auto& doze_gauge = BuildGauge() 
                            .Name("Doze_post_one")
                            .Help("doze from first post from neutron and gamma detector")
                            .Register(*registry);

  auto& doze_gauge_netron = doze_gauge.Add({{"Netron_doze", "mZvt"}});
  auto& doze_gauge_gamma = doze_gauge.Add({{"Gamma_doze", "mZvt"}});

  // ask the exposer to scrape the registry on incoming HTTP requests
  exposer.RegisterCollectable(registry);
  double doze = 0;
  double doze2 = 0;
  for (;;) {
    //testPanel->start_loop_goBabe();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto random_value = std::rand() / (double)RAND_MAX;
    if (random_value < 0.5){
      doze -= random_value;
      doze2 -= random_value;
    }
    else{
      doze += random_value;
      doze2 += random_value * 2;
    }
    doze_gauge_netron.Set(doze);
    doze_gauge_gamma.Set(doze2);
    //std::cout << "current gamma is: " << testPanel->getGammadoze() << std::endl;
  }
}



int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  print("begin");
  Panel *testPanel = new Panel(1, "192.168.0.10");
  print("panel done]");
  //for(;;){
    //std::this_thread::sleep_for(std::chrono::seconds(1)); // убрать это при работе с qcoreapp !!! и цикл бесконечный не нужен. а что раз в секунду выполнять это можно через таймера. и нужно через таймера. 
    //testPanel->start_loop_goBabe();
    //std::cout << testPanel->getGammadoze() << std::endl;
  //}
  print("hello");
  return a.exec();
}

#include "psTest.moc"
