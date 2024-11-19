#include <GLFW/glfw3.h>
#include "stdio.h"
#include <math.h>

#define width 640
#define height 480

#define GRID_SIZE 5
#define CELL_SIZE 0.39f
#define movementDisplacement 0.05f
#define PI 3.14159265359f
#define ROTATION_SPEED (PI/32.0f) 

float playerX = 0;
float playerY = 0;

float playerAngle = 0.0f;

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
    DrawSquare(playerX, playerY, 0.1f);
}

int CheckCollision(float x, float y) {

    int col = (int)((x + 1.0f) / CELL_SIZE); 
    int row = (int)((y + 1.0f) / CELL_SIZE);


    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE)
        return 1; 


    return grid[row][col] != 0 ? 1 : 0;
}

void DrawRay(float xO, float yO, float angle, float length) {
    float xF = xO + cosf(angle) * length;
    float yF = yO + sinf(angle) * length;
    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    glVertex2f(xO, yO);
    glVertex2f(xF, yF);
    glEnd();
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
            float newX = playerX + cosf(playerAngle) * movementDisplacement;
            float newY = playerY + sinf(playerAngle) * movementDisplacement;
            if (!CheckCollision(newX, newY)) {
                playerX = newX;
                playerY = newY;
            }
        }
        else if (key == GLFW_KEY_S) {
            float newX = playerX - cosf(playerAngle) * movementDisplacement;
            float newY = playerY - sinf(playerAngle) * movementDisplacement;
            if (!CheckCollision(newX, newY)) {
                playerX = newX;
                playerY = newY;
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
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        DrawGrid();
        DrawPlayer();
        DrawRay(playerX, playerY, playerAngle, 0.5f);
 

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}