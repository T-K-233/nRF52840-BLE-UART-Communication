#include "bluefruit.h"
#include "Adafruit_DotStar.h"

#include "rath_SHT35.h"
#include "rath_PMSA003I.h"
#include "rath_TSL2591.h"
#include "rath_MMC5983MA.h"


#define DEVICE_NAME   "node_1"

#define STATUS_LIGHT_ERROR    0x001100       // red
#define STATUS_LIGHT_PENDING  0x000011       // blue
#define STATUS_LIGHT_OK       0x110000       // green


BLEDis  ble_deviceinfo;   // device information
BLEBas  ble_battery;      // battery
BLEUart ble_uart;         // uart over ble

// lights are in GRB order
Adafruit_DotStar status_light(1, 8, 6);


rath::SHT35 sht35 = rath::SHT35(rath::I2C0, 0x44);  // I2C address for SHT35 is 0x44 or 0x45
rath::PMSA003I psma = rath::PMSA003I();             // I2C address for PMSA003I is 0x12
rath::TSL2591 tsl2591 = rath::TSL2591();            // I2C address for PMSA003I is 0x12
rath::MMC5983MA mmc = rath::MMC5983MA();            // I2C address for PMSA003I is 0x12



void startBLEAdvertisement(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  Bluefruit.Advertising.addService(ble_uart);

  Bluefruit.ScanResponse.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void onConnectHandler(uint16_t conn_handle) {
  status_light.setPixelColor(0, STATUS_LIGHT_OK);
  status_light.show();
}

void onDisconnectHandler(uint16_t conn_handle, uint8_t reason) {
  status_light.setPixelColor(0, STATUS_LIGHT_ERROR);
  status_light.show();
}

void setup() {
  Serial.begin(115200);

  // sensor init
  sht35.init();
  sht35.disableHeater();

  psma.init();
  
  tsl2591.init();

  tsl2591.setGain(rath::TSL2591::Gain::GAIN_MEDIUM);
  tsl2591.setIntegrationTime(rath::TSL2591::IntegrationTime::T_500_MS);

  mmc.init();
  // end of sensor init
  
  status_light.begin();

  status_light.setPixelColor(0, STATUS_LIGHT_ERROR);
  status_light.show();
  
  Bluefruit.autoConnLed(false);
  
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.Periph.setConnectCallback(onConnectHandler);
  Bluefruit.Periph.setDisconnectCallback(onDisconnectHandler);

  ble_deviceinfo.setManufacturer("URAP");
  ble_deviceinfo.setModel("EnvDataNode");
  ble_deviceinfo.begin();

  ble_battery.begin();
  ble_battery.write(100);

  ble_uart.begin();

  startBLEAdvertisement();
  
  status_light.setPixelColor(0, STATUS_LIGHT_PENDING);
  status_light.show();
}

void loop() {
  ble_uart.write("{\"method\": \"SET\", \"params\": {\"/node_name\": \"");
  Serial.write("{\"method\": \"SET\", \"params\": {\"/node_name\": \"");
  ble_uart.write(DEVICE_NAME);
  Serial.write(DEVICE_NAME);
  ble_uart.write("\", ");
  Serial.write("\", ");

  
  float temperature = sht35.getTemperature();
  float humidity = sht35.getHumidity();

  ble_uart.print("\"/temperature\": ");
  Serial.print("\"/temperature\": ");
  ble_uart.print(temperature, 4);
  Serial.print(temperature, 4);
  ble_uart.print(", ");
  Serial.print(", ");
  
  ble_uart.print("\"/humidity\": ");
  Serial.print("\"/humidity\": ");
  ble_uart.print(humidity, 6);
  Serial.print(humidity, 6);
  ble_uart.print(", ");
  Serial.print(", ");


  
  float lux = tsl2591.get();

  ble_uart.print("\"/lux\": ");
  Serial.print("\"/lux\": ");
  ble_uart.print(lux, 4);
  Serial.print(lux, 4);
  ble_uart.print(", ");
  Serial.print(", ");


  rath::MMC5983MA::DataFrame mmc_data_frame = mmc.get();
  
  ble_uart.print("\"/magnetic_x\": ");
  Serial.print("\"/magnetic_x\": ");
  ble_uart.print(mmc_data_frame.x, 4);
  Serial.print(mmc_data_frame.x, 4);
  ble_uart.print(", ");
  Serial.print(", ");
  ble_uart.print("\"/magnetic_y\": ");
  Serial.print("\"/magnetic_y\": ");
  ble_uart.print(mmc_data_frame.y, 4);
  Serial.print(mmc_data_frame.y, 4);
  ble_uart.print(", ");
  Serial.print(", ");
  ble_uart.print("\"/magnetic_z\": ");
  Serial.print("\"/magnetic_z\": ");
  ble_uart.print(mmc_data_frame.z, 4);
  Serial.print(mmc_data_frame.z, 4);


  ble_uart.print("}}\n");
  Serial.print("}}\n");
  
  delay(1000);
}
