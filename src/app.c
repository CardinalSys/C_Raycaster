#include <GLFW/glfw3.h>
#include "stdio.h"
#include <math.h>

#define width 1280
#define height 480

#define GRID_SIZE 10
#define CELL_SIZE 0.19f
#define movementDisplacement 0.05f
#define PI 3.14159265359f
#define ROTATION_SPEED (PI/32.0f) 
#define FOV (PI/2.0f)


float playerAngle = 0.0f;

float testDistance;

struct Vec2 {
    float x;
    float y;
};

struct Vec2 playerPos;

struct Col {
    struct Vec2;
    float distance;
};


int grid[GRID_SIZE][GRID_SIZE] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

double time = 0;
double oldTime = 0;

void Draw3DSquare(float distance, float angle, float xOffset);

void DrawSquare(float x, float y, float size) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + size);
    glVertex2f(x + size, y + size);
    glVertex2f(x + size, y);
    glEnd();
}

void DrawGrid() {

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 1) {
                glColor3f(0.05f, 0.05f, 0.05f); // Wall color
            }
            else {
                glColor3f(1, 1, 1); // Floor color
            }


            float x = -1.0f + i * CELL_SIZE;
            float y = -1.0f + j * CELL_SIZE;
            DrawSquare(x, y, CELL_SIZE);
        }
    }
}

void DrawPlayer() {
    glColor3f(1, 0, 0); // Player color
    float playerSize = CELL_SIZE / 4; // Player size is smaller than a cell

    // Center the player square on the player's position
    float x = playerPos.x - playerSize / 2;
    float y = playerPos.y - playerSize / 2;

    DrawSquare(x, y, playerSize);
}


int CheckCollision(float x, float y) {
    int col = (int)((x + 1.0f) / CELL_SIZE);
    int row = (int)((y + 1.0f) / CELL_SIZE);

    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE)
        return 1;

    return grid[col][row] != 0 ? 1 : 0;  // Fixed by swapping row and col
}



float DDA(struct Vec2 startPos, float angle, int debug) {
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
        if(debug == 1)
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
    float distance = sqrt(pow(startPos.x - x, 2) + pow(startPos.y - y, 2));
    printf("Distance: %f", distance);
    printf("Final position: (%f, %f)\n", x, y);
    printf("Points drawn: %d\n", i);
    printf("--- End DDA Debug ---\n");
    return distance;

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

    testDistance = DDA(playerPos, playerAngle, 1);

    glDisable(GL_SCISSOR_TEST);
}

void Draw3DSquare(float distance, float angle, float xOffset) {
    if (distance > 0) {
        float scale = 0.1f;


        float correctedDistance = distance * cosf(angle);
        float projectedHeight = (scale / correctedDistance);

        if (projectedHeight > 1.0f) projectedHeight = 1.0f;


        float stripWidth = 2.0f / 320.0f; 


        float brightness = 1.0f / (1.0f + correctedDistance * 0.5f);
        glColor3f(brightness, brightness, brightness);

        glBegin(GL_QUADS);
        glVertex2f(xOffset, projectedHeight);
        glVertex2f(xOffset, -projectedHeight);
        glVertex2f(xOffset + stripWidth, -projectedHeight);
        glVertex2f(xOffset + stripWidth, projectedHeight);
        glEnd();
    }
}
void Draw3DView() {
    glViewport(width / 2, 0, width / 2, height);
    glScissor(width / 2, 0, width / 2, height);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    int numRays = 320; // Number of vertical strips to render
    float rayAngle;
    float angleStep = FOV / numRays;


    float startAngle = playerAngle - FOV / 2;

    for (int i = 0; i < numRays; i++) {

        rayAngle = startAngle + (i * angleStep);
        rayAngle = normalizeAngle(rayAngle);


        float distance = DDA(playerPos, rayAngle, 0);

        float xOffset = -1.0f + (2.0f * i) / numRays;

        Draw3DSquare(distance, rayAngle - playerAngle, xOffset);
    }

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