/*
 * Program tworzący gierkę "Snake II" z Nokii 5110
 * w OpenGL, dla systemów Windows x86 i x64 :)
 */

/** SNEK Projekt - on MIT License
    CODE WRITTEN BY 'APPLEHAT' (ApplehatDoesStuff)
 komentarze w Polskim, bo ja - autor polakiem :) i zamieniać nie mam zamiaru.
**/

            /****  LISTA TODO: ****/
//(* - niegotowe; V - zaimplementowane; T - w trakcie testowania;)//
/** [V] dodaj sekretną opcję użycia config-(nazwa).ini **/
/** [v] dodaj opcje w pliku config.ini na zmiane rozdzielczości **/
/** [V] dodaj punktacje (PointCount) do dialogu przegranej **/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include "debugwin.h"
#include "resource.h"

#define KROKX 0.0260
#define KROKY 0.0450

 /*czemu wartości kroku dla X i Y są osobne?
 kropka idąc na górę stykała się z innymi kropkami.*/

#define PADDING 0.025   // odstęp linii bocznych, jak na Nokii

//domyślne wartości
#define X_MIN -2.5
#define X_MAX 2.5
#define DEFAULT_X 0.5
#define DEFAULT_Y 0.5
#define MAX_SEGMENTS 777     // issue #2
#define TIMER_SET 350
#define limit 0.025


int global_argc;
int PointCount = 0;
int LiczbaSegmentow = 1;
int show_xy;
int win_res;
float SegmentWeza[MAX_SEGMENTS][2];
float DotX = DEFAULT_X, DotY = DEFAULT_Y;
float KierunekX = KROKX; // Początkowy kierunek ruchu w poziomie
float KierunekY = 0.0;   // Początkowy kierunek ruchu w pionie
float FoodX;    //bez jedzenia nigdy się nie odbędzie.
float FoodY;
float Skalacja = 7.0;
bool debugMode = false;
bool snakeMoved = false;
char **global_argv;
char PointCountChar[100];
char About[] = "SNEK - Win32 Release version of SNEK\nv1.0.0.0-insiders\n\nCreated by Applehat (ApplehatDoesStuff)\nSNEK_GL Project is distributed under MIT License.\n\nCopyright (C) 2024 ApplehatDoesStuff\n";



void CzytajConfig(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Nie mozna otworzyc pliku konfiguracyjnego.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "SHOW_XY=", 8) == 0) {
            show_xy = atoi(line + 8);
        // dodaj zmiane rozdzielczości [TODO #2]
        } else if (strncmp(line, "WIN_RES=", 8) == 0) {
            win_res = atoi(line + 8);
        }
    }

    fclose(file);
}

/** JEDZENIE **/
void GenerateFood() {
    // Zakres dla FoodX i FoodY
    float minX = 0.050;
    float maxX = 0.950;
    float minY = 0.050;
    float maxY = 0.950;

    // Obliczenie zakresów do generowania losowej współrzędnej
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;

    // Generowanie losowych współrzędnych w odpowiednim zakresie
    FoodX = minX + ((float)rand() / RAND_MAX) * rangeX;
    FoodY = minY + ((float)rand() / RAND_MAX) * rangeY;
}



void RysujBoczneLinie(){
    /*
    rysuje boczne linie, tak jak to jest
    w oryginalnej wersji SNAKE II
     */
    glColor3f(0.0f, 0.0f, 0.0f);

    // Pobieranie szerokości i wysokości okna
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    // Przeskalowanie PADDING do przestrzeni okna
    float paddingX = PADDING * width;
    float paddingY = PADDING * height;

    // Rysowanie linii
    glBegin(GL_LINES);
        // Lewa linia
        glVertex2f(paddingX, paddingY);
        glVertex2f(paddingX, height - paddingY);

        // Prawa linia
        glVertex2f(width - paddingX, paddingY);
        glVertex2f(width - paddingX, height - paddingY);

        // Dolna linia
        glVertex2f(paddingX, paddingY);
        glVertex2f(width - paddingX, paddingY);

        // Górna linia
        glVertex2f(paddingX, height - paddingY);
        glVertex2f(width - paddingX, height - paddingY);
    glEnd();
}

/**RYSUJE FRAGMENT (JEDEN KWADRACIK) WEZA O WSPÓŁRZĘDNYCH X I Y**/
void RysujFragmentWeza(float x, float y){

    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    // skalowanie piksela do okna
    float pixelX = x * width;
    float pixelY = y * height;

    glPointSize(Skalacja);  // Ustawienie rozmiaru punktu
    glBegin(GL_POINTS);
        glVertex2f(pixelX, pixelY);   // Rysowanie punktu na terenie ekranu
    glEnd();
}

void RysujJedzenie(){
    glColor3f(0.5f, 0.5f, 0.5f);
    glPointSize(Skalacja);
    glBegin(GL_POINTS);
        glVertex2f(FoodX * glutGet(GLUT_WINDOW_WIDTH), FoodY * glutGet(GLUT_WINDOW_HEIGHT));
    glEnd();
}

/** LOGIKA KOLIZJI WEZA**/
bool KolizjaWeza(float x, float y){
    for (int i = 0; i < LiczbaSegmentow; i++) {
        /*if (x == SegmentWeza[i][0] && y == SegmentWeza[i][1]) {
            return true;
        }*/ if (DotX < 0.050 - KROKX || DotY < 0.050 - KROKY){
            return true;
        } else if (DotX > 0.950 + KROKX || DotY > 0.950 + KROKY){
            return true;
        }
    }
    return false;
}

/** kontrolowanie weza przez klawiature **/
void Kontrol(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            KierunekX = 0.0;
            KierunekY = KROKY;
            break;

        case GLUT_KEY_DOWN:
            KierunekX = 0.0;
            KierunekY = -KROKY;
            break;

        case GLUT_KEY_LEFT:
            KierunekX = -KROKX;
            KierunekY = 0.0;
            break;

        case GLUT_KEY_RIGHT:
            KierunekX = KROKX;
            KierunekY = 0.0;
            break;
        case GLUT_KEY_F1:
            MessageBoxA(NULL, About ,"About SNEK", MB_OK);
            break;
		case GLUT_KEY_F3:
			MessageBoxA(NULL, "Game has been paused. Press OK or close this messagebox to continue", "Pause!", MB_OK);
			break;

    }
}

void DodajSegmentWeza(float x, float y) {
    // Przesuń każdy segment węża na miejsce poprzedniego segmentu
    for (int i = LiczbaSegmentow - 1; i > 0; --i) {
        SegmentWeza[i][0] = SegmentWeza[i-1][0];
        SegmentWeza[i][1] = SegmentWeza[i-1][1];
    }

    // Ustaw nową głowę węża na podane współrzędne
    SegmentWeza[0][0] = x;
    SegmentWeza[0][1] = y;

    // Zwiększ liczbę segmentów węża, jeśli nie przekracza MAX_SEGMENTS
    if (LiczbaSegmentow < MAX_SEGMENTS) {
        LiczbaSegmentow++;
    }
}


/** FUNKCJA TIMEROWA **/


void Timer(int value) {
    float oldDotX = DotX;
    float oldDotY = DotY;

    //Dodaj ilość PointCount [TODO - #3]
    char buffer[100];    //bufor tymczasowy dla napisu
    sprintf(buffer, "Detected Wall Collision! - You Lost!\nPoints scored: %d", PointCount);    //do buforu: komunikat i liczba
    strcpy(PointCountChar, buffer);    //kopiuje to co z buforu na PointCountChar
    
    // Zapisz poprzednią pozycję głowy węża
    float prevX = DotX;
    float prevY = DotY;

    // Przesuń głowę węża zgodnie z aktualnym kierunkiem
    DotX += KierunekX;
    DotY += KierunekY;

    // Sprawdź kolizję z wężem
    if (KolizjaWeza(DotX, DotY)) {
        MessageBoxA(NULL, PointCountChar, "Game Over", MB_OK);
        exit(0);
    }

    // Sprawdź kolizję z jedzeniem i dodaj nowy segment
    if (fabs(DotX - FoodX) <= limit && fabs(DotY - FoodY) <= limit) {
        DodajSegmentWeza(prevX, prevY);  // Dodaj nowy segment na poprzedniej pozycji głowy
        PointCount += 100;    // jedno zdobycie jedzenia to 100 punktów.
        GenerateFood();
    }

    // Przesuń każdy segment węża na miejsce poprzedniego
    for (int i = LiczbaSegmentow - 1; i > 0; --i) {
        SegmentWeza[i][0] = SegmentWeza[i-1][0];
        SegmentWeza[i][1] = SegmentWeza[i-1][1];
    }

    // Ustaw nową pozycję głowy węża na początku tablicy segmentów
    SegmentWeza[0][0] = DotX;
    SegmentWeza[0][1] = DotY;

    glutPostRedisplay();  // Odśwież ekran
    glutTimerFunc(TIMER_SET, Timer, 0);  // Ustaw ponownie timer
}



/*** STREFA FUNKCJI GRAFICZNYCH - UWAZAJ ***/

void UstawRozdzielczosc(int resolution, int* width, int* height) {
    switch (resolution) {
        case 3:
            Skalacja = 11.5;
            *width = 504;
            *height = 288;
            break;
        case 2:
            Skalacja = 7.0;
            *width = 420;
            *height = 240;
        case 1:
        default:
            Skalacja = 7.0;
            *width = 336;
            *height = 192;
            break;
    }
}

void display() {

    glClearColor(153.0f/255.0f, 194.0f/255.0f, 2.0f/255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Tutaj są inne funkcje rysowania

    RysujBoczneLinie();
    RysujFragmentWeza(DotX, DotY);
    //RysujJedzenie();

    for (int i = 1; i < LiczbaSegmentow; ++i) {
        RysujFragmentWeza(SegmentWeza[i][0], SegmentWeza[i][1]);
    }

    RysujJedzenie();

    if (show_xy == 1) {
        // Wyświetlenie współrzędnych kropki
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(DotX + 0.1, DotY); // Przesunięcie od kropki
        char text[50];
        sprintf(text, "X: %.3f, Y: %.3f", DotX, DotY);
        glutBitmapString(GLUT_BITMAP_HELVETICA_12, (unsigned char*)text);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(X_MIN, X_MAX, -2.0, 2.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glutReshapeWindow(336, 192);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    global_argc = __argc;
    global_argv = __argv;
    
    srand(time(NULL));  // inicjalizacja generatora liczb losowych
    glutInit(&__argc, __argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    CzytajConfig("config.ini");

    int window_width, window_height;
    UstawRozdzielczosc(win_res, &window_width, &window_height);    //na podstawie wartości win_res, zmień rozdzielczość
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow("SNEK - Snake II dla systemow Windows");
    //nazwa to nie ma jakiegoś znaczenia

    //ustaw ikone z pliku resource.h (DEF_ICON)
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(DEF_ICON));
    SendMessage(GetActiveWindow(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(GetActiveWindow(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

     for (int i = 1; i < global_argc; ++i) {
        if (strcmp(global_argv[i], "--debugwin") == 0) {
            debugMode = true;
            break;
        }
    }

    if (debugMode) {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        CreateDebugWindow(hInstance, SW_SHOW);
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(Kontrol);

    /*HWND hwnd = FindWindow(NULL, "SNEK - Snake II dla systemow Windows");
    if (hwnd != NULL) {
        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_SIZEBOX);
    }*/

    GenerateFood();
    glutTimerFunc(TIMER_SET, Timer, 0);
    glutMainLoop();
    return 0;
}
