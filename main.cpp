#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

//function prototype - PaintRectangle
int PaintRectangle(SHORT left,
	SHORT top,
	USHORT width,
	USHORT height,
	COLORREF color,
	HDC hDstDC);

//declare window procedure
LRESULT WINAPI MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main()
{
	HINSTANCE hInstance = GetModuleHandle(0);

	MSG msg;	//a message struct instance
	WNDCLASS wc;	//a window class instance

	//GDI related variables
	HWND hWnd;	//handle to the main window
	HBITMAP hBackbuffer;	//handle to the backbuffer bitmap
	HDC hWndDC;	//handle to the device context of the main window
	HDC hBackbufferDC;	//handle to the device context of the backbuffer

	//rectangle variables
	int left = 0, top = 0;	//upper left corner
	USHORT width = 32, height = 24;	//size of the rectangle
	COLORREF color = RGB(255, 255, 255);	//create a 32-bit color using the RGB macro

	//set the member variables of the main window class instance
	wc.lpszClassName = TEXT("MainWindowClass");	//string identifier for this class instance
	wc.lpfnWndProc = MainWndProc;	//the name (address) of the window procedure

	/*	CS_HREDRAW and CS_VREDRAW specifies the window should be redrawn,
		both when resized horizontally and vertically.
		hInstance specifies program instance a created window belongs to.	*/
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;

	/*	Load the icon displayed in the corner of the window,
		the first argument specifies the HINSTANCE of the icon resource,
		since IDI_WINLOGO is a standard icon ID no HINSTANCE is specified.
		Also load the mouse cursor displayed when hovering over the window.	*/
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	//brushes are used to specify fill color (in this case the background)
	//the the color of a window is explicitly converted into a HBRUSH
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;	//name of window menu, we won't be creating one so it's NULL
	//the following two members specifies extra bytes to allocate for the window instance
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	if (RegisterClass(&wc) == 0)
	{
		//we have no window yet thus the HWND for the messagebox is set to NULL
		//MB_ICONERROR specifies an error icon should be displayed in the messagebox
		MessageBox(NULL, TEXT("RegisterClass failed."), TEXT("Double buffering"), MB_ICONERROR);
		return 0;	//no window has been created so return without destroying it
	}

	/*	create the window using the registered class,
		the first message for this window is sent now,
		it was the WM_CREATE message (see MainWndProc)	*/
	hWnd = CreateWindow(TEXT("MainWindowClass"),	//our window class identifier
		TEXT("Double buffering"),	//caption
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0,		//left edge position
		0,		//top edge position
		WINDOW_WIDTH,	//width of the window
		WINDOW_HEIGHT,	//height of the window
		NULL,	//parent HWND is NULL since this is the only window
		NULL,	//handle to the menu of the window
		hInstance,	//specifies which program instance window shall belong to
		NULL);
	//the last parameter can be a pointer to be sent with the WM_CREATE message

//test whether the window failed to be created - in that case return 0
	if (hWnd == NULL)	//if no handle was recieved
	{
		MessageBox(NULL, TEXT("CreateWindow failed."), TEXT("Double buffering"), MB_ICONERROR);
		return 0;
	}


	/*	Retrieve the window device context,
		and create a device context compatible with it.
		Then select it to describe hBackbuffer.	*/
	hWndDC = GetDC(hWnd);
	hBackbuffer = CreateCompatibleBitmap(hWndDC, WINDOW_WIDTH, WINDOW_HEIGHT);
	hBackbufferDC = CreateCompatibleDC(hWndDC);
	SelectObject(hBackbufferDC, hBackbuffer);

	//message loop
	while (true)
	{
		//peek messages for all windows belonging to this thread
		//PM_REMOVE specifies that
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)	//the window has been destroyed
				break;

			DispatchMessage(&msg); //send the message to the window procedure
		}
		else
		{
			//idle process - do the drawing here
			if (left >= WINDOW_WIDTH)
			{
				//randomize the color of the rectangle
				color = RGB(rand() % 255, rand() % 255, rand() % 255);

				//randomize the width and height of the rectangle
				width = rand() % WINDOW_WIDTH;
				height = rand() % WINDOW_HEIGHT;

				//randomize the position of the rectangle
				left = -width;
				top = rand() % (WINDOW_HEIGHT - height);
			}
			else
			{
				left += 2;	//move the rectangle to the right
			}

			//clear the backbuffer by painting a black (0) rectangle over it
			PaintRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, hBackbufferDC);
			//paint the moving rectangle onto the backbuffer
			PaintRectangle(left, top, width, height, color, hBackbufferDC);
			//copy the whole backbuffer to the main window
			BitBlt(hWndDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
				hBackbufferDC, 0, 0,
				SRCCOPY);

			//drawing is complete, pause the program for 30 ms by calling Sleep
			//this limits the speed and lowers the CPU usage
			Sleep(20);
		}
	}	//end of message loop

	//this should be done before exiting the program:
	ReleaseDC(hWnd, hWndDC); //retrieved device contexts are just released
	DeleteDC(hBackbufferDC); //created device contexts must be deleted
	DeleteObject(hBackbuffer); //created objects must be deleted

	//return the last handled message to the caller (exit the program)
	return msg.wParam;
}	//end of WinMain

LRESULT WINAPI MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//switch statement to determine the current message
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);	//destroys this window and send WM_DESTROY message
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);	//send WM_QUIT message with the return code 0
		return 0;
	}

	//send any unhandled messages to the default window procedure by calling DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int PaintRectangle(SHORT left, SHORT top, USHORT width, USHORT height, COLORREF color, HDC hDstDC)
{
	if (hDstDC == NULL)
		return -1;	//no device context specified - return

	int returnValue;

	//RECT struct instance to paint
	RECT rect = { left, top, left + width, top + height };
	HBRUSH hBrush;

	hBrush = CreateSolidBrush(color);

	returnValue = FillRect(hDstDC, &rect, hBrush);

	DeleteObject(hBrush);

	return returnValue;
}