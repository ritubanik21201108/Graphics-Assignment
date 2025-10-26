#include <iostream>
#include <cmath>
#include <cstdio>
#include <GL/freeglut.h>

// Define the window half-size for the centered coordinates
#define WINDOW_HALF_SIZE 250.0f
#define WINDOW_SIZE 500.0f

// Global variables to be set by terminal input
int P1_x, P1_y;
int P2_x, P2_y;
int W = 1; // Line width

// Global variable to track the current mode (1 for Standard, 2 for Thick)
int current_mode = 0;

// Function to set a pixel color
void setPixel(int x, int y) {
    glBegin(GL_POINTS);
    // x and y are passed directly, as the centered projection handles the translation.
    glVertex2i(x, y);
    glEnd();
}

// a. Standard Bresenham's Line Drawing Algorithm (Unchanged internally)
void bresenhamStandard(int x1, int y1, int x2, int y2) {
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;

    int p;
    int x = x1;
    int y = y1;

    // Handle x-major axis (|m| <= 1)
    if (dx >= dy) {
        p = 2 * dy - dx;

        for (int i = 0; i < dx; i++) {
            setPixel(x, y);
            if (p < 0) {
                p += 2 * dy;
            } else {
                p += 2 * (dy - dx);
                y += sy;
            }
            x += sx;
        }
    }
    // Handle y-major axis (|m| > 1)
    else {
        p = 2 * dx - dy;

        for (int i = 0; i < dy; i++) {
            setPixel(x, y);
            if (p < 0) {
                p += 2 * dx;
            } else {
                p += 2 * (dx - dy);
                x += sx;
            }
            y += sy;
        }
    }
    setPixel(x, y);
}


// b. Bresenham's Thick Line (Rectangle/Width-based Extension)

void bresenhamThick(int x1, int y1, int x2, int y2, int width) {
    double halfWidth = width / 2.0;
    double dx = (double)(x2 - x1);
    double dy = (double)(y2 - y1);
    double length = std::sqrt(dx * dx + dy * dy);

    if (length < 1.0) {
        glRecti(x1 - width/2, y1 - width/2, x1 + width/2, y1 + width/2);
        return;
    }

    // Perpendicular unit vector (nx, ny)
    double nx = -dy / length;
    double ny = dx / length;

    // Calculate the four corners of the bounding rectangle
    double C1_x = x1 - halfWidth * nx;
    double C1_y = y1 - halfWidth * ny;

    double C2_x = x1 + halfWidth * nx;
    double C2_y = y1 + halfWidth * ny;

    double C3_x = x2 + halfWidth * nx;
    double C3_y = y2 + halfWidth * ny;

    double C4_x = x2 - halfWidth * nx;
    double C4_y = y2 - halfWidth * ny;

    // Draw the filled rectangle/polygon
    glBegin(GL_POLYGON);
        glVertex2d(C1_x, C1_y);
        glVertex2d(C2_x, C2_y);
        glVertex2d(C3_x, C3_y);
        glVertex2d(C4_x, C4_y);
    glEnd();
}

// ---

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the Axes (0,0 is now the center)
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glBegin(GL_LINES);
        // X-axis (horizontal)
        glVertex2i(-WINDOW_HALF_SIZE, 0);
        glVertex2i(WINDOW_HALF_SIZE, 0);
        // Y-axis (vertical)
        glVertex2i(0, -WINDOW_HALF_SIZE);
        glVertex2i(0, WINDOW_HALF_SIZE);
    glEnd();

    // Draw the Line and Text
    glColor3f(1.0, 1.0, 1.0); // White
    char coord_buffer[128];

    if (current_mode == 1) {
        sprintf(coord_buffer, "Mode A: Standard Bresenham Line from (%d,%d) to (%d,%d)", P1_x, P1_y, P2_x, P2_y);
        glColor3f(0.0, 1.0, 0.0); // Green
        glPointSize(1.0);
        bresenhamStandard(P1_x, P1_y, P2_x, P2_y);
    } else if (current_mode == 2) {
        sprintf(coord_buffer, "Mode B: Thick Line (W=%d) from (%d,%d) to (%d,%d)", W, P1_x, P1_y, P2_x, P2_y);
        glColor3f(1.0, 1.0, 0.0); // Yellow
        bresenhamThick(P1_x, P1_y, P2_x, P2_y, W);
    }


    glRasterPos2i(-WINDOW_HALF_SIZE + 10, WINDOW_HALF_SIZE - 20);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)coord_buffer);



    glColor3f(1.0, 0.0, 0.0); // Red
    glPointSize(5.0);
    glBegin(GL_POINTS);
        glVertex2i(P1_x, P1_y);
        glVertex2i(P2_x, P2_y);
    glEnd();

    glFlush();
}

// PROJECTIVE SETUP FUNCTION
void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // SETS THE COORDINATE SYSTEM to (-250, -250) to (250, 250)
    gluOrtho2D(-WINDOW_HALF_SIZE, WINDOW_HALF_SIZE, -WINDOW_HALF_SIZE, WINDOW_HALF_SIZE);
}

void terminal_input() {
    int choice = 0;

    std::cout << "=====================================================" << std::endl;
    std::cout << "Bresenham's Line Drawing Algorithm Implementations" << std::endl;
    std::cout << "Coordinate System: Center (0, 0) | Range: -250 to +250" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;
    std::cout << "Which line drawing algorithm would you like to run?" << std::endl;
    std::cout << "1. Standard Bresenham Line (a)" << std::endl;
    std::cout << "2. Thick Line Extension (b)" << std::endl;
    std::cout << "Enter choice (1 or 2): ";

    // Get mode choice
    while (!(std::cin >> choice) || (choice != 1 && choice != 2)) {
        std::cout << "Invalid choice. Please enter 1 or 2: ";
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
    current_mode = choice;

    // Get line coordinates (Can now be negative, e.g., -100 50)
    std::cout << "\nEnter coordinates for Point P1 (x1 y1, e.g., -50 100): ";
    while (!(std::cin >> P1_x >> P1_y)) {
        std::cout << "Invalid input. Please enter two integers: ";
        std::cin.clear();
        std::cin.ignore(12000, '\n');
    }

    std::cout << "Enter coordinates for Point P2 (x2 y2, e.g., 100 -250): ";
    while (!(std::cin >> P2_x >> P2_y)) {
        std::cout << "Invalid input. Please enter two integers: ";
        std::cin.clear();
        std::cin.ignore(12000, '\n');
    }

    // Get width for option (b)
    if (current_mode == 2) {
        std::cout << "Enter line width W (integer, e.g., 20): ";
        while (!(std::cin >> W) || W <= 0) {
            std::cout << "Invalid input. Please enter a positive integer for width: ";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }

    std::cout << "\nMode " << current_mode << " selected. Opening graphics window..." << std::endl;
    std::cout << "=====================================================" << std::endl;
}

int main(int argc, char** argv) {

    terminal_input();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE); // 500x500 window
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Bresenham's Line Drawing (Centered)");

    init(); // Setup the centered projection
    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}
