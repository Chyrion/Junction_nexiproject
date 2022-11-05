#include <iostream>
#include <vector>
extern "C" {
  #include <nfc/nfc.h>
  #include <nfc/nfc-types.h>
  int pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout);
}

#define MAX_FRAME_LEN 300

std::vector<uint8_t> pn53x_transceive(struct nfc_device *pnd, std::vector<uint8_t> tx, int timeout=5000) {
    uint8_t abtRx[MAX_FRAME_LEN];
    size_t szRx = sizeof(abtRx);

    int res = pn53x_transceive(pnd, tx.data(), tx.size(), abtRx, szRx, timeout);
    if(res < 0) {
        throw std::runtime_error("pn53x_transceive");
    }
    std::vector<uint8_t> rx(abtRx, abtRx + res);
    return rx;
}


int main() {
  nfc_device* pnd;
  nfc_context *context;
  nfc_connstring connstring = "pn532_uart:/dev/ttyUSB0";
  nfc_target nt;

  std::vector<uint8_t> START_14443A = {0x4A, 0x01, 0x00};
  std::vector<uint8_t> START_14443B = {0x4A, 0x01, 0x03, 0x00};

  nfc_init(&context);
  pnd = nfc_open(context, connstring);
  if (pnd == NULL) {
    printf("Error connecting to device");
    exit(EXIT_FAILURE);
  }
  printf("Successfully connected to NFC device\n");
  nfc_initiator_init(pnd);
  printf("Reader opened: %s\n", nfc_device_get_name(pnd));
  std::vector<uint8_t> resp;
  try {
    resp = pn53x_transceive(pnd, START_14443A);
  } catch(std::runtime_error ex) {}
  if (resp.size() > 0) {
    printf("14443A Card found!\n");
  } else {
        // 14443B Card
        try {
            resp = pn53x_transceive(pnd, START_14443B);
        } catch(std::runtime_error ex) {}
        if (resp.size() > 0) {
            printf("14443B card found!\n");
        } else {
            printf("Card not found or not supported! Supported types: 14443A, 14443B.\n");
            exit(EXIT_FAILURE);
            return(1);
        }
    }

}