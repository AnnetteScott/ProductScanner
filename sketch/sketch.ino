#include "pio_usb.h"
#include "Adafruit_TinyUSB.h"
#include <string>

#define PIN_USB_HOST_DP  0
#define USBH_HELPER_H
#define PIN_5V_EN_STATE  1

Adafruit_USBH_Host USBHost;

//------------- Core0 -------------//
void setup() {
  Serial.begin(9000);
}

void loop() {
  Serial.flush();
}

//------------- Core1 -------------//
void setup1() {
  rp2040_configure_pio_usb();

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
  tuh_hid_receive_report(dev_addr, instance);
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
  String key = getChar(report[2]);
  Serial.print(key);
  
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
  else if(input == 40){ return "\n"; }

  return "";
}


static void rp2040_configure_pio_usb(void) {
  //while ( !Serial ) delay(10);   // wait for native usb
  Serial.println("Core1 setup to run TinyUSB host with pio-usb");

  // Check for CPU frequency, must be multiple of 120Mhz for bit-banging USB
  uint32_t cpu_hz = clock_get_hz(clk_sys);
  if (cpu_hz != 120000000UL && cpu_hz != 240000000UL) {
    while (!Serial) {
      delay(10);   // wait for native usb
    }
    Serial.printf("Error: CPU Clock = %lu, PIO USB require CPU clock must be multiple of 120 Mhz\r\n", cpu_hz);
    Serial.printf("Change your CPU Clock to either 120 or 240 Mhz in Menu->CPU Speed \r\n");
    while (1) {
      delay(1);
    }
  }

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
