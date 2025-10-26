#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 800;
const int UI_HEADER_HEIGHT = 70;
const int AXIS_GAP = 50;

const float DRAWING_AREA_WIDTH = WINDOW_WIDTH * 0.7;
const float DRAWING_AREA_HEIGHT = WINDOW_HEIGHT - UI_HEADER_HEIGHT;
const float SCREEN_CENTER_X = DRAWING_AREA_WIDTH / 2.0f;
const float SCREEN_CENTER_Y = DRAWING_AREA_HEIGHT / 2.0f;

const float OPENGL_MIN_X = -SCREEN_CENTER_X;
const float OPENGL_MAX_X = SCREEN_CENTER_X;
const float OPENGL_MIN_Y = -SCREEN_CENTER_Y;
const float OPENGL_MAX_Y = SCREEN_CENTER_Y;

struct Point {
    float x, y;
};

struct LineSegment {
    Point p1, p2;
};

std::vector<Point> visible_points;
std::vector<LineSegment> lines_to_clip;

float xmin, ymin, xmax, ymax;

// ------------------- Helper Functions -------------------
void draw_text(float x, float y, float r, float g, float b, const std::string &text, void* font) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(font, c);
}

// ------------------- Header -------------------
void draw_ui_header(const std::string& title, const std::string& instruction,
                    float r, float g, float b) {

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Header background (light teal)
    glColor3f(0.7f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(0, WINDOW_HEIGHT - UI_HEADER_HEIGHT);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT - UI_HEADER_HEIGHT);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();

    // Border line
    glColor3f(0.2f, 0.5f, 0.5f);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(0, WINDOW_HEIGHT - UI_HEADER_HEIGHT);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT - UI_HEADER_HEIGHT);
    glEnd();

    draw_text(10, WINDOW_HEIGHT - 25, r, g, b, title, GLUT_BITMAP_HELVETICA_18);
    draw_text(10, WINDOW_HEIGHT - 50, 0.2f, 0.2f, 0.2f, instruction, GLUT_BITMAP_HELVETICA_12);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ------------------- Axes -------------------
void draw_coordinate_system() {
    const float AXIS_R = 0.0f, AXIS_G = 0.7f, AXIS_B = 0.0f; // Green axes
    const float TICK_SIZE = 5.0f;

    glColor3f(AXIS_R, AXIS_G, AXIS_B);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(OPENGL_MIN_X, 0);
    glVertex2f(OPENGL_MAX_X, 0);
    glVertex2f(0, OPENGL_MIN_Y);
    glVertex2f(0, OPENGL_MAX_Y);
    glEnd();

    // Ticks
    for (int val = AXIS_GAP; val < OPENGL_MAX_X; val += AXIS_GAP) {
        // X-axis ticks
        glBegin(GL_LINES);
        glVertex2f(val, -TICK_SIZE);
        glVertex2f(val, TICK_SIZE);
        glVertex2f(-val, -TICK_SIZE);
        glVertex2f(-val, TICK_SIZE);
        glEnd();

        draw_text(val - 5, -20, AXIS_R, AXIS_G, AXIS_B, std::to_string(val), GLUT_BITMAP_HELVETICA_10);
        draw_text(-val - 10, -20, AXIS_R, AXIS_G, AXIS_B, std::to_string(-val), GLUT_BITMAP_HELVETICA_10);
    }

    for (int val = AXIS_GAP; val < OPENGL_MAX_Y; val += AXIS_GAP) {
        // Y-axis ticks
        glBegin(GL_LINES);
        glVertex2f(-TICK_SIZE, val);
        glVertex2f(TICK_SIZE, val);
        glVertex2f(-TICK_SIZE, -val);
        glVertex2f(TICK_SIZE, -val);
        glEnd();

        draw_text(10, val - 4, AXIS_R, AXIS_G, AXIS_B, std::to_string(val), GLUT_BITMAP_HELVETICA_10);
        draw_text(10, -val - 4, AXIS_R, AXIS_G, AXIS_B, std::to_string(-val), GLUT_BITMAP_HELVETICA_10);
    }

    draw_text(10, -20, AXIS_R, AXIS_G, AXIS_B, "0", GLUT_BITMAP_HELVETICA_10);
}

// ------------------- Clipping Window -------------------
void draw_clipping_window() {
    glColor3f(0.0f, 0.0f, 0.8f); // Blue frame
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(xmin, ymin);
    glVertex2f(xmax, ymin);
    glVertex2f(xmax, ymax);
    glVertex2f(xmin, ymax);
    glEnd();
}

// ------------------- Liang–Barsky -------------------
bool liang_barsky(float x0, float y0, float x1, float y1, float &t0, float &t1) {
    float dx = x1 - x0, dy = y1 - y0;
    float p[4] = {-dx, dx, -dy, dy};
    float q[4] = {x0 - xmin, xmax - x0, y0 - ymin, ymax - y0};
    t0 = 0.0f, t1 = 1.0f;

    for (int i = 0; i < 4; ++i) {
        if (fabs(p[i]) < 1e-6) {
            if (q[i] < 0) return false;
        } else {
            float t = q[i] / p[i];
            if (p[i] < 0) t0 = std::max(t0, t);
            else t1 = std::min(t1, t);
        }
    }
    return t0 <= t1;
}

// ------------------- Display -------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    visible_points.clear();

    draw_ui_header(
        "Liang–Barsky Line Clipping (Modified Version)",
        "Clipping Window: (" + std::to_string(xmin) + "," + std::to_string(ymin) +
        ") → (" + std::to_string(xmax) + "," + std::to_string(ymax) + ")",
        0.0f, 0.4f, 0.7f
    );

    draw_coordinate_system();
    draw_clipping_window();

    for (auto &line : lines_to_clip) {
        float x0 = line.p1.x, y0 = line.p1.y;
        float x1 = line.p2.x, y1 = line.p2.y;

        float t0, t1;

        // Original line (soft red)
        glColor3f(0.9f, 0.3f, 0.3f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(x0, y0);
        glVertex2f(x1, y1);
        glEnd();

        if (liang_barsky(x0, y0, x1, y1, t0, t1)) {
            float cx0 = x0 + t0 * (x1 - x0);
            float cy0 = y0 + t0 * (y1 - y0);
            float cx1 = x0 + t1 * (x1 - x0);
            float cy1 = y0 + t1 * (y1 - y0);

            visible_points.push_back({cx0, cy0});
            visible_points.push_back({cx1, cy1});

            // Clipped segment (orange)
            glColor3f(1.0f, 0.6f, 0.0f);
            glLineWidth(4.0f);
            glBegin(GL_LINES);
            glVertex2f(cx0, cy0);
            glVertex2f(cx1, cy1);
            glEnd();

            // Intersection dots (purple)
            glColor3f(0.5f, 0.0f, 0.8f);
            glPointSize(8.0f);
            glBegin(GL_POINTS);
            glVertex2f(cx0, cy0);
            glVertex2f(cx1, cy1);
            glEnd();
        }
    }

    // Label visible points
    for (size_t i = 0; i < visible_points.size(); ++i) {
        std::string lbl = "P" + std::to_string(i + 1);
        draw_text(visible_points[i].x + 8, visible_points[i].y + 8, 0.4f, 0.0f, 0.6f, lbl, GLUT_BITMAP_HELVETICA_12);
    }

    // Right panel
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float startX = DRAWING_AREA_WIDTH;
    glColor3f(0.92f, 0.96f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(startX, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, DRAWING_AREA_HEIGHT);
    glVertex2f(startX, DRAWING_AREA_HEIGHT);
    glEnd();

    glColor3f(0.4f, 0.6f, 0.6f);
    glBegin(GL_LINES);
    glVertex2f(startX, 0);
    glVertex2f(startX, DRAWING_AREA_HEIGHT);
    glEnd();

    draw_text(startX + 15, DRAWING_AREA_HEIGHT - 20, 0.1f, 0.3f, 0.3f, "Visible Points:", GLUT_BITMAP_HELVETICA_12);

    float y = DRAWING_AREA_HEIGHT - 40;
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    for (size_t i = 0; i < visible_points.size(); ++i) {
        ss.str("");
        ss << "P" << (i + 1) << " (" << visible_points[i].x << ", " << visible_points[i].y << ")";
        draw_text(startX + 15, y, 0.0f, 0.0f, 0.0f, ss.str(), GLUT_BITMAP_HELVETICA_10);
        y -= 15;
        if (y < 10) break;
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glFlush();
}

// ------------------- Input -------------------
void take_input() {
    int n;
    std::cout << "Enter Clipping Window (xmin ymin xmax ymax): ";
    std::cin >> xmin >> ymin >> xmax >> ymax;
    if (xmin > xmax) std::swap(xmin, xmax);
    if (ymin > ymax) std::swap(ymin, ymax);

    std::cout << "Number of lines: ";
    std::cin >> n;

    for (int i = 0; i < n; ++i) {
        float x1, y1, x2, y2;
        std::cout << "Line " << (i + 1) << " P1(x,y) P2(x,y): ";
        std::cin >> x1 >> y1 >> x2 >> y2;
        lines_to_clip.push_back({{x1, y1}, {x2, y2}});
    }
}

// ------------------- Init -------------------
void init() {
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(OPENGL_MIN_X, OPENGL_MAX_X, OPENGL_MIN_Y, OPENGL_MAX_Y);
    glMatrixMode(GL_MODELVIEW);
}

// ------------------- Main -------------------
int main(int argc, char** argv) {
    take_input();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Liang–Barsky Line Clipping (Modified)");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
