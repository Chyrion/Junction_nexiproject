# Junction 2022 - Nexi challenge

NFC card reader, primarily designed for payment cards.

### Dependencies

- [libnfc](https://github.com/nfc-tools/libnfc) (>1.8.0)
- NFC reader (Adafruit PN532 used, but should work with any)

### Usage

Current workflow:

1. Run rdr.cpp (make)
2. Scan an NFC card while the reader is running
3. Run the GUI

### Credits

- [tvlp](https://github.com/heyvito/tlvp) by Victor Gama
