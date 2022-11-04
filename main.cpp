#include <iostream>
extern "C" {
  #include <nfc/nfc.h>
  #include <nfc/nfc-types.h>
}

int main() {
  nfc_context *context;
  nfc_connstring connstring = "pn532_uart:/dev/ttyUSB0";
  nfc_init(&context);
  //std::cout << nfc_open(context, connstring);
  return 0;
}