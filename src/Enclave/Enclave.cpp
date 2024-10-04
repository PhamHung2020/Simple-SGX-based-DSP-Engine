#include <unistd.h>
#include <vector>
#include <string>
#include "Enclave_t.h"

#include "Enclave/Enclave.h"
#include "Enclave/tasks.h"
#include "Enclave/enclave_operators.h"

char decryptedData[10000];
char decryptedData1[1000];
char decryptedData2[1000];

const size_t size = 64;
char from[size];
char to[64];

void testOpenSslOCall() {
    std::vector<std::string> plainTexts {
            "KI#CtmQE^M(5g!OY",
            "#{My1T3Yw+f$Sha@",
            "l$^C#s79ZV{NX*xb",
            "DUxf!$wr1{7m{Cfd",
            "TnWRgbel+h%6c5v#",
            "qtwqO6y6{YJcr6RO",
            "{2ZJ7A}m+RtVI7Xo",
            "csap6b)&6zhjb(f7",
            "0%x)MpayOEA0*C!$",
            "VJ#D3Cf3v(WOFNDV",
            "5YXh6WPr9j5GCVB{",
            "g}RH!_gU1iO^afeD",
            "%m%3(Ym#z}!yM+%R",
            "x7Q}bh$o+(N@MJqH",
            "}Zgi4tL_Vo+_w&uk",
            "$5Q9(7L0Xws##(%T",
            "mg66%wiMO3Qdbx}F",
            "WHU!}d2lIhSrh@3&",
            "xXlxpn(!L$imYpfR",
            "eE{GI_jh*D+4VBLI",
            "Ll}5cKBKBauwCcEs",
            "Gep0ypT(JjF+A4Iw",
            "#wiUuQkdy!Jt62g4",
            "tacRE08T^!OEh6ME",
            "u8JRqqZ{M$itEiFd",
            "GlHW5yVf^4Ag2buS",
            "SksZ2(V5ST&%0P!E",
            "BzYn{f_LFvzFWSwQ",
            "PGNl#74&}ogv_ptD",
            "9tfN}4&krHmpWgwh",
            "}udr%1Jx24+nmna_",
            "BE(0rjKeLeY{sT@w",
            "Q0DbDyL!KZUU^N7z",
            "FTa#5Z@4Sd1Na{K&",
            "*Ks2EuzutcFt3L$8",
            "XOXrNgYzKCPzbKMU",
            "&nbphhszjidEz*Cv",
            "AKiX3tBzv#2G15IB",
            "MBH^$6J6DUrAhjlk",
            "2(T$SsYQwsnfMnv^",
            "LDKFLnoe%cZ4lS4j",
            "i%0q+IW7b93L&%XK",
            "Ee{#{AaUTssxCKoy",
            "(t8{DUZGGdRNQ^z%",
            "Vr!pC%Zq6#c@dWmD",
            "4XqBv2a6Ptc%EEm}",
            "NBEVUpFvU2V6gQPh",
            "Uz2}EgvncI5_B1_^",
            "BjiMqTc&yYWQSOa}",
            "r}in9$eoTjJHEjIJ",
            ")LtQ)bojgHoX0ILi",
            "ziJ)mQuCixf_RKKw",
            "imird6^pKBd723II",
            "EkCFsuUTY{AowhBO",
            "}#F@jG6XYzV0+slp",
            "XGZeA#2SqPZpuEnO",
            "jGu^rU6U^RT_&dno",
            "XnDeX{7A*alvGZds",
            "aTD2KiC&Xt5LgtFt",
            "+1C{D1}sAN8W9QWq",
            "ERv6#}jfs63Jw!H8",
            "ImNk9p_%(MFZ})nO",
            "JjN(tPyWBPmTrUqq",
            "$s10#HhqjvOeCSUc",
            "oqHAqg82Qkd{8IWx",
            "DfciYXXr+kOwt8tX",
            "(Xht0NO8+}#f%^74",
            "TA8Eak4X+MdwjhmT",
            "5TV2QLb{IT8J%e7(",
            "MjSV^iQyeIUDbAK*",
            "#$VKPGpYfVG_ytTC",
            "jOe1jioY)%h{RD#r",
            "wPXS5z&WG#aGD$FO",
            "_N*xZhs8Du9Su6u0",
            "60SigcV4%Oz&W7qA",
            "sb{xwNy045VMnYJg",
            "!__HeEp8rw&$!MGN",
            "+MDdmR7iOIIV3gTM",
            "Z@CRJ9^0nErvEM7_",
            "l1(!QACXrB8iuEbs",
            "J@3Qot$&Z3xLQCp{",
            "xYY5w}1z4z6YIDf7",
            "HFGkWFEacSDl6lor",
            "*MyTRiuClGi4_iCJ",
            "77gB3&U0a{aMGXMS",
            "i#vU(tt$sQk(8HRT",
            "{i^*yphC3^@Gzr0z",
            "kwo+WsJJ@MBl1P4}",
            "TPludKihT{bRqmA$",
            "tN5p&RA8EYs&pc7v",
            "WdutAY^q425QtSTm",
            "K&vdZ#R8}Q0LsNR!",
            "^WwGg5AB^@!UI&_S",
            "LJ^Eq1pHkfyPSB6z",
            "cQ6UKfi{*C^Mw}oX",
            "_udNGuVMEY4hwzTN",
            "(m8(HXPn_9pWzZ{G",
            "(&+gV(h#1_$BXG9g",
            "ur4kqqs&ZDL#LnM2",
            "G!*UvBxV#hG{y0A4",
    };

    char encryptedData[10000];
    int dataSize = 16;
    for (int i = 0; i < 1000000; ++i) {
        auto index = i % 100;

//        print("Plain text: %s\n", plainTexts[index].c_str());
        openSslAes128GcmEncrypt(
                (unsigned char *) plainTexts[index].c_str(),
                dataSize, NULL, 0,
                const_cast<unsigned char *>(SGX_AES_GCM_KEY),
                const_cast<unsigned char *>(SGX_AES_GCM_IV), SGX_AESGCM_IV_SIZE,
                (unsigned char *)(encryptedData + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE),
                (unsigned char *)(encryptedData));

        oCallRecordStartTime();

//        openSslAes128GcmDecrypt(
//                (unsigned char*)encryptedData + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
//                dataSize,
//                NULL, 0,
//                (unsigned char*)encryptedData,
//                (unsigned char *)(SGX_AES_GCM_KEY),
//                (unsigned char*)(SGX_AES_GCM_IV),
//                SGX_AESGCM_IV_SIZE,
//                (unsigned char*)decryptedData);
        aesGcmDecrypt(encryptedData, dataSize, decryptedData, dataSize);

        oCallRecordEndTime();

//        decryptedData[dataSize+1] = '\0';
//        print("Decrypted text: %s\n", decryptedData);
    }
}

void testMemCpy() {
    memcpy(to, from, 64);
}

void testMemCpy2(char* outMem) {
//    for (int i = 0; i < size; ++i) {
//        from[i] = (char )i;
//    }
    memcpy(outMem, from, size);
}

int ecallSum(int a, int b) {
    oCallRecordStartTime();
    int c = a + b;
    oCallRecordEndTime();
    return c;
}

static inline void FastCall_wait_ECall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)  __attribute__((always_inline));
static inline void FastCall_wait_ECall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)
{
    static int i = 0;
    // char* data = new char[fastCallData->data_buffer->data_size];
    while(true)
    {
        // if (!fastCallData->keepPolling) {
        //     break;
        // }
//        oCallRecordStartTime();
        char* data;
        // sgx_spin_lock((&fastCallData->spinlock));
        if (circular_buffer_pop(fastCallData->data_buffer, (void**)&data) == 0)
        {
            // sgx_spin_unlock((&fastCallData->spinlock));
            if (callId < callTable->numEntries)
            {
                callTable->callbacks[callId](data);
            }

            int next = fastCallData->data_buffer->tail + 1;
            if (next >= fastCallData->data_buffer->maxlen)
            {
                next = 0;
            }
            fastCallData->data_buffer->tail = next;
//            sgx_spin_lock(&fastCallData->data_buffer->lock_count);
//            fastCallData->data_buffer->popped_count += 1;
//            sgx_spin_unlock(&fastCallData->data_buffer->lock_count);

//            oCallRecordEndTime();
            continue;
        }

        if (!fastCallData->keepPolling) {
            break;
        }
        // sgx_spin_unlock((&fastCallData->spinlock));

        // for( i = 0; i<3; ++i)
        //     _mm_pause();
    }

    if (callId < callTable->numEntries)
    {
        callTable->callbacks[callId](NULL);
    }

    // delete[] data;
}

static inline void FastCall_wait_decrypt_ECall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId, char* decryptedData)  __attribute__((always_inline));
static inline void FastCall_wait_decrypt_ECall(FastCallStruct *fastCallData, FastCallTable* callTable, uint16_t callId)
{
    static int i = 0;
    char* data;
    int started = 0;
    uint32_t failedTime = 1;
    const int dataSize = fastCallData->data_buffer->data_size - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE;
    while(true)
    {
//        oCallRecordStartTime();
        if (circular_buffer_pop(fastCallData->data_buffer, (void**)&data) == 0)
        {
            started = 1;
            if (callId < callTable->numEntries)
            {
                // AES of SDK
                aesGcmDecrypt(
                        data,
                        dataSize,
                        decryptedData,
                        dataSize);

                // AES of OpenSSL
//                openSslAes128GcmDecrypt(
//                        (unsigned char*)data + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
//                        dataSize,
//                        NULL, 0,
//                        (unsigned char*)data,
//                        (unsigned char *)(SGX_AES_GCM_KEY),
//                        (unsigned char*)(SGX_AES_GCM_IV),
//                        SGX_AESGCM_IV_SIZE,
//                        (unsigned char*)decryptedData
//                );
                decryptedData[dataSize] = '\0';
                callTable->callbacks[callId](decryptedData);
            }

            int next = fastCallData->data_buffer->tail + 1;
            if (next >= fastCallData->data_buffer->maxlen)
            {
                next = 0;
            }
            fastCallData->data_buffer->tail = next;
//            oCallRecordEndTime();
            continue;
        }

        if (started) {
            failedTime++;
        }

        if (!fastCallData->keepPolling) {
            break;
        }
    }

    if (callId < callTable->numEntries)
    {
        callTable->callbacks[callId](NULL);
    }

    fastCallData->data_buffer->popped_count = failedTime;
}

void EcallStartResponder(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

//    FastCall_wait(fastECallData, &callTable, callId);
    FastCall_wait_ECall(fastECallData, &callTable, callId);
}

void EcallStartResponderWithDecryption(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

//    FastCall_wait_decrypt(fastECallData, &callTable, callId, decryptedData);
    FastCall_wait_decrypt_ECall(fastECallData, &callTable, callId);

    print("Failed time: %d\n", fastECallData->data_buffer->popped_count);
}

void EcallStartResponder2(FastCallStruct* fastECallData1, FastCallStruct* fastECallData2, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

//    FastCall_wait(fastECallData, &callTable, callId);
    FastCall_wait_2(fastECallData1, fastECallData2, &callTable, callId);
}

void EcallStartResponder2WithDecryption(FastCallStruct* fastECallData1, FastCallStruct* fastECallData2, FastCallStruct* fastOCallData, const uint16_t callId)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait_2_decrypt(fastECallData1, fastECallData2, &callTable, callId, decryptedData1, decryptedData2);
}

void EcallStartResponderWithHotCall(FastCallStruct* fastECallData, FastCallStruct* fastOCallData, const uint16_t callId, HotCall* hotCall)
{
    globalFastOCall = fastOCallData;
    fastOCallBuffer = fastOCallData->data_buffer;

    FastCallTable callTable;
    callTable.numEntries = TASK_COUNT;
    callTable.callbacks  = callbacks;

    FastCall_wait_hotcall(fastECallData, &callTable, callId, hotCall);
}
