#include "stdafx.h"
#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "crostpclient.h"

#if __GNUC__
#define __in
#define __in_ecount(x)
typedef void* PVOID;
typedef PVOID HDEVINFO;
WINHIDSDI BOOL WINAPI HidD_SetOutputReport(HANDLE, PVOID, ULONG);
#endif

typedef struct _crostp_client_t
{
	HANDLE hSettings;
} crostp_client_t;

//
// Function prototypes
//

HANDLE
SearchMatchingHwID(
	USAGE myUsagePage,
	USAGE myUsage
	);

HANDLE
OpenDeviceInterface(
	HDEVINFO HardwareDeviceInfo,
	PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
	USAGE myUsagePage,
	USAGE myUsage
	);

BOOLEAN
CheckIfOurDevice(
	HANDLE file,
	USAGE myUsagePage,
	USAGE myUsage
	);

BOOL
HidOutput(
	BOOL useSetOutputReport,
	HANDLE file,
	PCHAR buffer,
	ULONG bufferSize
	);

//
// Copied this structure from hidport.h
//

typedef struct _HID_DEVICE_ATTRIBUTES {

	ULONG           Size;
	//
	// sizeof (struct _HID_DEVICE_ATTRIBUTES)
	//
	//
	// Vendor ids of this hid device
	//
	USHORT          VendorID;
	USHORT          ProductID;
	USHORT          VersionNumber;
	USHORT          Reserved[11];

} HID_DEVICE_ATTRIBUTES, *PHID_DEVICE_ATTRIBUTES;

static USHORT TpVendorID = 0;

USHORT getVendorID() {
	return TpVendorID;
}

//
// Implementation
//

pcrostp_client crostp_alloc(void)
{
	return (pcrostp_client)malloc(sizeof(crostp_client_t));
}

void crostp_free(pcrostp_client crostp)
{
	free(crostp);
}

BOOL crostp_connect(pcrostp_client crostp)
{
	//
	// Find the HID devices
	//

	crostp->hSettings = SearchMatchingHwID(0xff00, 0x0003);
	if (crostp->hSettings == INVALID_HANDLE_VALUE || crostp->hSettings == NULL)
	{
		crostp_disconnect(crostp);
		return FALSE;
	}

	//
	// Set the buffer count to 10 on the setting HID
	//

	if (!HidD_SetNumInputBuffers(crostp->hSettings, 10))
	{
		printf("failed HidD_SetNumInputBuffers %d\n", GetLastError());
		crostp_disconnect(crostp);
		return FALSE;
	}

	return TRUE;
}

void crostp_disconnect(pcrostp_client crostp)
{
	if (crostp->hSettings != NULL)
		CloseHandle(crostp->hSettings);
	crostp->hSettings = NULL;
}

BOOL crostp_read_message(pcrostp_client vmulti, CrosTpInfoReport* pReport)
{
	ULONG bytesRead;

	//
	// Read the report
	//

	if (!ReadFile(vmulti->hSettings, pReport, sizeof(CrosTpInfoReport), &bytesRead, NULL))
	{
		printf("failed ReadFile %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL crostp_write_message(pcrostp_client crostp, CrosTpSettingsReport* pReport)
{
	ULONG bytesWritten;

	//
	// Write the report
	//

	if (!WriteFile(crostp->hSettings, pReport, sizeof(CrosTpSettingsReport), &bytesWritten, NULL))
	{
		printf("failed WriteFile %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

HANDLE
SearchMatchingHwID(
	USAGE myUsagePage,
	USAGE myUsage
	)
{
	HDEVINFO                  hardwareDeviceInfo;
	SP_DEVICE_INTERFACE_DATA  deviceInterfaceData;
	SP_DEVINFO_DATA           devInfoData;
	GUID                      hidguid;
	int                       i;

	HidD_GetHidGuid(&hidguid);

	hardwareDeviceInfo =
		SetupDiGetClassDevs((LPGUID)&hidguid,
			NULL,
			NULL, // Define no
			(DIGCF_PRESENT |
				DIGCF_INTERFACEDEVICE));

	if (INVALID_HANDLE_VALUE == hardwareDeviceInfo)
	{
		printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
		return INVALID_HANDLE_VALUE;
	}

	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	//
	// Enumerate devices of this interface class
	//

	printf("\n....looking for our HID device (with UP=0x%x "
		"and Usage=0x%x)\n", myUsagePage, myUsage);

	for (i = 0; SetupDiEnumDeviceInterfaces(hardwareDeviceInfo,
		0, // No care about specific PDOs
		(LPGUID)&hidguid,
		i, //
		&deviceInterfaceData);
	i++)
	{

		//
		// Open the device interface and Check if it is our device
		// by matching the Usage page and Usage from Hid_Caps.
		// If this is our device then send the hid request.
		//

		HANDLE file = OpenDeviceInterface(hardwareDeviceInfo, &deviceInterfaceData, myUsagePage, myUsage);

		if (file != INVALID_HANDLE_VALUE)
		{
			SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
			return file;
		}

		//
		//device was not found so loop around.
		//

	}

	printf("Failure: Could not find our HID device \n");

	SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);

	return INVALID_HANDLE_VALUE;
}

HANDLE
OpenDeviceInterface(
	HDEVINFO hardwareDeviceInfo,
	PSP_DEVICE_INTERFACE_DATA deviceInterfaceData,
	USAGE myUsagePage,
	USAGE myUsage
	)
{
	PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;

	DWORD        predictedLength = 0;
	DWORD        requiredLength = 0;
	HANDLE       file = INVALID_HANDLE_VALUE;

	SetupDiGetDeviceInterfaceDetail(
		hardwareDeviceInfo,
		deviceInterfaceData,
		NULL, // probing so no output buffer yet
		0, // probing so output buffer length of zero
		&requiredLength,
		NULL
		); // not interested in the specific dev-node

	predictedLength = requiredLength;

	deviceInterfaceDetailData =
		(PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(predictedLength);

	if (!deviceInterfaceDetailData)
	{
		printf("Error: OpenDeviceInterface: malloc failed\n");
		goto cleanup;
	}

	deviceInterfaceDetailData->cbSize =
		sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	if (!SetupDiGetDeviceInterfaceDetail(
		hardwareDeviceInfo,
		deviceInterfaceData,
		deviceInterfaceDetailData,
		predictedLength,
		&requiredLength,
		NULL))
	{
		printf("Error: SetupDiGetInterfaceDeviceDetail failed\n");
		free(deviceInterfaceDetailData);
		goto cleanup;
	}

	file = CreateFile(deviceInterfaceDetailData->DevicePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, // no SECURITY_ATTRIBUTES structure
		OPEN_EXISTING, // No special create flags
		0, // No special attributes
		NULL); // No template file

	if (INVALID_HANDLE_VALUE == file) {
		printf("Error: CreateFile failed: %d\n", GetLastError());
		goto cleanup;
	}

	if (CheckIfOurDevice(file, myUsagePage, myUsage)) {

		goto cleanup;

	}

	CloseHandle(file);

	file = INVALID_HANDLE_VALUE;

cleanup:

	free(deviceInterfaceDetailData);

	return file;

}


BOOLEAN
CheckIfOurDevice(
	HANDLE file,
	USAGE myUsagePage,
	USAGE myUsage)
{
	PHIDP_PREPARSED_DATA Ppd = NULL; // The opaque parser info describing this device
	HIDD_ATTRIBUTES                 Attributes; // The Attributes of this hid device.
	HIDP_CAPS                       Caps; // The Capabilities of this hid device.
	BOOLEAN                         result = FALSE;

	if (!HidD_GetPreparsedData(file, &Ppd))
	{
		printf("Error: HidD_GetPreparsedData failed \n");
		goto cleanup;
	}

	if (!HidD_GetAttributes(file, &Attributes))
	{
		printf("Error: HidD_GetAttributes failed \n");
		goto cleanup;
	}

	if (Attributes.VendorID == CYAPA_VID && Attributes.ProductID == CYAPA_PID ||
		Attributes.VendorID == ELAN_VID && Attributes.ProductID == ELAN_PID ||
		Attributes.VendorID == ATMEL_VID && Attributes.ProductID == ATMEL_PID ||
		Attributes.VendorID == SYNA_VID && Attributes.ProductID == SYNA_PID)
	{
		TpVendorID = Attributes.VendorID;
		
		if (!HidP_GetCaps(Ppd, &Caps))
		{
			printf("Error: HidP_GetCaps failed \n");
			goto cleanup;
		}

		if ((Caps.UsagePage == myUsagePage) && (Caps.Usage == myUsage))
		{
			printf("Success: Found my device.. \n");
			result = TRUE;
		}
	}

cleanup:

	if (Ppd != NULL)
	{
		HidD_FreePreparsedData(Ppd);
	}

	return result;
}

BOOL
HidOutput(
	BOOL useSetOutputReport,
	HANDLE file,
	PCHAR buffer,
	ULONG bufferSize
	)
{
	ULONG bytesWritten;
	if (useSetOutputReport)
	{
		//
		// Send Hid report thru HidD_SetOutputReport API
		//

		if (!HidD_SetOutputReport(file, buffer, bufferSize))
		{
			printf("failed HidD_SetOutputReport %d\n", GetLastError());
			return FALSE;
		}
	}
	else
	{
		if (!WriteFile(file, buffer, bufferSize, &bytesWritten, NULL))
		{
			printf("failed WriteFile %d\n", GetLastError());
			return FALSE;
		}
	}

	return TRUE;
}