#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <QCoreApplication>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QVariant>
#include <QTimer>

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
// протестировать промлибу тут
// попробовать qt plugin для вскод?

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

  private slots:
    void onReadReady(QModbusReply* reply);
    void loop_goBabe();

  signals:
    void readFinished(QModbusReply* reply);

  public:
     int var;     // data member

  Panel(int modbusSlaveID, std::string ip) {
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
    QObject::connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    std::cout << "Versioning, name, contacts, date" << std::endl;
    readLoopTimer = new QTimer(this);
    QObject::connect(readLoopTimer, SIGNAL(timeout()), this, SLOT(loop_goBabe()));
    readLoopTimer->start(1000);
  }
};

void Panel::loop_goBabe(){
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

void Panel::onReadReady(QModbusReply* reply){
  if (!reply)
      return;
  if (reply->error() == QModbusDevice::NoError) {
    const QModbusDataUnit unit = reply->result();
    unsigned short data[2];
    data[0] = unit.value(0);
    data[1] = unit.value(1);
    memcpy(&gammaDoze, data, 4);
    if (gammaDoze > 0 & gammaDoze < 0.1)
        gammaDoze = 0.1;
    data[0] = unit.value(3);
    data[1] = unit.value(4);
    memcpy(&netronDoze, data, 4);
    if (netronDoze > 0 & netronDoze < 0.1)
        netronDoze = 0.1;
  } else if (reply->error() == QModbusDevice::ProtocolError) {
      std::cout << "Read response error: %1 (Mobus exception: 0x%2)";
  } else {
      std::cout << "Read response error: %1 (code: 0x%2)";
  }
  reply->deleteLater();
}

int main() {
  std::cout << "vasili4" << std::endl;
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
  }
  std::cout << "till the end of times " << std::endl;
  return 0;
}

#include "psTest.moc"
