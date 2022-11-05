#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <vector>
#include <string>
#include <map>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

// Choose whether to mask the PAN or not
#define MASKED 0

#define MAX_FRAME_LEN 300

extern "C" {
	#include <nfc/nfc.h>

	// declare func
	int pn53x_transceive(struct nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, const size_t szRxLen, int timeout);
}

std::ofstream datafile;

std::string bytes_to_string(std::vector<uint8_t> data, std::string prefix="", std::string separator="") {
    std::stringstream ss;
    ss << std::hex;
    for(auto b = data.begin(); b < data.end(); b++) {
        ss << prefix << std::setw(2) << std::setfill('0') << (unsigned int) *b;
        if(b + 1 != data.end())
            ss << separator;
    }
    return ss.str();
}

std::vector<uint8_t> pn53x_transceive(struct nfc_device *pnd, std::vector<uint8_t> tx, int timeout=5000) {
    uint8_t abtRx[MAX_FRAME_LEN];
    size_t szRx = sizeof(abtRx);

    int res = pn53x_transceive(pnd, tx.data(), tx.size(), abtRx, szRx, timeout);
    if(res < 0) {
        throw std::runtime_error("pn53x_transceive");
    }
    std::vector<uint8_t> rx(abtRx + 1, abtRx + res);
    return rx;
}

std::vector<uint8_t> try_starting(struct nfc_device *pnd) {
    static std::vector<uint8_t> START_14443A = {0x4A, 0x01, 0x00}; //InListPassiveTarget
    static std::vector<uint8_t> START_14443B = {0x4A, 0x01, 0x03, 0x00}; //InListPassiveTarget
    std::vector<uint8_t> resp;
    try {
        // 14443A Card
        resp = pn53x_transceive(pnd, START_14443A);
    } catch(std::runtime_error ex) {}
    if (resp.size() > 0) {
        std::cout << "[+] 14443A card found!!" << std::endl;
    } else {
        // 14443B Card
        try {
            // 14443B Card
            resp = pn53x_transceive(pnd, START_14443B);
        } catch(std::runtime_error ex) {}
        if (resp.size() > 0) {
            std::cout << "[+] 14443B card found!!" << std::endl;
        } else {
            std::cout << "[x] Card not found or not supported!! Supported types: 14443A, 14443B." << std::endl;
        }
    }

    return resp;
}

std::vector<uint8_t> select_AID(struct nfc_device *pnd, std::vector<uint8_t> AID, uint8_t p1=0x04, uint8_t p2=0x00) {
    std::vector<uint8_t> select_cmd = {0x40, 0x01, 0x00, 0xA4, p1, p2, (uint8_t)AID.size()};
    select_cmd.insert( select_cmd.end(), AID.begin(), AID.end() );
    select_cmd.push_back(0x00);
    return pn53x_transceive(pnd, select_cmd);
}

std::vector<uint8_t> get_processing_options(nfc_device* pnd, std::vector<uint8_t> capabilities={0x83, 0x00}) {
    std::vector<uint8_t> GPO_cmd = {0x40, 0x01, 0x80, 0xA8, 0x00, 0x00, (uint8_t)capabilities.size() };
    GPO_cmd.insert( GPO_cmd.end(), capabilities.begin(), capabilities.end() );
    GPO_cmd.push_back(0x00);    
    return pn53x_transceive(pnd, GPO_cmd);
}

std::vector<uint8_t> read_record(struct nfc_device *pnd, uint8_t p1, uint8_t p2) {
    std::vector<uint8_t> read_cmd = {0x40, 0x01, 0x00, 0xB2, p1, p2, 0x00};
    return pn53x_transceive(pnd, read_cmd);
}

void dump_TLV(std::vector<uint8_t> data) {
    datafile << bytes_to_string(data);
}

void decode_TLV(std::vector<uint8_t> data) {
    if(data.empty())
        return;
    std::cout << "https://emvlab.org/tlvutils/?data=" << bytes_to_string(data, "", "") << std::endl;
#if 0
    /* Look for cardholder name */
    uint8_t * res = data.data();
    unsigned char output[50], c, amount[10],msg[100];
    int szRx = data.size();
    unsigned int i, j, expiry;
    for (i=0;i<(unsigned int) szRx-1;i++) {
            if (*res==0x5f&&*(res+1)==0x20) {
                strncpy(output, res+3, (int) *(res+2));
                output[(int) *(res+2)]=0;
                printf("Cardholder name: %s\n",output);            
            }
            res++;
    }

    /* Look for PAN & Expiry date */
    res = data.data();
    for (i=0;i<(unsigned int) szRx-1;i++) {
        if ((*res==0x4d&&*(res+1)==0x57)||(*res==0x9f&&*(res+1)==0x6b)) {
            strncpy(output, res+3, 13);
            output[11]=0;
            std::cout << "PAN:";
            
            for (j=0;j<8;j++) {
                if (j%2==0) std::cout << " ";
                c = output[j];
                if (MASKED&j>=1&j<=5) {
                    std::cout << "**";
                }
                else {
                    printf("%02x",c&0xff);
                }
            }
            std::cout << std::endl;
            expiry = (output[10]+(output[9]<<8)+(output[8]<<16))>>4;
            printf("Expiration date: %02x/20%02x\n\n",(expiry&0xff),((expiry>>8)&0xff));
            break;            
        }
        res++;
    }       

    /* Look for public certificates */
    res = data.data();
    szRx = data.size();
    for (i=0;i<(unsigned int) szRx-1;i++) {
        if (*res==0x9f && *(res+1)==0x46 && *(res+2)==0x81) {
            printf("ICC Public Key Certificate:\n");
            int k;
            for (k=4;k<(int)148;k++) {
                printf("%02x",(unsigned int)*(res+k));
            }
            printf("\n\n");
            break;            
        }
        res++;
    }
    
    /* Look for public certificates */
    res = data.data();
    szRx = data.size();
    for (i=0;i<(unsigned int) szRx-1;i++) {
        if (*res==0x90 && *(res+1)==0x81 && *(res+2)==0xb0) {
            printf("Issuer Public Key Certificate:\n");
            int k;
            for (k=3;k<(int)173;k++) {
                printf("%02x",(unsigned int)*(res+k));
            }
            printf("\n\n");
            break;            
        }
        res++;
    }     

    // Looking for transaction logs
    szRx = data.size();
    if (szRx==18) { // Non-empty transaction
        res = data.data();

        /* Look for date */
        sprintf(msg,"%02x/%02x/20%02x",res[14],res[13],res[12]);

        /* Look for transaction type */
        if (res[15]==0) {
            sprintf(msg,"%s %s",msg,"Payment");
        }
        else if (res[15]==1) {
            sprintf(msg,"%s %s",msg,"Withdrawal");
        }
        
        /* Look for amount*/
        sprintf(amount,"%02x%02x%02x",res[3],res[4],res[5]);
        sprintf(msg,"%s\t%d,%02x€",msg,atoi(amount),res[6]);
        printf("%s\n",msg);
    }
#endif
}

std::map<std::string, std::vector<uint8_t>> get_AIDs_from_PSE(std::vector<uint8_t> pse) {
    std::map<std::string, std::vector<uint8_t>> result;

    int i = 0;
    int app_index = 0;
    while(i < pse.size()) {
        if(pse[i] == 0xA5) { // Proprietary information encoded in BER-TLV
            int pi_end = i + 2 + pse[i + 1];
            i += 2;

            while(i < pi_end) {
                if(pse[i] == 0xBF && pse[i + 1] == 0x0C) { // FCI issuer discretionary data
                    int fci_end = i + 3 + pse[i + 2];
                    i += 3;

                    while(i < fci_end) {
                        if(pse[i] == 0x61) { // Application template
                            int app_end = i + 2 + pse[i + 1];
                            std::cout << "[+] Found Application template at " << i << " until " << app_end << std::endl;
                            i += 2;

                            std::vector<uint8_t> AID;
                            std::string app_lable("UNLABLED AID #" + app_index++);

                            while(i < app_end) {
                                if(pse[i] == 0x4F) { // Application ID
                                    int appid_len = pse[i + 1];
                                    int appid = i + 2;
                                    int appid_end = appid + appid_len;

                                    AID = std::vector<uint8_t>(pse.begin() + appid, pse.begin() + appid_end);

                                    std::cout << "[+] Found AID of len " << appid_len << " at " << i << ": " 
                                        << bytes_to_string(AID) << std::endl;

                                    i = appid_end;
                                } else if(pse[i] == 0x50) { // Application Lable
                                    int lable_len = pse[i + 1];
                                    int lable = i + 2;
                                    int lable_end = lable + lable_len;

                                    app_lable = std::string( (char*) &*(pse.begin() + lable) , lable_len);
                                    std::cout << "[+] Found an Application lable " << app_lable << std::endl;

                                    i = lable_end;
                                } else if(pse[i] == 0x9F && pse[i + 1] == 0x0A) { // Application Selection Registered Proprietary Data
                                    i += 3 + pse[i + 2]; // ignored
                                } else {
                                    i += 2 + pse[i + 1];
                                }
                            }
                            result[app_lable] = AID;

                            i = app_end;
                        } else {
                            i += 2 + pse[i + 1];
                        }
                    }
                    i = fci_end;
                } else {
                    i += 2 + pse[i + 1];
                }
            }
        } else {
            i += 2 + pse[i + 1];              
        }
    }

    return result;
}

std::map<std::string, std::vector<uint8_t>> try_known_AIDs(nfc_device* pnd) {
    #include "known_AIDs.hpp"
    std::map<std::string, std::vector<uint8_t>> result;

    std::cout << "[-] Trying known AIDs..." << std::endl;

    std::copy_if(known_AIDs.begin(), known_AIDs.end(), std::inserter(result,result.end()), [pnd](auto AID) {
        auto resp = select_AID(pnd, AID.second);

        return resp[0] == 0x6F;
    });

    return result;
}

std::map<std::string, std::vector<uint8_t>> get_AIDs(nfc_device* pnd) {
    std::map<std::string, std::vector<uint8_t>> AIDs;

    std::vector<uint8_t> PSE = {0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31};
    std::vector<uint8_t> PPSE = {0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31};

    auto resp = select_AID(pnd, PPSE);
    if(resp.empty())
        resp = select_AID(pnd, PSE);
    if(!resp.empty() && resp[0] == 0x6F) {
        std::vector<uint8_t> pse(resp.begin() + 2, resp.end() - 2);
        std::cout << "[+] Got PSE/PPSE:" << std::endl << bytes_to_string(pse) << std::endl;
        dump_TLV(pse);
        AIDs = get_AIDs_from_PSE(pse);
    } else {
        AIDs = try_known_AIDs(pnd);
    }

    std::cout << "[+] Found " << AIDs.size() << " AIDs" << std::endl;

    return AIDs;
}

std::vector<std::pair<uint16_t, uint8_t>> get_PDOL_from_SELECT_response(std::vector<uint8_t> resp) {
    int i = 0;
    while(i < resp.size()) {
        if(resp[i] == 0xA5) { // Proprietary information encoded in BER-TLV
            int pi_end = i + 2 + resp[i + 1];
            i += 2;

            while(i < pi_end) {
                if(resp[i] == 0x9F && resp[i + 1] == 0x38) { // Processing Options Data Object List (PDOL)
                    int pdol = i + 3;
                    int pdol_len = resp[i + 2];
                    int pdol_end = pdol + resp[i + 2];
                    
                    std::vector<uint8_t> PDOL_raw(resp.begin() + pdol, resp.begin() + pdol_end);

                    std::vector<std::pair<uint16_t, uint8_t>> PDOL;

                    for(int j = 0; j < PDOL_raw.size(); j += 3) {
                        uint16_t tag = (PDOL_raw[j] << 8) | PDOL_raw[j + 1];
                        uint8_t length = PDOL_raw[j + 2];
                        PDOL.push_back(std::make_pair(tag, length));
                    }
                    
                    return PDOL;

                } else {
                    i += 2 + resp[i + 1];
                }
            }
        } else {
            i += 2 + resp[i + 1];              
        }
    }

    return std::vector<std::pair<uint16_t, uint8_t>>();
}

std::vector<uint8_t> get_capabilities_from_PDOL(std::vector<std::pair<uint16_t, uint8_t>> PDOL) {
    std::vector<uint8_t> capabilities({0x83, 0x00});

    for(auto tl: PDOL) {
        uint16_t tag = tl.first;
        uint8_t length = tl.second;

        switch(tag) {
            default:
                for(int i = 0; i < length; i++)
                    capabilities.push_back(0x00);
                break;
        }

        capabilities[1] += length;
    }

    return capabilities;
}

void try_reading_sector(nfc_device* pnd, uint8_t p1, uint8_t p2) {
    auto resp = read_record(pnd, p1, p2);
    if ( resp[0]==0x6a && (resp[1]==0x81 || resp[1]==0x82 || resp[1]==0x83)) {
        // File or application not found
        return;
    } else if ( resp[0]==0x6a && resp[1]==0x86) {
        // Wrong parameters
        return;
    }/* else if ( resp[0]==0x40 || resp[0]==0x00) {
        // Something weird
        return;
    }*/
    else{
        std::cout << std::endl << "> READ RECORD " << std::hex
            << std::setw(2) << std::setfill('0') << (unsigned int) p1 << "-" 
            << std::setw(2) << std::setfill('0') << (unsigned int) p2 << " suceeded" << std::endl;
        std::cout << (unsigned int) resp[0] << " " << (unsigned int) resp[1] << std::endl;
        std::vector<uint8_t> tlv(resp.begin(), resp.end() - 2);
        dump_TLV(tlv);
        decode_TLV(tlv);
    }
}

static void close(nfc_device*pnd, nfc_context* context) {
    std::cout << "[-] Closing device..." << std::endl;
    if(pnd)
        nfc_close(pnd);
    if(context)
        nfc_exit(context);
    if(datafile)
        datafile.close();
}

int main(int argc, char **argv) {
    nfc_context *context;
    nfc_device* pnd;
    nfc_modulation nm;
    nfc_target ant[1];

    try {
        std::cout << "[-] Connecting to the reader..." << std::endl;
        nfc_init(&context);
        pnd = nfc_open(context,NULL);
        if (pnd == NULL) {
            std::cout << "[x] Unable to connect to NFC device." << std::endl;
            close(pnd, context);
            return(1);
        }

        std::cout << "[+] Connected to NFC reader" << std::endl;
        nfc_initiator_init(pnd);

        // Checking card type...
        std::cout << "[-] Trying to start..." << std::endl;
        
        auto resp = try_starting(pnd);
        if(!resp.empty()) {
            datafile.open("data.txt");

            std::cout << "[-] Searching for AIDs..." << std::endl;

            auto AIDs = get_AIDs(pnd);
            
            for(auto AID: AIDs) {
                std::cout << "[-] Selecting app \"" << AID.first << "\"..." << std::endl;
                resp = select_AID(pnd, AID.second);
                decode_TLV(resp);
                if(resp[0] != 0x6F)
                    continue;

                resp = std::vector<uint8_t>(resp.begin() + 2, resp.end() - 2);
                auto PDOL = get_PDOL_from_SELECT_response(resp);

                std::cout << "[-] Getting Processing Options..." << std::endl;
                auto capabilities = get_capabilities_from_PDOL(PDOL);
                resp = get_processing_options(pnd, capabilities);
                decode_TLV(resp);

                // Looking for data in the records
                for (int ef = 1; ef <= 31; ef++) {
                    for (int rec = 0; rec <= 16; rec++) {
                        try_reading_sector(pnd, rec, (ef << 3) | 4);
                    }
                }
            }
            std::cout << std::endl;
        }
    } catch(std::runtime_error ex) {
        close(pnd, context);
        throw ex;
    }

    close(pnd, context);
    return(0);
}