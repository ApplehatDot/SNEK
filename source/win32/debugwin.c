#include <windows.h>
#include <stdio.h>

HWND hwndDebug = NULL;  // Uchwyt do okna debugowego

void CreateDebugWindow(HINSTANCE hInstance, int nCmdShow) {
    // Utworzenie okna bez u�ycia klasy
    hwndDebug = CreateWindow("STATIC", NULL,
        WS_CAPTION | WS_VISIBLE | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 100,
        NULL, NULL, hInstance, NULL);

    if (!hwndDebug) {
        MessageBox(NULL, "Nie mo�na utworzy� okna debugowego!", "B��d", MB_ICONERROR);
        return;
    }

    // Ustawienie czcionki
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, "Arial");
    SendMessage(hwndDebug, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Wy�wietlenie okna
    ShowWindow(hwndDebug, nCmdShow);
    UpdateWindow(hwndDebug);
}

void UpdateDebugWindow(float dotX, float dotY, float foodX, float foodY, int liczbaSegmentow) {
    if (!hwndDebug) return;

    // Formatowanie tekstu dla tre�ci okna
    char buffer[256];
    sprintf(buffer, "Pozycja kropki: X=%.3f, Y=%.3f | Pozycja jedzenia: X=%.3f, Y=%.3f | Liczba segment�w: %d",
                    dotX, dotY, foodX, foodY, liczbaSegmentow);

    // Aktualizacja tre�ci okna
    SetWindowText(hwndDebug, buffer);
}
