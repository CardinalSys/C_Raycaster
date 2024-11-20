#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

#define width 1280
#define height 480

#define GRID_SIZE 10
#define CELL_SIZE 0.19f
#define movementDisplacement 0.05f
#define PI 3.14159265359f
#define ROTATION_SPEED (PI/32.0f)
#define FOV (PI/2.0f)

double previousTime = 0.0;
int frameCount = 0;
double fps = 0.0;

float playerAngle = 0.0f;

struct Vec2 {
    float x;
    float y;
};

struct Vec2 playerPos;

int grid[GRID_SIZE][GRID_SIZE] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

void DrawSquare(float x, float y, float size) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + size);
    glVertex2f(x + size, y + size);
    glVertex2f(x + size, y);
    glEnd();
}

void DrawGrid() {
    float gridOriginX = -1.0f;
    float gridOriginY = -1.0f;

    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            if (grid[row][col] == 1) {
                glColor3f(0.05f, 0.05f, 0.05f); // Wall color
            }
            else {
                glColor3f(1, 1, 1); // Floor color
            }

            float x = gridOriginX + col * CELL_SIZE;
            float y = gridOriginY + row * CELL_SIZE;
            DrawSquare(x, y, CELL_SIZE);
        }
    }
}

void DrawPlayer() {
    glColor3f(1, 0, 0);
    float playerSize = CELL_SIZE / 4;

    float x = playerPos.x - playerSize / 2;
    float y = playerPos.y - playerSize / 2;

    DrawSquare(x, y, playerSize);
}

int CheckCollision(float x, float y) {
    float gridOriginX = -1.0f;
    float gridOriginY = -1.0f;

    int col = (int)((x - gridOriginX) / CELL_SIZE);
    int row = (int)((y - gridOriginY) / CELL_SIZE);

    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE)
        return 1;

    return grid[row][col] != 0 ? 1 : 0;
}

float DDA(struct Vec2 startPos, float angle, int debug) {

    float rayDirX = cosf(angle);
    float rayDirY = sinf(angle);

 
    int mapX = (int)((startPos.x + 1.0f) / CELL_SIZE);
    int mapY = (int)((startPos.y + 1.0f) / CELL_SIZE);


    float sideDistX;
    float sideDistY;


    float deltaDistX = fabsf(CELL_SIZE / rayDirX);
    float deltaDistY = fabsf(CELL_SIZE / rayDirY);
    int stepX;
    int stepY;
    int hit = 0;
    int side; 

    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = ((startPos.x + 1.0f) / CELL_SIZE - mapX) * deltaDistX;
    }
    else {
        stepX = 1;
        sideDistX = (mapX + 1.0f - (startPos.x + 1.0f) / CELL_SIZE) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = ((startPos.y + 1.0f) / CELL_SIZE - mapY) * deltaDistY;
    }
    else {
        stepY = 1;
        sideDistY = (mapY + 1.0f - (startPos.y + 1.0f) / CELL_SIZE) * deltaDistY;
    }

    // Perform DDA
    while (hit == 0) {
        // Jump to next map square in x or y direction
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        }
        else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }
        // Check if ray has hit a wall
        if (mapX < 0 || mapX >= GRID_SIZE || mapY < 0 || mapY >= GRID_SIZE) {
            // Ray is out of bounds
            return -1.0f;
        }
        if (grid[mapY][mapX] > 0) hit = 1;
    }

    // Calculate distance to the point of impact
    float perpWallDist;
    if (side == 0) {
        perpWallDist = (sideDistX - deltaDistX);
    }
    else {
        perpWallDist = (sideDistY - deltaDistY);
    }


    if (debug == 1) {
        glColor3f(0, 0, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(startPos.x, startPos.y);
        float hitX = startPos.x + rayDirX * perpWallDist;
        float hitY = startPos.y + rayDirY * perpWallDist;
        glVertex2f(hitX, hitY);
        glEnd();
    }

    return perpWallDist;
}

float normalizeAngle(float angle) {
    angle = fmodf(angle, 2.0f * PI);
    if (angle < 0) {
        angle += 2.0f * PI;
    }
    return angle;
}

float normalizeAngleDiff(float angle) {
    angle = fmodf(angle + PI, 2.0f * PI);
    if (angle < 0) {
        angle += 2.0f * PI;
    }
    angle -= PI;
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

    DDA(playerPos, playerAngle, 1);

    glDisable(GL_SCISSOR_TEST);
}

void Draw3DSquare(float distance, float angleDiff, float xOffset) {
    if (distance > 0) {
        float scale = 0.1f;

        float correctedDistance = distance * cosf(angleDiff);



        float projectedHeight = (scale / correctedDistance);

        if (projectedHeight > 1.0f) projectedHeight = 1.0f;

        float stripWidth = 2.0f / 320.0f;
        float brightness = 1.0f / (1.0f + correctedDistance * 0.5f);
        if (brightness > 1.0f) brightness = 1.0f;
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
    float angleStep = FOV / numRays;

    float startAngle = playerAngle - FOV / 2;

    for (int i = 0; i < numRays; i++) {
        float rayAngle = startAngle + (i * angleStep);
        rayAngle = normalizeAngle(rayAngle);

        float distance = DDA(playerPos, rayAngle, 0);

        float xOffset = -1.0f + (2.0f * i) / numRays;

        float angleDiff = rayAngle - playerAngle;
        angleDiff = normalizeAngleDiff(angleDiff);

        Draw3DSquare(distance, angleDiff, xOffset);
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

    /* Initialize player position */
    playerPos.x = -1.0f + CELL_SIZE * 1.5f;
    playerPos.y = -1.0f + CELL_SIZE * 1.5f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        frameCount++;

        // Si ha pasado un segundo, calcula FPS
        if (currentTime - previousTime >= 1.0) {
            fps = (double)frameCount / (currentTime - previousTime);
            previousTime = currentTime;
            frameCount = 0;

            // Imprime los FPS en la consola (opcional)
            printf("FPS: %.2f\n", fps);
        }

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
