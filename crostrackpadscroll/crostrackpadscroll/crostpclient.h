#if !defined(_CROSTP_CLIENT_H_)
#define _CROSTP_CLIENT_H_

#include "crostpcommon.h"

typedef struct _crostp_client_t* pcrostp_client;

pcrostp_client crostp_alloc(void);

void crostp_free(pcrostp_client crostp);

BOOL crostp_connect(pcrostp_client crostp);
BOOL crostp_connect2(pcrostp_client crostp);

void crostp_disconnect(pcrostp_client crostp);
void crostp_disconnect2(pcrostp_client crostp);

BOOL crostp_read_message(pcrostp_client crostp, CrosTpScrollReport* pReport);
BOOL crostp_write_message(pcrostp_client crostp, CrosTpScrollControlReport* pReport);
BOOL crostp_write_setting(pcrostp_client crostp, CrosTpSettingsReport* pReport);

void CrosTpClientChangeSetting(pcrostp_client crostp, int settingRegister, int settingValue);

#endif