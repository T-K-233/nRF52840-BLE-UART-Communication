#include <bluefruit.h>
#include <Adafruit_DotStar.h>


#define DEVICE_NAME   "node_1"

#define STATUS_LIGHT_ERROR    0x001100       // red
#define STATUS_LIGHT_PENDING  0x000011       // blue
#define STATUS_LIGHT_OK       0x110000       // green

BLEDis  ble_deviceinfo;   // device information
BLEBas  ble_battery;      // battery
BLEUart ble_uart;         // uart over ble

// lights are in GRB order
Adafruit_DotStar status_light(1, 8, 6);

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
  // custom message here
  ble_uart.write("{\"method\": \"SET\", \"params\": {\"/node_name\": \"");
  ble_uart.write(DEVICE_NAME);
  ble_uart.write("\", \"/temperature\": 0.222, /temperature\": 0.222, temperature\": 0.222, temperature\": 0.222, temperature\": 0.222, temperature\": 0.222}\n");
  Serial.write("{\"method\": \"SET\", \"params\": {\"/node_name\": ");
  Serial.write(DEVICE_NAME);
  Serial.write("\"/temperature\": 0.222, /temperature\": 0.222, temperature\": 0.222, temperature\": 0.222, temperature\": 0.222, temperature\": 0.222}\n");
  

  while (ble_uart.available()) {
    uint8_t c;
    c = (uint8_t) ble_uart.read();
    Serial.write(c);
  }

  delay(1000);
}
