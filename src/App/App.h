#ifndef _APP_H_
#define _APP_H_

#include "sgx_eid.h"     /* sgx_enclave_id_t */

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

#if defined(__cplusplus)
extern "C" {
#endif

extern sgx_enclave_id_t globalEnclaveID;    /* global enclave id */

#if defined(__cplusplus)
}
#endif

# define TOKEN_FILENAME   "enclave.token"
# define ENCLAVE_FILENAME "enclave.signed.so"


#endif /* !_APP_H_ */
