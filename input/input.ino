#include "pio_usb.h"
#include "Adafruit_TinyUSB.h"
#include <string>

#define PIN_USB_HOST_DP  0
#define USBH_HELPER_H
#define PIN_5V_EN_STATE  1

String barcode;

Adafruit_USBH_Host USBHost;

//------------- Core0 -------------//
void setup() {
  Serial.begin(115200);
  Serial2.setRX(9);
  Serial2.setTX(8);
  Serial2.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  //while ( !Serial ) delay(10);   // wait for native usb
  Serial.println("TinyUSB Dual: HID Device Report Example");
}

void loop() {
  Serial.flush();
  Serial2.flush();
}

//------------- Core1 -------------//
void setup1() {
  // configure pio-usb: defined in usbh_helper.h
  rp2040_configure_pio_usb();

  // run host stack on controller (rhport) 1
  // Note: For rp2040 pico-pio-usb, calling USBHost.begin() on core1 will have most of the
  // host bit-banging processing works done in core1 to free up core0 for other works
  USBHost.begin(1);
}

void loop1() {
  USBHost.task();
}


// Invoked when device with hid interface is mounted
// Invoked when device is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    Serial.printf("Error: cannot request to receive report\r\n");
  }
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_umount_cb(uint8_t daddr) {
  Serial.printf("Device removed, address = %d\r\n", daddr);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
  if(report[2] != 0){
    String key = getChar(report[2]);
    if(key == "END"){
      Serial2.println(barcode);
      digitalWrite(LED_BUILTIN, LOW);
      barcode = "";
    }else {
      digitalWrite(LED_BUILTIN, HIGH);
      barcode += key;
    }
    Serial.print(key);
  }

  // continue to request to receive report
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    Serial.printf("Error: cannot request to receive report\r\n");
  }
}

String getChar(uint8_t input){
  if(input == 30){ return "1"; }
  else if(input == 31){ return "2"; }
  else if(input == 32){ return "3"; }
  else if(input == 33){ return "4"; }
  else if(input == 34){ return "5"; }
  else if(input == 35){ return "6"; }
  else if(input == 36){ return "7"; }
  else if(input == 37){ return "8"; }
  else if(input == 38){ return "9"; }
  else if(input == 39){ return "0"; }
  else if(input == 40){ return "END"; }

  return "";
}


static void rp2040_configure_pio_usb(void) {
  // Check for CPU frequency, must be multiple of 120Mhz for bit-banging USB
  uint32_t cpu_hz = clock_get_hz(clk_sys);

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = PIN_USB_HOST_DP;

  // For pico-w, PIO is also used to communicate with cyw43
  // Therefore we need to alternate the pio-usb configuration
  // details https://github.com/sekigon-gonnoc/Pico-PIO-USB/issues/46
  pio_cfg.sm_tx      = 3;
  pio_cfg.sm_rx      = 2;
  pio_cfg.sm_eop     = 3;
  pio_cfg.pio_rx_num = 0;
  pio_cfg.pio_tx_num = 1;
  pio_cfg.tx_ch      = 9;

  USBHost.configure_pio_usb(1, &pio_cfg);
}
