#include <iostream>
#include <cmath>
#include <vector>
#include <GL/freeglut.h>

// Window dimensions
const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 500;

// Center of the circles
const int CENTER_X = 0;
const int CENTER_Y = 0;

// Circle parameters
const int NUM_CIRCLES = 15; // Number of concentric circles
const int MIN_RADIUS = 10;
const int RADIUS_INCREMENT = 12;
const int THICKNESS_INCREMENT = 1.5;

// Function to set a pixel color
void setPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// Midpoint Circle Drawing Algorithm (Unchanged)
void drawCircle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int p = 1 - r;

    auto plot_points = [&]() {
        setPixel(xc + x, yc + y);
        setPixel(xc - x, yc + y);
        setPixel(xc + x, yc - y);
        setPixel(xc - x, yc - y);
        setPixel(xc + y, yc + x);
        setPixel(xc - y, yc + x);
        setPixel(xc + y, yc - x);
        setPixel(xc - y, yc - x);
    };

    plot_points();

    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        plot_points();
    }
}


void getSmoothColor(int index, int max_index, float& r, float& g, float& b) {
    // Calculate hue based on index (0.0 for first circle, approx 1.0 for last)
    // We use max_index - 1 to ensure the last circle doesn't wrap back to red (0.0)
    float hue = (float)index / (max_index);

    // HSV components (Saturation and Value are maxed for vibrant colors)
    float h = hue * 270.0f; // Hue in degrees [0, 270]
    float s = 1.0f;         // Saturation
    float v = 1.0f;         // Value (Brightness)

    // Standard HSV to RGB conversion logic
    int i = (int)(h / 60.0f) % 6;
    float f = h / 60.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i) {
        case 0: r = v; g = t; b = p; break; // Red -> Yellow
        case 1: r = q; g = v; b = p; break; // Yellow -> Green
        case 2: r = p; g = v; b = t; break; // Green -> Cyan
        case 3: r = p; g = q; b = v; break; // Cyan -> Blue
        case 4: r = t; g = p; b = v; break; // Blue -> Magenta
        case 5: r = v; g = p; b = q; break; // Magenta -> Red
    }
}
// ====================================================================


void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the Axes
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);
        glVertex2i(-WINDOW_WIDTH/2, 0);
        glVertex2i( WINDOW_WIDTH/2, 0);
        glVertex2i(0, -WINDOW_HEIGHT/2);
        glVertex2i(0,  WINDOW_HEIGHT/2);
    glEnd();

    // Loop to draw concentric circles
    for (int i = 0; i < NUM_CIRCLES; ++i) {
        int current_radius = MIN_RADIUS + i * RADIUS_INCREMENT;
        int current_thickness = 1 + i * THICKNESS_INCREMENT;

        // Calculate smooth color gradient across all circles
        float r, g, b;
        getSmoothColor(i, NUM_CIRCLES, r, g, b);
        glColor3f(r, g, b);

        // Draw the thick circle by drawing multiple thin circles
        for (int t = 0; t < current_thickness; ++t) {
            int effective_radius = current_radius - (current_thickness / 2) + t;
            if (effective_radius > 0) {
                drawCircle(CENTER_X, CENTER_Y, effective_radius);
            }
        }
    }

    glFlush();
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Set up a centered 2D coordinate system
    gluOrtho2D(-WINDOW_WIDTH/2, WINDOW_WIDTH/2, -WINDOW_HEIGHT/2, WINDOW_HEIGHT/2);
}

int main(int argc, char** argv) {
    std::cout << "Drawing " << NUM_CIRCLES << " concentric circles with a smooth, continuous rainbow gradient." << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Smooth Rainbow Circles");

    init();
    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}
