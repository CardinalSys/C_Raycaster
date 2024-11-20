#include <GLFW/glfw3.h>
#include "stdio.h"
#include <math.h>

#define width 1280
#define height 480

#define GRID_SIZE 5
#define CELL_SIZE 0.39f
#define movementDisplacement 0.05f
#define PI 3.14159265359f
#define ROTATION_SPEED (PI/32.0f) 
#define FOV (PI/2.0f)


float playerAngle = 0.0f;

struct Vec2 {
    float x;
    float y;
};

struct Vec2 playerPos;

struct Col {
    struct Vec2;
    float distance;
};

struct Col LeftCol;
struct Col CenterCol;
struct Col RightCol;

int grid[GRID_SIZE][GRID_SIZE] = {
    {1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1}
};

void DrawSquare(float x, float y, float size) {
    glBegin(GL_QUADS);
    glVertex2f(x - size / 2, y - size / 2);
    glVertex2f(x - size / 2, y + size / 2);
    glVertex2f(x + size / 2, y + size / 2);
    glVertex2f(x + size / 2, y - size / 2);
    glEnd();
}

void DrawGrid() {

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 1) {
                glColor3f(0.05f, 0.05f, 0.05f);
            }
            else {
                glColor3f(1, 1, 1);
            }
            float x = -0.8f + i * 0.4f;
            float y = -0.8f + j * 0.4f;
            DrawSquare(x, y, CELL_SIZE);
        }
    }

}

void DrawPlayer() {
    glColor3f(1, 0, 0);
    DrawSquare(playerPos.x, playerPos.y, 0.1f);
}

int CheckCollision(float x, float y) {

    int col = (int)((x + 1.0f) / CELL_SIZE); 
    int row = (int)((y + 1.0f) / CELL_SIZE);


    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE)
        return 1; 


    return grid[row][col] != 0 ? 1 : 0;
}

void DrawRay(struct Vec2 startPos, float angle, float length) {
    float xF = startPos.x + cosf(angle) * length;
    float yF = startPos.y + sinf(angle) * length;
    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    glVertex2f(startPos.x, startPos.y);
    glVertex2f(xF, yF);
    glEnd();
}

void DDA(struct Vec2 startPos, float angle) {
    printf("\n--- DDA Debug ---\n");
    printf("Start position: (%f, %f)\n", startPos.x, startPos.y);
    printf("Angle: %f radians\n", angle);

    float d = 2.0f;
    float step;
    float dx = d * cos(angle);
    float dy = d * sin(angle);

    printf("dx: %f, dy: %f\n", dx, dy);


    if (abs(dx) >= abs(dy)) {
        step = abs(dx) * 100;  // Increase steps for better resolution
    }
    else {
        step = abs(dy) * 100;  // Increase steps for better resolution
    }

    printf("Number of steps: %f\n", step);

    dx = dx / step;
    dy = dy / step;

    printf("Step sizes - dx: %f, dy: %f\n", dx, dy);

    float x = startPos.x;
    float y = startPos.y;
    int i = 0;


    glColor3f(0, 0, 1.0f); 
    glPointSize(2.0f);


    glBegin(GL_POINTS);
    while (i <= step && !CheckCollision(x, y)) {
        glVertex2f(x, y);
        x += dx;
        y += dy;
        i++;

        if (i % 100 == 0) {
            printf("Point %d: (%f, %f)\n", i, x, y);
        }
    }
    glEnd();
    glPointSize(1.0f);

    printf("Final position: (%f, %f)\n", x, y);
    printf("Points drawn: %d\n", i);
    printf("--- End DDA Debug ---\n");
}

float normalizeAngle(float angle) {
    angle = fmodf(angle, 2.0f * PI);
    if (angle < 0) {
        angle += 2.0f * PI;
    }
    return angle;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W) {
            float newX = playerPos.x + cosf(playerAngle) * movementDisplacement;
            float newY = playerPos.y + sinf(playerAngle) * movementDisplacement;
            if (!CheckCollision(newX, newY)) {
                playerPos.x = newX;
                playerPos.y = newY;
            }
        }
        else if (key == GLFW_KEY_S) {
            float newX = playerPos.x - cosf(playerAngle) * movementDisplacement;
            float newY = playerPos.y - sinf(playerAngle) * movementDisplacement;
            if (!CheckCollision(newX, newY)) {
                playerPos.x = newX;
                playerPos.y = newY;
            }
        }
        else if (key == GLFW_KEY_A) {
            playerAngle = normalizeAngle(playerAngle + ROTATION_SPEED);
        }
        else if (key == GLFW_KEY_D) {
            playerAngle = normalizeAngle(playerAngle - ROTATION_SPEED);
        }


    }
}

void DrawGameView() {

    glViewport(0, 0, width / 2, height);

    glScissor(0, 0, width / 2, height);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    DrawGrid();
    DrawPlayer();

    DDA(playerPos, playerAngle);

    glDisable(GL_SCISSOR_TEST);
}

void Draw3DSquare(struct Col col, float angle, float xOffset) {  // Added xOffset parameter
    if (col.distance > 0) {
        float scale = 0.1f;

        // Calculate the projected height accounting for fisheye correction
        float correctedDistance = col.distance * cosf(angle);  // Correct for fisheye
        float projectedHeight = (scale / correctedDistance);

        if (projectedHeight > 1.0f) projectedHeight = 1.0f;

        // Use xOffset instead of calculating displacement based on angle
        float brightness = 1.0f / (1.0f + correctedDistance * 0.5f);
        glColor3f(brightness, brightness, brightness);

        glBegin(GL_QUADS);
        glVertex2f(-0.1f + xOffset, projectedHeight);
        glVertex2f(-0.1f + xOffset, -projectedHeight);
        glVertex2f(0.1f + xOffset, -projectedHeight);
        glVertex2f(0.1f + xOffset, projectedHeight);
        glEnd();
    }
}

void Draw3DView() {
    glViewport(width / 2, 0, width / 2, height);
    glScissor(width / 2, 0, width / 2, height);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    // Calculate relative angles for each ray
    float leftAngle = -FOV / 2;
    float centerAngle = 0.0f;
    float rightAngle = FOV / 2;

    // Space the squares evenly across the view
    Draw3DSquare(LeftCol, leftAngle, -0.6f);    // Left portion
    Draw3DSquare(CenterCol, centerAngle, 0.0f); // Center portion
    Draw3DSquare(RightCol, rightAngle, 0.6f);   // Right portion

    glDisable(GL_SCISSOR_TEST);
}
int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Raycaster", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, keyCallback);
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        DrawGameView();
        Draw3DView();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}