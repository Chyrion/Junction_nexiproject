#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <vector>
#include <string>
#include <iostream>
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

void show(size_t recvlg, uint8_t *recv) {
    int i;

    for (i=0;i<(int) recvlg;i++) {
        printf("%02x",(unsigned int) recv[i]);
    }
}

void show(std::vector<uint8_t> data) {
    for(auto b: data) {
        printf("%02x",(unsigned int) b);
    }
}

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

std::vector<uint8_t> try_starting(struct nfc_device *pnd) {
    static std::vector<uint8_t> START_14443A = {0x4A, 0x01, 0x00}; //InListPassiveTarget
    static std::vector<uint8_t> START_14443B = {0x4A, 0x01, 0x03, 0x00}; //InListPassiveTarget

    std::vector<uint8_t> resp;
    try {
        // 14443A Card
        resp = pn53x_transceive(pnd, START_14443A);
    } catch(std::runtime_error ex) {}
    if (resp.size() > 0) {
        printf("[+] 14443A card found!!\n");
    } else{
        // 14443B Card
        try {
            // 14443A Card
            resp = pn53x_transceive(pnd, START_14443B);
        } catch(std::runtime_error ex) {}
        if (resp.size() > 0) {
            printf("[+] 14443B card found!!\n");
        } else {
            printf("[x] Card not found or not supported!! Supported types: 14443A, 14443B.\n");
        }
    }

    return resp;
}

std::vector<uint8_t> select_AID(struct nfc_device *pnd, std::vector<uint8_t> AID) {
    std::vector<uint8_t> select_cmd = {0x40, 0x01, 0x00, 0xA4, 0x04, 0x00};
    select_cmd.push_back((uint8_t)AID.size());
    select_cmd.insert( select_cmd.end(), AID.begin(), AID.end() );
    select_cmd.push_back(0x00);
    return pn53x_transceive(pnd, select_cmd);
}

std::vector<uint8_t> read_record(struct nfc_device *pnd, uint8_t p1, uint8_t p2) {
    std::vector<uint8_t> read_cmd = {0x40, 0x01, 0x00, 0xB2, p1, p2, 0x00};
    return pn53x_transceive(pnd, read_cmd);
}

std::vector<std::vector<uint8_t>> get_AIDs_from_PSE(uint8_t * pse) {
    std::vector<std::vector<uint8_t>> result;

    if(pse[0] != 0x6F)
        return result;

    int len = pse[1];
    int i = 2;
    while(i < len + 2) {
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
                            std::cout << "Found Application template at " << i << " until " << app_end << std::endl;
                            i += 2;

                            while(i < app_end) {
                                if(pse[i] == 0x4F) { // Application ID
                                    int appid_len = pse[i + 1];
                                    int appid = i + 2;
                                    int appid_end = appid + appid_len;

                                    std::vector<uint8_t> AID(pse + appid, pse + appid_end);

                                    std::cout << "Found AID of len " << appid_len << " at " << i << ": ";
                                    show(AID);
                                    std::cout << std::endl;

                                    result.push_back(AID);

                                    i = appid_end;
                                } else if(pse[i] == 0x50) { // Application Label
                                    int label_len = pse[i + 1];
                                    int label = i + 2;
                                    int label_end = label + label_len;

                                    std::string app_label( (char*)(pse + label) , label_len);
                                    std::cout << "Found an Application label " << app_label << std::endl;

                                    i = label_end;
                                } else if(pse[i] == 0x9F && pse[i + 1] == 0x0A) { // Application Selection Registered Proprietary Data
                                    i += 3 + pse[i + 2]; // ignored
                                } else {
                                    i += 2 + pse[i + 1];
                                }
                            }
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
}

std::vector<std::vector<uint8_t>> try_known_AIDs(nfc_device* pnd) {
    std::vector<std::vector<uint8_t>> result;
    return result;
}

void decode_TLV(std::vector<uint8_t> data) {
    std::cout << "https://emvlab.org/tlvutils/?data="; show(data); std::cout << std::endl;

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
            printf("PAN:");
            
            for (j=0;j<8;j++) {
                if (j%2==0) printf(" ");
                c = output[j];
                if (MASKED&j>=1&j<=5) {
                    printf("**");
                }
                else {
                    printf("%02x",c&0xff);
                }
            }
            printf("\n");
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
        //show(szRx, abtRx);
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
}

void try_reading_sector(nfc_device* pnd, uint8_t p1, uint8_t p2) {
    auto resp = read_record(pnd, p1, p2);
    if ( resp[1]==0x6a && (resp[2]==0x81 || resp[2]==0x82 || resp[2]==0x83)) {
        // File or application not found
        return;
    }
    else if ( resp[1]==0x6a && resp[2]==0x86) {
        // Wrong parameters
        return;
    }
    else{
        printf("\n-------------------------------------\n");
        printf("> READ RECORD %02x-%02x suceeded\n",p1,p2);
        //show(resp); printf("\n");
        decode_TLV(std::vector<uint8_t>( resp.begin() + 1, resp.end() ) );
    }
}

static void close(nfc_device*pnd, nfc_context* context) {
    printf("[-] Closing device\n");
    if(pnd)
        nfc_close(pnd);
    if(context)
        nfc_exit(context);
}

int main(int argc, char **argv) {
    nfc_context *context;
    nfc_device* pnd;
    nfc_modulation nm;
    nfc_target ant[1];
    
    printf("[-] Connecting to the reader...\n");
    nfc_init(&context);
    pnd = nfc_open(context,NULL);
    if (pnd == NULL) {
        printf("[x] Unable to connect to NFC device.\n");
        close(pnd, context);
        return(1);
    }

    printf("[+] Connected to NFC reader\n");
    nfc_initiator_init(pnd);

    // Checking card type...
    printf("[-] Looking for known card types...\n");
    
    auto resp = try_starting(pnd);
    if(!resp.empty()) {

        printf("[-] Finding out AIDS\n");

        std::vector<std::vector<uint8_t>> AIDs;

        std::vector<uint8_t> PSE = {0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31};
        std::vector<uint8_t> PPSE = {0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31};

        resp = select_AID(pnd, PPSE);
        if(resp.empty())
            resp = select_AID(pnd, PSE);
        if(!resp.empty()) {
            std::cout << "Got PSE/PPSE: \n"; show(resp); std::cout << std::endl;
            AIDs = get_AIDs_from_PSE(resp.data() + 1);
        } else {
            AIDs = try_known_AIDs(pnd);
        }

        std::cout << "Found " << AIDs.size() << " AIDs" << std::endl;
        for(auto AID: AIDs) {
            select_AID(pnd, AID);
            // Looking for data in the records
            for (int p1 = 0; p1 <= 10; p1 += 1) {
                for (int p2 = 12; p2 <= 28; p2 += 8) {
                    try_reading_sector(pnd, p1, p2);
                }
            }
        }
    }

    close(pnd, context);
    return(0);
}