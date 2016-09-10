// crostrackpadscroll.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <windows.h>
#include <shlobj.h>
#include "crostpclient.h"
#include "helperclasses.h"

using namespace std;

bool scrollNatural = true;
bool horizontalScroll = true;
float scrollMultiplier = 1.0f;
bool inertiaScroll = true;
bool zoomEnabled = true;

BOOL isPropertyValid(USHORT prop) {
	if (prop == 65535)
		return false;
	return true;
}

USHORT average(USHORT a1, USHORT a2) {
	return (a1 + a2) / 2;
}

bool isSameSign(int n1, int n2) {
	if (n1 == 0 || n2 == 0)
		return true;
	if (n1 > 0 && n2 > 0)
		return true;
	if (n1 < 0 && n2 < 0)
		return true;
	return false;
}

SimpleAverage<int, 32> dy_history;
SimpleAverage<int, 32> dx_history;
SimpleAverage<int, 32> dist_history;

int lastx1 = 0;
int lasty1 = 0;
int lastx2 = 0;
int lasty2 = 0;

float lastdistance = 0;

bool isTouchActive = false;

int momentumscrollsamplesmin = 3;

int momentumscrollcurrentx = 0;
int momentumscrollmultiplierx = 98;
int momentumscrolldivisorx = 100;
int momentumscrollrest1x = 0;
int momentumscrollrest2x = 0;

int momentumscrollcurrenty = 0;
int momentumscrollmultipliery = 98;
int momentumscrolldivisory = 100;
int momentumscrollrest1y = 0;
int momentumscrollrest2y = 0;

bool cancelDelayScroll = false;

bool isZooming;

pcrostp_client crostp;

void setScrollingCurrently(int enabled) {
	_CROSTP_SCROLL_CONTROL_REPORT report;
	report.ReportID = REPORTID_SCROLLCTRL;
	report.Flag = enabled;
	crostp_write_message(crostp, &report);
}

void disableScrollingDelayed() {
	Sleep(300);
	if (!cancelDelayScroll) {
		setScrollingCurrently(0);
		cancelDelayScroll = false;
	}
}

void disableScrollingDelayLaunch() {
	cancelDelayScroll = false;
	std::thread disableScrollingThread(disableScrollingDelayed);
	disableScrollingThread.detach();
}

int touchXCenter;
int touchYCenter;

POINT screenCenter;

int lastZoomX1, lastZoomY1, lastZoomX2, lastZoomY2;

void CancelZoom() {
	if (isZooming) {
		POINTER_TOUCH_INFO contact[2];
		memset(&contact[0], 0, sizeof(POINTER_TOUCH_INFO));
		memset(&contact[1], 0, sizeof(POINTER_TOUCH_INFO));

		//Check Pointer Id is taken as 0 for contact 0
		contact[0].pointerInfo.pointerType = PT_TOUCH;
		contact[0].pointerInfo.pointerId = 0;          //Id 0 for contact 0
		contact[0].pointerInfo.ptPixelLocation.y = lastZoomY1;
		contact[0].pointerInfo.ptPixelLocation.x = lastZoomX1;
		//Defining Touch flag and touchmask for contact 0
		contact[0].touchFlags = TOUCH_FLAG_NONE;
		contact[0].touchMask = TOUCH_MASK_CONTACTAREA;

		contact[0].rcContact.top = contact[0].pointerInfo.ptPixelLocation.y - 2;
		contact[0].rcContact.bottom = contact[0].pointerInfo.ptPixelLocation.y + 2;
		contact[0].rcContact.left = contact[0].pointerInfo.ptPixelLocation.x - 2;
		contact[0].rcContact.right = contact[0].pointerInfo.ptPixelLocation.x + 2;


		contact[1].pointerInfo.pointerType = PT_TOUCH;
		contact[1].pointerInfo.pointerId = 1;          //Id 0 for contact 1
		contact[1].pointerInfo.ptPixelLocation.y = lastZoomY2;
		contact[1].pointerInfo.ptPixelLocation.x = lastZoomX1;
		//Defining Touch flag and touchmask for contact 1
		contact[1].touchFlags = TOUCH_FLAG_NONE;
		contact[1].touchMask = TOUCH_MASK_CONTACTAREA;
		contact[1].rcContact.top = contact[1].pointerInfo.ptPixelLocation.y - 2;
		contact[1].rcContact.bottom = contact[1].pointerInfo.ptPixelLocation.y + 2;
		contact[1].rcContact.left = contact[1].pointerInfo.ptPixelLocation.x - 2;
		contact[1].rcContact.right = contact[1].pointerInfo.ptPixelLocation.x + 2;

		contact[0].pointerInfo.pointerFlags = POINTER_FLAG_UP;
		contact[1].pointerInfo.pointerFlags = POINTER_FLAG_UP;
		InjectTouchInput(2, contact);
		InjectTouchInput(0, NULL);
		SetCursorPos(screenCenter.x, screenCenter.y);
		isZooming = false;
	}
}

void ProcessZoom(_CROSTP_SCROLL_REPORT *report) {
	touchXCenter = average(report->Touch1XValue, report->Touch2XValue);
	touchYCenter = average(report->Touch1XValue, report->Touch2XValue);

	POINTER_TOUCH_INFO contact[2];
	memset(&contact[0], 0, sizeof(POINTER_TOUCH_INFO));
	memset(&contact[1], 0, sizeof(POINTER_TOUCH_INFO));

	//Check Pointer Id is taken as 0 for contact 0
	contact[0].pointerInfo.pointerType = PT_TOUCH;
	contact[0].pointerInfo.pointerId = 0;          //Id 0 for contact 0
	contact[0].pointerInfo.ptPixelLocation.y = screenCenter.y + (report->Touch1YValue - touchYCenter) / 2;
	contact[0].pointerInfo.ptPixelLocation.x = screenCenter.x + (report->Touch1XValue - touchXCenter) / 2;
	//Defining Touch flag and touchmask for contact 0
	contact[0].touchFlags = TOUCH_FLAG_NONE;
	contact[0].touchMask = TOUCH_MASK_CONTACTAREA;

	contact[0].rcContact.top = contact[0].pointerInfo.ptPixelLocation.y - 2;
	contact[0].rcContact.bottom = contact[0].pointerInfo.ptPixelLocation.y + 2;
	contact[0].rcContact.left = contact[0].pointerInfo.ptPixelLocation.x - 2;
	contact[0].rcContact.right = contact[0].pointerInfo.ptPixelLocation.x + 2;

	lastZoomX1 = contact[0].pointerInfo.ptPixelLocation.x;
	lastZoomY1 = contact[0].pointerInfo.ptPixelLocation.y;


	contact[1].pointerInfo.pointerType = PT_TOUCH;
	contact[1].pointerInfo.pointerId = 1;          //Id 0 for contact 1
	contact[1].pointerInfo.ptPixelLocation.y = screenCenter.y + (report->Touch2YValue - touchYCenter) / 2;
	contact[1].pointerInfo.ptPixelLocation.x = screenCenter.x + (report->Touch2XValue - touchXCenter) / 2;
	//Defining Touch flag and touchmask for contact 1
	contact[1].touchFlags = TOUCH_FLAG_NONE;
	contact[1].touchMask = TOUCH_MASK_CONTACTAREA;
	contact[1].rcContact.top = contact[1].pointerInfo.ptPixelLocation.y - 2;
	contact[1].rcContact.bottom = contact[1].pointerInfo.ptPixelLocation.y + 2;
	contact[1].rcContact.left = contact[1].pointerInfo.ptPixelLocation.x - 2;
	contact[1].rcContact.right = contact[1].pointerInfo.ptPixelLocation.x + 2;

	lastZoomX2 = contact[1].pointerInfo.ptPixelLocation.x;
	lastZoomY2 = contact[1].pointerInfo.ptPixelLocation.y;

	if (!isZooming) {
		contact[0].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
		contact[1].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
		isZooming = true;
		setScrollingCurrently(1);
	}
	else {
		contact[0].pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
		contact[1].pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
	}

	InjectTouchInput(2, contact);
}

int noScrollCounter = 0;
int zoomCounter = 0;

void ProcessScroll(_CROSTP_SCROLL_REPORT *report) {
	int x1 = report->Touch1XValue;
	int y1 = report->Touch1YValue;
	int x2 = report->Touch2XValue;
	int y2 = report->Touch2YValue;

	float distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

	int delta_x1 = x1 - lastx1;
	int delta_y1 = y1 - lasty1;

	int delta_x2 = x2 - lastx2;
	int delta_y2 = y2 - lasty2;

	float delta_dist = distance - lastdistance;

	if (delta_dist != 0) {
		dist_history.filter(delta_dist);
	}

	lastx1 = x1;
	lasty1 = y1;
	lastx2 = x2;
	lasty2 = y2;

	lastdistance = distance;

	int avgy = (delta_y1 + delta_y2) / 2;
	int avgx = (delta_x1 + delta_x2) / 2;

	if (isZooming) {
		ProcessZoom(report);
		return;
	}

	bool onlyScroll = false;

	if (abs(avgx) > 100) {
		avgx = 0;
		onlyScroll = true;
	}
	if (abs(avgy) > 100) {
		avgy = 0;
		onlyScroll = true;
	}
	if (zoomEnabled && abs(delta_dist) > abs(avgx) && abs(delta_dist) > abs(avgy) && abs(delta_dist) > 10 && !onlyScroll) {
		if (!isZooming) {
			zoomCounter++;
			if (zoomCounter > 3) {
				GetCursorPos(&screenCenter);
				zoomCounter = 0;
				ProcessZoom(report);
			}
			return;
		}
	}

	if (avgx == 0 && avgy == 0) {
		noScrollCounter++;
		if (noScrollCounter < 3)
			return;
	}
	else {
		noScrollCounter = 0;
		zoomCounter = 0;
	}

	if (abs(avgy) > abs(avgx)) {
		INPUT in;
		in.type = INPUT_MOUSE;
		in.mi.dx = 0;
		in.mi.dy = 0;
		in.mi.dwFlags = MOUSEEVENTF_WHEEL;
		in.mi.time = 0;
		in.mi.dwExtraInfo = 0;
		if (scrollNatural)
			in.mi.mouseData = avgy;
		else
			in.mi.mouseData = -avgy;

		if (!isSameSign(momentumscrollcurrenty, avgy)) {
			momentumscrollcurrenty = 0;
			momentumscrollrest1y = 0;
			momentumscrollrest2y = 0;
			dy_history.reset();
		}

		SendInput(1, &in, sizeof(in));
		dy_history.filter(avgy);
		dx_history.reset();
		dist_history.reset();
	}
	else {
		if (horizontalScroll) {
			INPUT in;
			in.type = INPUT_MOUSE;
			in.mi.dx = 0;
			in.mi.dy = 0;
			in.mi.dwFlags = MOUSEEVENTF_HWHEEL;
			in.mi.time = 0;
			in.mi.dwExtraInfo = 0;
			if (scrollNatural)
				in.mi.mouseData = -avgx;
			else
				in.mi.mouseData = avgx;
			SendInput(1, &in, sizeof(in));
		}

		if (!isSameSign(momentumscrollcurrentx, -avgx)) {
			momentumscrollcurrentx = 0;
			momentumscrollrest1x = 0;
			momentumscrollrest2x = 0;
			dx_history.reset();
		}

		dx_history.filter(-avgx);
		dy_history.reset();
		dist_history.reset();
	}
	cancelDelayScroll = true;
	setScrollingCurrently(1);
}

void scrollTimer() {
	if (momentumscrollcurrentx) {
		int dx = momentumscrollcurrentx / 10 + momentumscrollrest2x;

		if (abs(dx) > 7) {
			//dispatch the scroll event
			if (inertiaScroll && horizontalScroll) {
				INPUT in;
				in.type = INPUT_MOUSE;
				in.mi.dx = 0;
				in.mi.dy = 0;
				in.mi.dwFlags = MOUSEEVENTF_HWHEEL;
				in.mi.time = 0;
				in.mi.dwExtraInfo = 0;
				if (scrollNatural)
					in.mi.mouseData = (dx / 7);
				else
					in.mi.mouseData = -(dx / 7);
				SendInput(1, &in, sizeof(in));
			}

			momentumscrollrest2x = dx % 1;

			momentumscrollcurrentx = momentumscrollcurrentx * momentumscrollmultiplierx + momentumscrollrest1x;
			momentumscrollrest1x = momentumscrollcurrentx % momentumscrolldivisorx;
			momentumscrollcurrentx /= momentumscrolldivisorx;
		}
		else {
			momentumscrollcurrentx = 0;
		}
	}

	if (momentumscrollcurrenty) {
		int dy = momentumscrollcurrenty / 10 + momentumscrollrest2y;

		if (abs(dy) > 7) {
			//dispatch the scroll event
			if (inertiaScroll) {
				INPUT in;
				in.type = INPUT_MOUSE;
				in.mi.dx = 0;
				in.mi.dy = 0;
				in.mi.dwFlags = MOUSEEVENTF_WHEEL;
				in.mi.time = 0;
				in.mi.dwExtraInfo = 0;
				if (scrollNatural)
					in.mi.mouseData = (dy / 7);
				else
					in.mi.mouseData = -(dy / 7);
				SendInput(1, &in, sizeof(in));
			}

			momentumscrollrest2y = dy % 1;

			momentumscrollcurrenty = momentumscrollcurrenty * momentumscrollmultipliery + momentumscrollrest1y;
			momentumscrollrest1y = momentumscrollcurrenty % momentumscrolldivisory;
			momentumscrollcurrenty /= momentumscrolldivisory;
		}
		else {
			momentumscrollcurrenty = 0;
		}
	}

	if (momentumscrollcurrentx == 0 && momentumscrollcurrenty == 0)
		disableScrollingDelayLaunch();
	else {
		cancelDelayScroll = true;
		setScrollingCurrently(1);
	}
}

void scrollTimerLoop() {
	while (true) {
		scrollTimer();
		Sleep(10);
	}
}

std::vector<std::string> split(const std::string &text, char sep) {
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

std::string getPath(int csidl) {
	char out[MAX_PATH];
	if (SHGetSpecialFolderPathA(NULL, out, csidl, 0)) {
		return out;
	}
	else {
		return "";
	}
}

void applyDriverSettings() {
	crostp_connect2(crostp);

	ifstream file = ifstream(getPath(CSIDL_APPDATA) + "\\crostp-driversettings.txt");
	if (!file)
		return;
	stringstream buffer;
	buffer << file.rdbuf();

	string settings = buffer.str();

	vector<string> lines = split(settings, '\n');

	/* do something with data in buffer */

	for (int i = 0;i < lines.size();i++) {
		string line = lines[i];
		if (line.length() < 3)
			continue;

		vector<string> parameters = split(line, ' ');

		int cmd = atoi(parameters[0].c_str());
		int value = atoi(parameters[1].c_str());
		CrosTpClientChangeSetting(crostp, cmd, value);
	}

	crostp_disconnect2(crostp);
}

void applySettings(string settings) {
	vector<string> lines = split(settings, '\n');

	/* do something with data in buffer */

	for (int i = 0;i < lines.size();i++) {
		string line = lines[i];
		if (line.length() < 3)
			continue;

		vector<string> parameters = split(line, ' ');

		int cmd = atoi(parameters[0].c_str());
		int value = atoi(parameters[1].c_str());;

		switch (cmd) {
		case 0:
			scrollNatural = value;
			break;
		case 1:
			horizontalScroll = value;
			break;
		case 2:
			scrollMultiplier = (float)value / 10.0f;
			break;
		case 3:
			inertiaScroll = value;
			break;
		case 4:
			zoomEnabled = value;
			break;
		}
	}
}

void applySettings() {
	ifstream file = ifstream(getPath(CSIDL_APPDATA) + "\\crostp-scrollsettings.txt");
	if (!file)
		return;
	stringstream buffer;
	buffer << file.rdbuf();

	string settings = buffer.str();
	applySettings(settings);
}

void settingsChangeLoop() {
	HANDLE hPipe;
	char buffer[1024];
	DWORD dwRead;


	hPipe = CreateNamedPipe(TEXT("\\\\.\\Pipe\\CrosTrackpadSettings"),
		PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
		PIPE_WAIT,
		1,
		1024 * 16,
		1024 * 16,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
	while (hPipe != INVALID_HANDLE_VALUE)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)   // wait for someone to connect to the pipe
		{
			while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
			{
				/* add terminating zero */
				buffer[dwRead] = '\0';

				string data = string(buffer);
				applySettings(data);
			}
		}

		DisconnectNamedPipe(hPipe);
	}
}

int WINAPI wWinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	PWSTR       lpCmdLine,
	int         nCmdShow
	)
{
	InitializeTouchInjection(2, TOUCH_FEEDBACK_INDIRECT);

	crostp = crostp_alloc();

	if (crostp == NULL)
	{
		return 2;
	}

	if (!crostp_connect(crostp))
	{
		crostp_free(crostp);
		return 3;
	}

	applySettings();
	applyDriverSettings();

	std::thread timerLoop(scrollTimerLoop);
	timerLoop.detach();

	std::thread settingsLoop(settingsChangeLoop);
	settingsLoop.detach();

	while (true) {
		_CROSTP_SCROLL_REPORT report;
		crostp_read_message(crostp, &report);
		if (report.Flag == 1) {
			dist_history.reset();
			CancelZoom();
			isZooming = false;

			momentumscrollcurrentx = 0;
			momentumscrollrest1x = 0;
			momentumscrollrest2x = 0;
			dx_history.reset();

			momentumscrollcurrenty = 0;
			momentumscrollrest1y = 0;
			momentumscrollrest2y = 0;
			dy_history.reset();
			isTouchActive = false;

			disableScrollingDelayLaunch();
		} 
		else if (isPropertyValid(report.Touch1XValue) &&
			isPropertyValid(report.Touch1YValue) &&
			isPropertyValid(report.Touch2XValue) &&
			isPropertyValid(report.Touch2YValue)) {
			ProcessScroll(&report);

			isTouchActive = true;
		}
		else {
			CancelZoom();

			if (isTouchActive) {
				if (dx_history.count() > momentumscrollsamplesmin) {
					int scrollx = dx_history.sum() * 10;
					if (isSameSign(momentumscrollcurrentx, scrollx))
						momentumscrollcurrentx += scrollx;
					else
						momentumscrollcurrentx = scrollx;
					momentumscrollrest1x = 0;
					momentumscrollrest2x = 0;
				}

				if (dy_history.count() > momentumscrollsamplesmin) {
					int scrolly = dy_history.sum() * 10;
					if (isSameSign(momentumscrollcurrenty, scrolly))
						momentumscrollcurrenty += scrolly;
					else
						momentumscrollcurrenty = scrolly;
					momentumscrollrest1y = 0;
					momentumscrollrest2y = 0;
				}

				dx_history.reset();
				dy_history.reset();
				isTouchActive = false;
			}

			lastx1 = 0;
			lasty1 = 0;
			lastx2 = 0;
			lasty2 = 0;
		}
	}

	crostp_disconnect(crostp);

	crostp_free(crostp);

    return 0;
}

