#include <bluefruit.h>


typedef struct {
  uint16_t conn_handle;
  BLEClientUart bleuart;
  
  uint8_t buffer[128];
  uint16_t buffer_size;
} client_connection;

client_connection clients[BLE_MAX_CONNECTION];


void onScanHandler(ble_gap_evt_adv_report_t *report) {
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with bleuart service advertised  
  // Connect to the device with bleuart service in advertising packet
  Bluefruit.Central.connect(report);
}

void onConnectHandler(uint16_t conn_handle) {
  // Find an available ID to use
  int id  = findConnHandle(BLE_CONN_HANDLE_INVALID);

  // Exceeded the number of connections
  if (id < 0) return;
  
  client_connection *peer = &clients[id];
  peer->conn_handle = conn_handle;
  
  if (peer->bleuart.discover(conn_handle)) {
    peer->bleuart.enableTXD();
    Bluefruit.Scanner.start(0);
  }
  else {
    // disconnect since we couldn't find bleuart service
    Bluefruit.disconnect(conn_handle);
  }
}

void onDisconnectHandler(uint16_t conn_handle, uint8_t reason) {

  // Mark the ID as invalid
  int id  = findConnHandle(conn_handle);

  // Non-existant connection, something went wrong, DBG !!!
  if (id < 0) return;

  // Mark conn handle as invalid
  clients[id].conn_handle = BLE_CONN_HANDLE_INVALID;
}

void onBLEUARTRXHandler(BLEClientUart& uart_svc) {
  uint16_t conn_handle = uart_svc.connHandle();

  int id = findConnHandle(conn_handle);
  client_connection *peer = &clients[id];
  
  // Print sender's name
  
  // Read then forward to all peripherals
  while (uart_svc.available()) {
    uint8_t c = uart_svc.read();
    if (c == '\n') {
      Serial.write(peer->buffer, peer->buffer_size);    
      Serial.write('\n');

      peer->buffer_size = 0;
      break;
    }
    peer->buffer[peer->buffer_size] = c;
    peer->buffer_size += 1;
  }
}

void sendAll(const char *str) {
  Serial.print("[Send to All]: ");
  Serial.println(str);
  
  for(uint8_t id=0; id<BLE_MAX_CONNECTION; id+=1) {
    client_connection *peer = &clients[id];

    if (peer->bleuart.discovered()) {
      peer->bleuart.print(str);
    }
  }
}

int findConnHandle(uint16_t conn_handle) {
  for(int id=0; id<BLE_MAX_CONNECTION; id+=1) {
    if (conn_handle == clients[id].conn_handle) {
      return id;
    }
  }
  return -1;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  
  // Initialize Bluefruit with max concurrent connections as Peripheral = 0, Central = 4
  // SRAM usage required by SoftDevice will increase with number of connections
  Bluefruit.begin(0, 4);
  Bluefruit.setName("Bluefruit52 Central");
  
  for (uint8_t i=0; i<BLE_MAX_CONNECTION; i+=1) {
    clients[i].conn_handle = BLE_CONN_HANDLE_INVALID;
    clients[i].bleuart.begin();
    clients[i].bleuart.setRxCallback(onBLEUARTRXHandler);
    
    clients[i].buffer_size = 0;
  }

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(onConnectHandler);
  Bluefruit.Central.setDisconnectCallback(onDisconnectHandler);

  Bluefruit.Scanner.setRxCallback(onScanHandler);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.filterUuid(BLEUART_UUID_SERVICE);
  Bluefruit.Scanner.useActiveScan(false);       // Don't request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds
}

void loop() {
  if (Bluefruit.Central.connected()) {
    
//    if (Serial.readBytes(buf, sizeof(buf)-1)) {
//      sendAll(buf);
//    }
  }
}
