# Junction 2022 - Nexi challenge

NFC card reader, primarily designed for payment cards.

### Dependencies

- [libnfc](https://github.com/nfc-tools/libnfc) (>1.8.0)
- NFC reader (currently only Adafruit PN532 is supported)
- PyQt5


### Usage

Current workflow:

1. Run make in the root folder
2. Scan an NFC card while the reader is running
3. The GUI will open displaying the data

- Tested only working on Linux (it is possible to run on Windows if libnfc is working)

### Credits

- [nfc_creditcard_reader] (https://gist.github.com/bluec0re/7b32da7ec317b1f1c812) by bluec0re
- [tlvutils] (https://emvlab.org/tlvutils/) by emvlab
- [tvlp](https://github.com/heyvito/tlvp) by Victor Gama
