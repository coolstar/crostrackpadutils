#if !defined(_CROSTP_COMMON_H_)
#define _CROSTP_COMMON_H_

//
//These are the device attributes returned by vmulti in response
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//

#define CYAPA_PID              0x0004
#define CYAPA_VID              0x04B4

#define ELAN_PID               0x0004
#define ELAN_VID               0x04F3

#define ATMEL_PID               0x0004
#define ATMEL_VID               0x03EB

#define SYNA_PID               0x0004
#define SYNA_VID               0x06CB

//
// These are the report ids
//

#define REPORTID_FEATURE        0x02
#define REPORTID_RELATIVE_MOUSE 0x04
#define REPORTID_TOUCHPAD       0x05
#define REPORTID_SCROLL			0x06
#define REPORTID_KEYBOARD       0x07
#define REPORTID_SCROLLCTRL		0x08
#define REPORTID_SETTINGS		0x09

#define SETTING_INFO			255;

#pragma pack(1)
typedef struct _CROSTP_SETTINGS_REPORT
{

	BYTE        ReportID;

	BYTE		SettingsRegister;

	BYTE		SettingsValue;

} CrosTpSettingsReport;
#pragma pack()

#pragma pack(1)
typedef struct _CROSTP_INFO_REPORT
{

	BYTE        ReportID;

	BYTE		Value[64];

} CrosTpInfoReport;
#pragma pack()

#endif