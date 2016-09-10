#if !defined(_CROSTP_CLIENT_H_)
#define _CROSTP_CLIENT_H_

#include "crostpcommon.h"

typedef struct _crostp_client_t* pcrostp_client;

USHORT getVendorID();

pcrostp_client crostp_alloc(void);

void crostp_free(pcrostp_client vmulti);

BOOL crostp_connect(pcrostp_client vmulti);

void crostp_disconnect(pcrostp_client vmulti);

BOOL crostp_read_message(pcrostp_client vmulti, CrosTpInfoReport* pReport);

BOOL crostp_write_message(pcrostp_client vmulti, CrosTpSettingsReport* pReport);

#endif