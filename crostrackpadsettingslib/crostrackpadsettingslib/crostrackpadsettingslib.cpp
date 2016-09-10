// crostrackpadsettingslib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <Windows.h>
#include "crostpclient.h"
#include <stdio.h>

pcrostp_client client;

extern "C" {

	__declspec(dllexport) int ConnectCrosTpClient() {
		client = crostp_alloc();

		if (client == NULL)
		{
			return 2;
		}

		if (!crostp_connect(client))
		{
			crostp_free(client);
			return 3;
		}
		return 0;
	}

	__declspec(dllexport) int DisconnectCrosTpClient() {
		crostp_disconnect(client);
		crostp_free(client);

		return 0;
	}

	__declspec(dllexport) void CrosTpClientChangeSetting(int settingRegister, int settingValue) {
		_CROSTP_SETTINGS_REPORT settingsReport;
		settingsReport.ReportID = REPORTID_SETTINGS;
		settingsReport.SettingsRegister = settingRegister;
		settingsReport.SettingsValue = settingValue;
		crostp_write_message(client, &settingsReport);
	}

	__declspec(dllexport) char *CrosTpDriverVersion() {
		char driverVersion[64];

		_CROSTP_SETTINGS_REPORT settingsReport;
		settingsReport.ReportID = REPORTID_SETTINGS;
		settingsReport.SettingsRegister = SETTING_INFO;
		settingsReport.SettingsValue = 0;
		crostp_write_message(client, &settingsReport);

		_CROSTP_INFO_REPORT infoReport;
		crostp_read_message(client, &infoReport);

		strcpy_s(driverVersion, (char *)infoReport.Value);
		driverVersion[strlen((char *)infoReport.Value)] = '\0';
		return driverVersion;
	}

	__declspec(dllexport) char *CrosTpProductId() {
		char productID[64];

		_CROSTP_SETTINGS_REPORT settingsReport;
		settingsReport.ReportID = REPORTID_SETTINGS;
		settingsReport.SettingsRegister = SETTING_INFO;
		settingsReport.SettingsValue = 1;
		crostp_write_message(client, &settingsReport);

		_CROSTP_INFO_REPORT infoReport;
		crostp_read_message(client, &infoReport);

		strcpy_s(productID, (char *)infoReport.Value);
		productID[strlen((char *)infoReport.Value)] = '\0';
		return productID;
	}

	__declspec(dllexport) char *CrosTpFirmwareVersion() {
		char firmwareVersion[64];

		_CROSTP_SETTINGS_REPORT settingsReport;
		settingsReport.ReportID = REPORTID_SETTINGS;
		settingsReport.SettingsRegister = SETTING_INFO;
		settingsReport.SettingsValue = 2;
		crostp_write_message(client, &settingsReport);

		_CROSTP_INFO_REPORT infoReport;
		crostp_read_message(client, &infoReport);

		strcpy_s(firmwareVersion, (char *)infoReport.Value);
		return firmwareVersion;
	}

	__declspec(dllexport) int CrosTpType() {
		if (getVendorID() == CYAPA_VID)
			return 0;
		else if (getVendorID() == ELAN_VID)
			return 1;
		else if (getVendorID() == ATMEL_VID)
			return 2;
		else if (getVendorID() == SYNA_VID)
			return 3;
		return -1;
	}
}