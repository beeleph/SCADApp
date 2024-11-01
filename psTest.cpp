#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <QMainWindow>
#include <QModbusTCPClient>
#include <QVariant>

#include <array>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include <iostream>

class Panel : QObject {
  private:
    int modbusSlaveID = 0; 
    std::string ip; 
    QString ip;
    QModbusTcpClient *modbus = nullptr;
    QModbusDataUnit *gamma = nullptr;
    QModbusDataUnit *neutron = nullptr;
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
        std::cout << "Could not create Modbus master.";
    }
    const QString a("vasili4");
    modbus->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QString::fromStdString(ip));
    modbus->setConnectionParameter(QModbusDevice::NetworkPortParameter, "502");
    modbus->setTimeout(3000);
    if (!modbus->connectDevice()) {
      std::cout << "Connect failed: " << modbus->errorString().toStdString();
    }
    gamma = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 2); // CONFIRM THAT!
    neutron = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 2, 2);// CONFIRM THAT!
    QObject::connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
      std::cout << "Hello";
  }
};

int main() {
  /*using namespace prometheus;

  // create an http server running on port 8080
  Exposer exposer{"127.0.0.1:8080"};

  // create a metrics registry
  // @note it's the users responsibility to keep the object alive
  auto registry = std::make_shared<Registry>();

  auto& doze_gauge = BuildGauge()
                            .Name("Doze_post_one")
                            .Help("doze from first post from neutron and gamma detector")
                            .Register(*registry);

  auto& doze_gauge_netron = doze_gauge.Add({{"Netron_doze", "mZvt"}});
  auto& doze_gauge_gamma = doze_gauge.Add({{"Gamma_doze", "mZvt"}});

  // add a new counter family to the registry (families combine values with the
  // same name, but distinct label dimensions)
  //
  // @note please follow the metric-naming best-practices:
  // https://prometheus.io/docs/practices/naming/
  /*auto& packet_counter = BuildCounter()
                             .Name("observed_packets_total")
                             .Help("Number of observed packets")
                             .Register(*registry);

  // add and remember dimensional data, incrementing those is very cheap
  auto& tcp_rx_counter =
      packet_counter.Add({{"protocol", "tcp"}, {"direction", "rx"}});
  auto& tcp_tx_counter =
      packet_counter.Add({{"protocol", "tcp"}, {"direction", "tx"}});
  auto& udp_rx_counter =
      packet_counter.Add({{"protocol", "udp"}, {"direction", "rx"}});
  auto& udp_tx_counter =
      packet_counter.Add({{"protocol", "udp"}, {"direction", "tx"}});

  // add a counter whose dimensional data is not known at compile time
  // nevertheless dimensional values should only occur in low cardinality:
  // https://prometheus.io/docs/practices/naming/#labels
  auto& http_requests_counter = BuildCounter()
                                    .Name("http_requests_total")
                                    .Help("Number of HTTP requests")
                                    .Register(*registry);*/

  // ask the exposer to scrape the registry on incoming HTTP requests
  /*exposer.RegisterCollectable(registry);
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
    /*
    if (random_value & 1) tcp_rx_counter.Increment();
    if (random_value & 2) tcp_tx_counter.Increment();
    if (random_value & 4) udp_rx_counter.Increment();
    if (random_value & 8) udp_tx_counter.Increment();

    const std::array<std::string, 4> methods = {"GET", "PUT", "POST", "HEAD"};
    auto method = methods.at(random_value % methods.size());
    // dynamically calling Family<T>.Add() works but is slow and should be
    // avoided
    http_requests_counter.Add({{"method", method}}).Increment();*/
  //}
  return 0;
}
