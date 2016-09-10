#if !defined(_CROSTP_COMMON_H_)
#define _CROSTP_COMMON_H_

//
//These are the device attributes returned by crostp in response
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

#define CROSTP_VERSION          0x0003

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

#pragma pack(1)
typedef struct _CROSTP_SETTINGS_REPORT
{

	BYTE        ReportID;

	BYTE		SettingsRegister;

	BYTE		SettingsValue;

} CrosTpSettingsReport;
#pragma pack()

//
// Scroll specific report information
//
#pragma pack(1)
typedef struct _CROSTP_SCROLL_REPORT
{

	BYTE        ReportID;

	BYTE		Flag;

	USHORT        Touch1XValue;

	USHORT        Touch1YValue;

	USHORT        Touch2XValue;

	USHORT        Touch2YValue;

} CrosTpScrollReport;
#pragma pack()

#pragma pack(1)
typedef struct _CROSTP_SCROLL_CONTROL_REPORT
{

	BYTE        ReportID;

	BYTE		Flag;

} CrosTpScrollControlReport;
#pragma pack()

#endif