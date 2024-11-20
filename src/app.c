#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#define WIDTH 1280
#define HEIGHT 480

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

GLuint brickId;

#define MAX_TEXTURES 10

struct TextureManager {
    GLuint textureIds[MAX_TEXTURES];
    int textureCount;
};

struct TextureManager textureManager;

int grid[GRID_SIZE][GRID_SIZE] = {
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 2},
    {2, 0, 0, 0, 0, 0, 0, 1, 0, 2},
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 2},
    {2, 0, 1, 0, 0, 0, 1, 1, 0, 2},
    {2, 1, 1, 0, 0, 0, 0, 0, 0, 2},
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 2},
    {2, 0, 0, 0, 1, 0, 0, 0, 0, 2},
    {2, 0, 0, 0, 1, 0, 0, 0, 0, 2},
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
};

void InitTextureManager() {
    textureManager.textureCount = 0;
}

GLuint LoadTexture(const char* filePath) {
    // Flip images vertically to match OpenGL's coordinate system
    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);
    if (!data) {
        printf("Error loading texture: %s\n", stbi_failure_reason());
        return 0;
    }

    // Verify 64x64 dimensions
    if (width != 64 || height != 64) {
        printf("Warning: Texture %s is not 64x64 (found %dx%d)\n", filePath, width, height);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
    return textureID;
}

int AddTexture(const char* filePath) {
    if (textureManager.textureCount >= MAX_TEXTURES) {
        printf("Error: Maximum texture limit reached\n");
        return -1;
    }

    GLuint textureId = LoadTexture(filePath);
    if (textureId == 0) {
        return -1;
    }

    textureManager.textureIds[textureManager.textureCount] = textureId;
    return textureManager.textureCount++;
}

struct RaycastHit {
    float distance;
    int wallType;
    float texX; // Texture coordinate x (0.0 - 1.0)
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
            if (grid[row][col] == 1 || grid[row][col] == 2) {
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

struct RaycastHit DDA(struct Vec2 startPos, float angle, int debug) {
    struct RaycastHit raycastHit = { -1.0f, 0, 0.0f };

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
            side = 0; // 0 for vertical wall
        }
        else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1; // 1 for horizontal wall
        }

        // Check if ray has hit a wall
        if (mapX < 0 || mapX >= GRID_SIZE || mapY < 0 || mapY >= GRID_SIZE) {
            // Ray is out of bounds
            return raycastHit;
        }

        if (grid[mapY][mapX] > 0) {
            hit = 1;
            raycastHit.wallType = grid[mapY][mapX];
            if (side == 0) {
                raycastHit.distance = (sideDistX - deltaDistX);
                float hitY = startPos.y + rayDirY * raycastHit.distance;
                float relativeY = fmodf(hitY / CELL_SIZE, 1.0f);
                raycastHit.texX = relativeY;
            }
            else {
                raycastHit.distance = (sideDistY - deltaDistY);
                float hitX = startPos.x + rayDirX * raycastHit.distance;
                float relativeX = fmodf(hitX / CELL_SIZE, 1.0f);
                raycastHit.texX = relativeX;
            }
        }
    }

    if (debug == 1) {
        glColor3f(0, 0, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(startPos.x, startPos.y);
        float hitX = startPos.x + rayDirX * raycastHit.distance;
        float hitY = startPos.y + rayDirY * raycastHit.distance;
        glVertex2f(hitX, hitY);
        glEnd();
    }

    return raycastHit;
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
            playerAngle = normalizeAngle(playerAngle - ROTATION_SPEED);
        }
        else if (key == GLFW_KEY_D) {
            playerAngle = normalizeAngle(playerAngle + ROTATION_SPEED);
        }
    }
}

void DrawGameView() {
    glViewport(0, 0, WIDTH / 2, HEIGHT);

    glScissor(0, 0, WIDTH / 2, HEIGHT);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    DrawGrid();
    DrawPlayer();

    DDA(playerPos, playerAngle, 1);

    glDisable(GL_SCISSOR_TEST);
}

void Draw3DSquare(float distance, float angleDiff, float xOffset, int textureID, float texX) {
    if (distance > 0) {

        float scale = 0.1f;

        float correctedDistance = distance * cosf(angleDiff);
        if (correctedDistance < 0.01f) { // Valor mínimo para evitar deformaciones
            correctedDistance = 0.01f;
        }

        float projectedHEIGHT = (scale / correctedDistance);

        if (projectedHEIGHT > 1.0f) projectedHEIGHT = 1.0f;

        float stripWIDTH = 2.0f / 320.0f; // Assuming 320 strips
        float brightness = 1.0f / (1.0f + correctedDistance * 0.5f);
        if (brightness > 1.0f) brightness = 1.0f;
        glColor3f(brightness, brightness, brightness);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBegin(GL_QUADS);
        // Top-Left Vertex (u, v)
        glTexCoord2f(texX, 1.0f);
        glVertex2f(xOffset, projectedHEIGHT);

        // Bottom-Left Vertex (u, v)
        glTexCoord2f(texX, 0.0f);
        glVertex2f(xOffset, -projectedHEIGHT);

        // Bottom-Right Vertex (u, v)
        glTexCoord2f(texX, 0.0f);
        glVertex2f(xOffset + stripWIDTH, -projectedHEIGHT);

        // Top-Right Vertex (u, v)
        glTexCoord2f(texX, 1.0f);
        glVertex2f(xOffset + stripWIDTH, projectedHEIGHT);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
}

void Draw3DView() {
    glViewport(WIDTH / 2, 0, WIDTH / 2, HEIGHT);
    glScissor(WIDTH / 2, 0, WIDTH / 2, HEIGHT);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    int numRays = HEIGHT; // Number of vertical strips to render
    float angleStep = FOV / numRays;

    float startAngle = playerAngle - FOV / 2;

    // Draw Sky
    glColor3f(0.5f, 0.75f, 1.0f); // Sky color
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, 0.0f);  // Bottom-Left
    glVertex2f(1.0f, 0.0f);   // Bottom-Right
    glVertex2f(1.0f, 1.0f);   // Top-Right
    glVertex2f(-1.0f, 1.0f);  // Top-Left
    glEnd();

    // Draw Ground
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f); // Bottom-Left
    glVertex2f(1.0f, -1.0f);  // Bottom-Right
    glVertex2f(1.0f, 0.0f);   // Top-Right
    glVertex2f(-1.0f, 0.0f);  // Top-Left
    glEnd();

    for (int i = 0; i < numRays; i++) {
        float rayAngle = startAngle + (i * angleStep);
        rayAngle = normalizeAngle(rayAngle);

        struct RaycastHit hit = DDA(playerPos, rayAngle, 0);

        float xOffset = -1.0f + (2.0f * i) / numRays;

        float angleDiff = rayAngle - playerAngle;
        angleDiff = normalizeAngleDiff(angleDiff);

        GLuint textureId;
        if (hit.wallType > 0 && hit.wallType <= textureManager.textureCount) {
            textureId = textureManager.textureIds[hit.wallType - 1];
        }
        else {
            textureId = textureManager.textureIds[0]; // Default texture
        }

        // Pass texX to Draw3DSquare
        Draw3DSquare(hit.distance, angleDiff, xOffset, textureId, hit.texX);
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
    window = glfwCreateWindow(WIDTH, HEIGHT, "Raycaster", NULL, NULL);
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

    InitTextureManager();

    int greyStoneIndex = AddTexture("src/Textures/greystone.png");
    int redBrickIndex = AddTexture("src/Textures/redbrick.png");

    if (greyStoneIndex == -1 || redBrickIndex == -1) {
        printf("Failed to load one or more textures.\n");
        glfwTerminate();
        return -1;
    }

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        frameCount++;

        // If a second has passed, calculate FPS
        if (currentTime - previousTime >= 1.0) {
            fps = (double)frameCount / (currentTime - previousTime);
            previousTime = currentTime;
            frameCount = 0;

            // Print FPS to console (optional)
            printf("FPS: %.2f\n", fps);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        DrawGameView();
        Draw3DView();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up textures
    for (int i = 0; i < textureManager.textureCount; i++) {
        glDeleteTextures(1, &textureManager.textureIds[i]);
    }

    glfwTerminate();
    return 0;
}
