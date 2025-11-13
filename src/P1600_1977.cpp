/*
 * Ancient East Asian Warriors - Collectibles Game
 * Computer Graphics Assignment 2
 * Theme: Ancient East Asian Warriors (Ninjas/Samurais)
 * 
 * CONTROLS:
 * Movement: W/A/S/D or Arrow Keys
 * Camera Views: 1=Top, 2=Side, 3=Front, 0=Free
 * Mouse: Click+Drag for free camera rotation
 * Animations (after collecting all items): Z, X, C, V
 * R: Restart game
 * ESC: Exit
 */

// macOS uses different include paths
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

// ==================== CONSTANTS ====================
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const float GROUND_SIZE = 50.0f;
const float WALL_HEIGHT = 10.0f;
const float PLAYER_SPEED = 0.3f;
const int GAME_TIME = 120; // seconds

// ==================== STRUCTURES ====================
struct Vector3 {
    float x, y, z;
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Color {
    float r, g, b;
    Color(float r = 1, float g = 1, float b = 1) : r(r), g(g), b(b) {}
};

struct Collectible {
    Vector3 position;
    bool collected;
    int platform;
    Collectible(Vector3 pos, int plat) : position(pos), collected(false), platform(plat) {}
};

struct Platform {
    Vector3 position;
    Vector3 size;
    Color color;
    bool allCollected;
    bool animationActive;
    float animationValue;
    int animationType;
    Platform(Vector3 pos, Vector3 sz, Color col, int anim) 
        : position(pos), size(sz), color(col), allCollected(false), 
          animationActive(false), animationValue(0), animationType(anim) {}
};

// ==================== GLOBAL VARIABLES ====================
enum GameState { PLAYING, WIN, GAME_OVER };
GameState gameState = PLAYING;
int gameTimeRemaining = GAME_TIME;
int lastTime = 0;

Vector3 playerPos(0, 0.5, 0);
float playerRotation = 0.0f;

Vector3 cameraPos(0, 15, 25);
float cameraAngleX = 30.0f;
float cameraAngleY = 0.0f;
float cameraDistance = 35.0f;
int mouseX = 0, mouseY = 0;
bool mouseDown = false;
int cameraMode = 0;

std::vector<Platform> platforms;
std::vector<Collectible> collectibles;
float globalRotation = 0.0f;

bool keys[256] = {false};
bool specialKeys[256] = {false};

// ==================== UTILITY FUNCTIONS ====================
float distance(Vector3 a, Vector3 b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

// ==================== DRAWING PRIMITIVES ====================
void drawCube(float size, Color color) {
    glColor3f(color.r, color.g, color.b);
    glutSolidCube(size);
}

void drawSphere(float radius, Color color) {
    glColor3f(color.r, color.g, color.b);
    glutSolidSphere(radius, 20, 20);
}

void drawCylinder(float radius, float height, Color color) {
    glColor3f(color.r, color.g, color.b);
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, radius, radius, height, 20, 20);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void drawCone(float radius, float height, Color color) {
    glColor3f(color.r, color.g, color.b);
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(quad, radius, 0, height, 20, 20);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

void drawTorus(float innerRadius, float outerRadius, Color color) {
    glColor3f(color.r, color.g, color.b);
    glutSolidTorus(innerRadius, outerRadius, 16, 16);
}

// ==================== GAME OBJECTS ====================

// Ground (1 primitive)
void drawGround() {
    glPushMatrix();
    glColor3f(0.2f, 0.3f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-GROUND_SIZE, 0, -GROUND_SIZE);
    glVertex3f(GROUND_SIZE, 0, -GROUND_SIZE);
    glVertex3f(GROUND_SIZE, 0, GROUND_SIZE);
    glVertex3f(-GROUND_SIZE, 0, GROUND_SIZE);
    glEnd();
    
    glColor3f(0.3f, 0.4f, 0.3f);
    glBegin(GL_LINES);
    for (float i = -GROUND_SIZE; i <= GROUND_SIZE; i += 5) {
        glVertex3f(i, 0.01f, -GROUND_SIZE);
        glVertex3f(i, 0.01f, GROUND_SIZE);
        glVertex3f(-GROUND_SIZE, 0.01f, i);
        glVertex3f(GROUND_SIZE, 0.01f, i);
    }
    glEnd();
    glPopMatrix();
}

// Walls (3 primitives - 1 each)
void drawWalls() {
    // Front wall
    glPushMatrix();
    glTranslatef(0, WALL_HEIGHT/2, -GROUND_SIZE);
    glScalef(GROUND_SIZE * 2, WALL_HEIGHT, 0.5f);
    drawCube(1, Color(0.8f, 0.2f, 0.2f));
    glPopMatrix();
    
    // Left wall
    glPushMatrix();
    glTranslatef(-GROUND_SIZE, WALL_HEIGHT/2, 0);
    glScalef(0.5f, WALL_HEIGHT, GROUND_SIZE * 2);
    drawCube(1, Color(0.8f, 0.2f, 0.2f));
    glPopMatrix();
    
    // Right wall
    glPushMatrix();
    glTranslatef(GROUND_SIZE, WALL_HEIGHT/2, 0);
    glScalef(0.5f, WALL_HEIGHT, GROUND_SIZE * 2);
    drawCube(1, Color(0.8f, 0.2f, 0.2f));
    glPopMatrix();
}

// Player Character (8 primitives)
void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerPos.x, playerPos.y, playerPos.z);
    glRotatef(playerRotation, 0, 1, 0);
    
    // Legs (2 cylinders)
    glPushMatrix();
    glTranslatef(-0.2f, -0.3f, 0);
    drawCylinder(0.15f, 0.6f, Color(0.1f, 0.1f, 0.1f));
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.2f, -0.3f, 0);
    drawCylinder(0.15f, 0.6f, Color(0.1f, 0.1f, 0.1f));
    glPopMatrix();
    
    // Torso (cube)
    glPushMatrix();
    glTranslatef(0, 0.4f, 0);
    glScalef(0.8f, 1.0f, 0.5f);
    drawCube(1, Color(0.8f, 0.0f, 0.0f));
    glPopMatrix();
    
    // Arms (2 cylinders)
    glPushMatrix();
    glTranslatef(-0.5f, 0.4f, 0);
    glRotatef(90, 0, 0, 1);
    drawCylinder(0.1f, 0.4f, Color(0.8f, 0.0f, 0.0f));
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.5f, 0.4f, 0);
    glRotatef(-90, 0, 0, 1);
    drawCylinder(0.1f, 0.4f, Color(0.8f, 0.0f, 0.0f));
    glPopMatrix();
    
    // Head (sphere)
    glPushMatrix();
    glTranslatef(0, 1.1f, 0);
    drawSphere(0.3f, Color(0.9f, 0.8f, 0.7f));
    glPopMatrix();
    
    // Hat (cone)
    glPushMatrix();
    glTranslatef(0, 1.4f, 0);
    drawCone(0.4f, 0.5f, Color(0.1f, 0.1f, 0.1f));
    glPopMatrix();
    
    // Sword (cylinder)
    glPushMatrix();
    glTranslatef(-0.7f, 0.6f, 0);
    glRotatef(45, 0, 0, 1);
    drawCylinder(0.05f, 1.2f, Color(0.7f, 0.7f, 0.8f));
    glPopMatrix();
    
    glPopMatrix();
}

// Platform (2 primitives each)
void drawPlatform(Platform& platform) {
    glPushMatrix();
    glTranslatef(platform.position.x, platform.position.y, platform.position.z);
    
    // Base
    glPushMatrix();
    glScalef(platform.size.x, platform.size.y, platform.size.z);
    drawCube(1, platform.color);
    glPopMatrix();
    
    // Top
    glPushMatrix();
    glTranslatef(0, platform.size.y/2 + 0.1f, 0);
    glScalef(platform.size.x * 1.1f, 0.2f, platform.size.z * 1.1f);
    Color topColor(platform.color.r * 0.8f, platform.color.g * 0.8f, platform.color.b * 0.8f);
    drawCube(1, topColor);
    glPopMatrix();
    
    glPopMatrix();
}

// Collectible (3 primitives)
void drawCollectible(Vector3 pos) {
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(globalRotation * 2, 0, 1, 0);
    
    // Center sphere
    drawSphere(0.2f, Color(1.0f, 0.84f, 0.0f));
    
    // Ring
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    drawTorus(0.05f, 0.3f, Color(0.9f, 0.7f, 0.0f));
    glPopMatrix();
    
    // Top cone
    glPushMatrix();
    glTranslatef(0, 0.3f, 0);
    drawCone(0.15f, 0.2f, Color(1.0f, 0.84f, 0.0f));
    glPopMatrix();
    
    glPopMatrix();
}

// Platform 1: Lantern (5 primitives) - Rotation
void drawLantern(Platform& platform) {
    glPushMatrix();
    // Position above the platform
    glTranslatef(platform.position.x, platform.position.y + 3, platform.position.z);

    float rot = 0.0f;
    float bob = 0.0f;
    // Use a fixed color for the lantern so rotation does not change its color
    const float fixedGlow = 0.84f;
    if (platform.animationActive) {
        // fast rotation and vertical bob (no color pulsing)
        rot = platform.animationValue * 3.0f;
        bob = 0.25f * sin(platform.animationValue * 0.05f);
    }
    glTranslatef(0, bob, 0);
    glRotatef(rot, 0, 1, 0);

    // Top
    drawCylinder(0.3f, 0.5f, Color(0.6f, 0.3f, 0.0f));

    // Middle sphere (fixed color)
    glPushMatrix();
    glTranslatef(0, 0.8f, 0);
    drawSphere(0.6f, Color(1.0f, fixedGlow, 0.0f));
    glPopMatrix();

    // Bottom cone
    glPushMatrix();
    glTranslatef(0, 1.5f, 0);
    drawCone(0.4f, 0.5f, Color(0.6f, 0.3f, 0.0f));
    glPopMatrix();

    // Chain
    glPushMatrix();
    glTranslatef(0, -0.5f, 0);
    drawCylinder(0.05f, 0.5f, Color(0.5f, 0.5f, 0.5f));
    glPopMatrix();

    // Ring
    glPushMatrix();
    glTranslatef(0, 2.2f, 0);
    glRotatef(90, 1, 0, 0);
    drawTorus(0.1f, 0.5f, Color(0.8f, 0.6f, 0.0f));
    glPopMatrix();

    glPopMatrix();
}

// Platform 2: Pagoda (6 primitives) - Scaling
void drawPagoda(Platform& platform) {
    glPushMatrix();
    glTranslatef(platform.position.x, platform.position.y + 2, platform.position.z);

    if (platform.animationActive) {
        // Asymmetric breathing scale and slight tilt for pagoda
        float s1 = 1.0f + 0.35f * sin(platform.animationValue * 0.035f);
        float s2 = 1.0f + 0.15f * sin(platform.animationValue * 0.04f + 1.0f);
        float s3 = 1.0f + 0.25f * sin(platform.animationValue * 0.03f + 2.0f);
        glScalef(s1, s2, s3);
        glRotatef(sin(platform.animationValue * 0.015f) * 6.0f, 0, 0, 1);
    }

    // Base
    glPushMatrix();
    glScalef(1.5f, 0.3f, 1.5f);
    drawCube(1, Color(0.5f, 0.0f, 0.0f));
    glPopMatrix();
    
    // Level 1
    glPushMatrix();
    glTranslatef(0, 0.5f, 0);
    glScalef(1.2f, 0.8f, 1.2f);
    drawCube(1, Color(0.8f, 0.1f, 0.1f));
    glPopMatrix();
    
    // Level 2
    glPushMatrix();
    glTranslatef(0, 1.2f, 0);
    glScalef(0.9f, 0.6f, 0.9f);
    drawCube(1, Color(0.8f, 0.1f, 0.1f));
    glPopMatrix();
    
    // Level 3
    glPushMatrix();
    glTranslatef(0, 1.7f, 0);
    glScalef(0.6f, 0.4f, 0.6f);
    drawCube(1, Color(0.8f, 0.1f, 0.1f));
    glPopMatrix();
    
    // Roof
    glPushMatrix();
    glTranslatef(0, 2.2f, 0);
    drawCone(0.8f, 0.7f, Color(0.3f, 0.2f, 0.1f));
    glPopMatrix();
    
    // Top sphere
    glPushMatrix();
    glTranslatef(0, 3.0f, 0);
    drawSphere(0.2f, Color(1.0f, 0.84f, 0.0f));
    glPopMatrix();
    
    glPopMatrix();
}

// Platform 3: Statue (7 primitives) - Translation
void drawStatue(Platform& platform) {
    glPushMatrix();
    // We'll orbit the statue slightly around its platform center when animating
    float orbitX = 0.0f;
    float orbitZ = 0.0f;
    float offsetY = 0.0f;
    float rotY = 0.0f;
    if (platform.animationActive) {
        float ang = platform.animationValue * 3.14159f / 180.0f; // radians
        float radius = 0.6f;
        orbitX = radius * cos(ang * 0.6f);
        orbitZ = radius * sin(ang * 0.6f);
        offsetY = 0.6f * sin(ang * 1.2f);
        rotY = fmod(platform.animationValue * 0.2f, 360.0f);
    }
    glTranslatef(platform.position.x + orbitX, platform.position.y + 2 + offsetY, platform.position.z + orbitZ);
    glRotatef(rotY, 0, 1, 0);
    
    // Base
    glPushMatrix();
    glScalef(1.0f, 0.5f, 1.0f);
    drawCube(1, Color(0.4f, 0.4f, 0.4f));
    glPopMatrix();
    
    // Lower body
    glPushMatrix();
    glTranslatef(0, 1.0f, 0);
    drawCylinder(0.4f, 1.0f, Color(0.6f, 0.5f, 0.4f));
    glPopMatrix();
    
    // Upper body
    glPushMatrix();
    glTranslatef(0, 2.0f, 0);
    glScalef(0.8f, 0.6f, 0.5f);
    drawCube(1, Color(0.6f, 0.5f, 0.4f));
    glPopMatrix();
    
    // Head
    glPushMatrix();
    glTranslatef(0, 2.8f, 0);
    drawSphere(0.35f, Color(0.6f, 0.5f, 0.4f));
    glPopMatrix();
    
    // Left arm
    glPushMatrix();
    glTranslatef(-0.5f, 1.8f, 0);
    drawCylinder(0.15f, 0.7f, Color(0.6f, 0.5f, 0.4f));
    glPopMatrix();
    
    // Right arm
    glPushMatrix();
    glTranslatef(0.5f, 1.8f, 0);
    drawCylinder(0.15f, 0.7f, Color(0.6f, 0.5f, 0.4f));
    glPopMatrix();
    
    // Crown
    glPushMatrix();
    glTranslatef(0, 3.2f, 0);
    glRotatef(90, 1, 0, 0);
    drawTorus(0.1f, 0.4f, Color(1.0f, 0.84f, 0.0f));
    glPopMatrix();
    
    glPopMatrix();
}

// Platform 4: Weapon Rack (5 primitives) - Color Change
void drawWeaponRack(Platform& platform) {
    glPushMatrix();
    glTranslatef(platform.position.x, platform.position.y + 2, platform.position.z);

    // Sword swing angle and color change when active
    float swing = 0.0f;
    Color weaponColor(0.7f, 0.7f, 0.8f);
    if (platform.animationActive) {
        swing = sin(platform.animationValue * 0.06f) * 25.0f; // swing degrees
        float r = 0.4f + 0.6f * fabs(sin(platform.animationValue * 0.03f));
        float g = 0.4f + 0.6f * fabs(sin(platform.animationValue * 0.03f + 2.0f));
        float b = 0.4f + 0.6f * fabs(sin(platform.animationValue * 0.03f + 4.0f));
        weaponColor = Color(r, g, b);
    }

    // Stand base
    glPushMatrix();
    glScalef(1.0f, 0.2f, 0.5f);
    drawCube(1, Color(0.3f, 0.2f, 0.1f));
    glPopMatrix();

    // Vertical support
    glPushMatrix();
    glTranslatef(0, 0.8f, 0);
    drawCylinder(0.1f, 1.5f, Color(0.3f, 0.2f, 0.1f));
    glPopMatrix();

    // Sword 1 (left) - swings
    glPushMatrix();
    glTranslatef(-0.3f, 1.2f, 0);
    glRotatef(30 + swing, 0, 0, 1);
    drawCylinder(0.05f, 1.5f, weaponColor);
    glPopMatrix();

    // Sword 2 (right) - swings opposite
    glPushMatrix();
    glTranslatef(0.3f, 1.2f, 0);
    glRotatef(-30 - swing, 0, 0, 1);
    drawCylinder(0.05f, 1.5f, weaponColor);
    glPopMatrix();

    // Top ornament
    glPushMatrix();
    glTranslatef(0, 2.5f, 0);
    drawSphere(0.2f, Color(1.0f, 0.84f, 0.0f));
    glPopMatrix();

    glPopMatrix();
}

// ==================== TEXT RENDERING ====================
void renderText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
    
    glEnable(GL_LIGHTING);
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void renderHUD() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    char buffer[100];
    
    // Timer
    sprintf(buffer, "Time: %d:%02d", gameTimeRemaining / 60, gameTimeRemaining % 60);
    renderText(10, WINDOW_HEIGHT - 30, buffer, GLUT_BITMAP_TIMES_ROMAN_24);
    
    // Collectibles
    int collected = 0;
    for (const auto& c : collectibles) if (c.collected) collected++;
    sprintf(buffer, "Collected: %d/%d", collected, (int)collectibles.size());
    renderText(10, WINDOW_HEIGHT - 60, buffer);
    
    // Platform status
    renderText(10, WINDOW_HEIGHT - 90, "Platforms:");
    for (size_t i = 0; i < platforms.size(); i++) {
        sprintf(buffer, "P%d: %s %s", (int)i+1, 
                platforms[i].allCollected ? "✓" : "✗",
                platforms[i].animationActive ? "[ON]" : "[OFF]");
        renderText(10, WINDOW_HEIGHT - 110 - i*20, buffer);
    }
    
    // Controls
    renderText(10, 50, "WASD: Move | 1/2/3: Views | Z/X/C/V: Animations");
    renderText(10, 30, "Mouse: Camera | R: Restart | ESC: Exit");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void renderWinScreen() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Semi-transparent overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0, 0, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();
    glDisable(GL_BLEND);
    
    // Win message
    glColor3f(1, 1, 0);
    renderText(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2 + 50, "CONGRATULATIONS!", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1, 1, 1);
    renderText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2, "YOU WIN!");
    renderText(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 - 40, "Press R to restart");
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void renderGameOverScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glColor3f(0.1f, 0, 0);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1, 0, 0);
    renderText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 50, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1, 1, 1);
    renderText(WINDOW_WIDTH/2 - 80, WINDOW_HEIGHT/2, "Time's Up!");
    renderText(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 - 40, "Press R to restart");
    renderText(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 70, "Press ESC to exit");
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glEnable(GL_LIGHTING);
}

// ==================== GAME LOGIC ====================
void initGame() {
    srand(time(NULL));
    
    platforms.clear();
    platforms.push_back(Platform(Vector3(-15, 0.5, -15), Vector3(5, 1, 5), Color(0.8f, 0.2f, 0.2f), 0));
    platforms.push_back(Platform(Vector3(15, 0.5, -15), Vector3(5, 1, 5), Color(0.2f, 0.8f, 0.2f), 1));
    platforms.push_back(Platform(Vector3(-15, 0.5, 15), Vector3(5, 1, 5), Color(0.2f, 0.2f, 0.8f), 2));
    platforms.push_back(Platform(Vector3(15, 0.5, 15), Vector3(5, 1, 5), Color(0.8f, 0.8f, 0.2f), 3));
    
    collectibles.clear();
    for (size_t i = 0; i < platforms.size(); i++) {
        for (int j = 0; j < 3; j++) {
            float offsetX = (rand() % 3 - 1) * 1.5f;
            float offsetZ = (rand() % 3 - 1) * 1.5f;
            // Place collectibles slightly above the top surface of the platform so they're visible
            float collectY = platforms[i].position.y + platforms[i].size.y/2.0f + 0.6f;
            Vector3 pos(platforms[i].position.x + offsetX, collectY, platforms[i].position.z + offsetZ);
            collectibles.push_back(Collectible(pos, i));
        }
    }
    
    gameState = PLAYING;
    gameTimeRemaining = GAME_TIME;
    playerPos = Vector3(0, 0.5, 0);
    playerRotation = 0;
}

void checkCollectibles() {
    for (auto& c : collectibles) {
        if (!c.collected) {
            float dist = distance(playerPos, c.position);
            // collect when close enough
            if (dist < 1.5f) {
                c.collected = true;
                std::cout << "Collectible picked up!" << std::endl;
            }
        }
    }
    
    for (size_t i = 0; i < platforms.size(); i++) {
        bool allCollected = true;
        for (const auto& c : collectibles) {
            if (c.platform == i && !c.collected) {
                allCollected = false;
                break;
            }
        }
        // If platform was not complete and now is, mark complete and auto-enable animation
        if (!platforms[i].allCollected && allCollected) {
            platforms[i].allCollected = true;
            platforms[i].animationActive = true; // auto-enable animation on completion
            std::cout << "Platform " << (i+1) << " completed. Animation auto-enabled." << std::endl;
        } else {
            platforms[i].allCollected = allCollected;
        }
    }
    
    bool allComplete = true;
    for (const auto& p : platforms) {
        if (!p.allCollected) {
            allComplete = false;
            break;
        }
    }
    if (allComplete && gameState == PLAYING) {
        gameState = WIN;
        std::cout << "YOU WIN!" << std::endl;
    }
}

bool checkCollision(Vector3 newPos) {
    if (newPos.x < -GROUND_SIZE + 1 || newPos.x > GROUND_SIZE - 1) return true;
    if (newPos.z < -GROUND_SIZE + 1 || newPos.z > GROUND_SIZE - 1) return true;
    return false;
}

// ==================== OPENGL CALLBACKS ====================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    if (gameState == GAME_OVER) {
        renderGameOverScreen();
        glutSwapBuffers();
        return;
    }
    
    // Set camera
    if (cameraMode == 1) { // Top view
        gluLookAt(playerPos.x, 40, playerPos.z, 
                 playerPos.x, 0, playerPos.z, 
                 0, 0, -1);
    } else if (cameraMode == 2) { // Side view
        gluLookAt(40, 10, playerPos.z, 
                 0, 10, playerPos.z, 
                 0, 1, 0);
    } else if (cameraMode == 3) { // Front view
        gluLookAt(playerPos.x, 10, 40, 
                 playerPos.x, 10, 0, 
                 0, 1, 0);
    } else { // Free camera
        float camX = cameraDistance * sin(cameraAngleY * M_PI / 180.0) * cos(cameraAngleX * M_PI / 180.0);
        float camY = cameraDistance * sin(cameraAngleX * M_PI / 180.0);
        float camZ = cameraDistance * cos(cameraAngleY * M_PI / 180.0) * cos(cameraAngleX * M_PI / 180.0);
        gluLookAt(playerPos.x + camX, playerPos.y + camY, playerPos.z + camZ,
                 playerPos.x, playerPos.y, playerPos.z,
                 0, 1, 0);
    }
    
    // Draw scene
    drawGround();
    drawWalls();
    drawPlayer();
    
    // Draw platforms
    for (auto& platform : platforms) {
        drawPlatform(platform);
    }
    
    // Draw platform objects
    drawLantern(platforms[0]);
    drawPagoda(platforms[1]);
    drawStatue(platforms[2]);
    drawWeaponRack(platforms[3]);
    
    // Draw collectibles
    for (const auto& c : collectibles) {
        if (!c.collected) {
            drawCollectible(c.position);
        }
    }
    
    // Draw HUD
    renderHUD();
    
    // Draw win screen overlay
    if (gameState == WIN) {
        renderWinScreen();
    }
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void update(int value) {
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    int deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Update timer
    if (gameState == PLAYING) {
        static int timeAccumulator = 0;
        timeAccumulator += deltaTime;
        if (timeAccumulator >= 1000) {
            gameTimeRemaining--;
            timeAccumulator = 0;
            if (gameTimeRemaining <= 0) {
                gameState = GAME_OVER;
            }
        }
    }
    
    // Update animations
    globalRotation += 1.0f;
    if (globalRotation > 360) globalRotation -= 360;
    
    for (auto& platform : platforms) {
        if (platform.animationActive) {
            // Use different speeds per platform (animationType)
            float speed = 2.0f;
            switch (platform.animationType) {
                case 0: speed = 4.0f; break; // lantern - faster rotation
                case 1: speed = 2.5f; break; // pagoda - medium scale
                case 2: speed = 3.5f; break; // statue - bouncy
                case 3: speed = 2.0f; break; // weapon rack - color shift
                default: speed = 2.0f; break;
            }
            platform.animationValue += speed;
            if (platform.animationValue > 360) platform.animationValue -= 360;
        }
    }
    
    // Update player movement
    // Allow player movement while PLAYING or after WIN (player retains control on win screen)
    if (gameState == PLAYING || gameState == WIN) {
        Vector3 newPos = playerPos;
        bool moved = false;
        
        if (keys['w'] || keys['W'] || specialKeys[GLUT_KEY_UP]) {
            newPos.z -= PLAYER_SPEED;
            playerRotation = 180;
            moved = true;
        }
        if (keys['s'] || keys['S'] || specialKeys[GLUT_KEY_DOWN]) {
            newPos.z += PLAYER_SPEED;
            playerRotation = 0;
            moved = true;
        }
        if (keys['a'] || keys['A'] || specialKeys[GLUT_KEY_LEFT]) {
            newPos.x -= PLAYER_SPEED;
            playerRotation = 90;
            moved = true;
        }
        if (keys['d'] || keys['D'] || specialKeys[GLUT_KEY_RIGHT]) {
            newPos.x += PLAYER_SPEED;
            playerRotation = 270;
            moved = true;
        }
        
        if (moved && !checkCollision(newPos)) {
            playerPos = newPos;
            static int moveCounter = 0;
            if (++moveCounter % 30 == 0) { // Print every 30 frames (0.5 seconds)
                std::cout << "Player moved to: (" << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << ")" << std::endl;
            }
        }
        
        // Continue to allow collectible checks (optional) so HUD updates if items are collected after win
        checkCollectibles();
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;
    
    if (key == 27) { // ESC
        exit(0);
    }
    
    if (key == 'r' || key == 'R') {
        initGame();
        lastTime = glutGet(GLUT_ELAPSED_TIME);
    }
    
    // Camera modes
    if (key == '0') cameraMode = 0; // Free
    if (key == '1') cameraMode = 1; // Top
    if (key == '2') cameraMode = 2; // Side
    if (key == '3') cameraMode = 3; // Front
    
    // Animation toggles (only if platform complete)
    if (key == 'z' || key == 'Z') {
        if (platforms.size() > 0 && platforms[0].allCollected) {
            platforms[0].animationActive = !platforms[0].animationActive;
            std::cout << "P1 animation toggled to " << (platforms[0].animationActive ? "ON" : "OFF") << std::endl;
        } else {
            std::cout << "P1 not complete yet. Collect all items to enable animation." << std::endl;
        }
    }
    if (key == 'x' || key == 'X') {
        if (platforms.size() > 1 && platforms[1].allCollected) {
            platforms[1].animationActive = !platforms[1].animationActive;
            std::cout << "P2 animation toggled to " << (platforms[1].animationActive ? "ON" : "OFF") << std::endl;
        } else {
            std::cout << "P2 not complete yet. Collect all items to enable animation." << std::endl;
        }
    }
    if (key == 'c' || key == 'C') {
        if (platforms.size() > 2 && platforms[2].allCollected) {
            platforms[2].animationActive = !platforms[2].animationActive;
            std::cout << "P3 animation toggled to " << (platforms[2].animationActive ? "ON" : "OFF") << std::endl;
        } else {
            std::cout << "P3 not complete yet. Collect all items to enable animation." << std::endl;
        }
    }
    if (key == 'v' || key == 'V') {
        if (platforms.size() > 3 && platforms[3].allCollected) {
            platforms[3].animationActive = !platforms[3].animationActive;
            std::cout << "P4 animation toggled to " << (platforms[3].animationActive ? "ON" : "OFF") << std::endl;
        } else {
            std::cout << "P4 not complete yet. Collect all items to enable animation." << std::endl;
        }
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void specialKeysCallback(int key, int x, int y) {
    specialKeys[key] = true;
}

void specialKeysUpCallback(int key, int x, int y) {
    specialKeys[key] = false;
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseDown = true;
            mouseX = x;
            mouseY = y;
        } else {
            mouseDown = false;
        }
    }
    
    // Mouse wheel for zoom
    if (button == 3) { // Scroll up
        cameraDistance -= 2.0f;
        if (cameraDistance < 5.0f) cameraDistance = 5.0f;
    }
    if (button == 4) { // Scroll down
        cameraDistance += 2.0f;
        if (cameraDistance > 60.0f) cameraDistance = 60.0f;
    }
}

void mouseMotion(int x, int y) {
    if (mouseDown && cameraMode == 0) {
        int deltaX = x - mouseX;
        int deltaY = y - mouseY;
        
        cameraAngleY += deltaX * 0.5f;
        cameraAngleX += deltaY * 0.5f;
        
        if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
        if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;
        
        mouseX = x;
        mouseY = y;
    }
}

void initGL() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Setup lighting
    GLfloat lightPos[] = { 0.0f, 30.0f, 0.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
}

// ==================== MAIN ====================
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ancient Warriors - Collectibles Game");
    
    initGL();
    initGame();
    
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeysCallback);
    glutSpecialUpFunc(specialKeysUpCallback);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(16, update, 0);
    
    std::cout << "=== Ancient Warriors Game ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD / Arrow Keys - Move character" << std::endl;
    std::cout << "  1 - Top View" << std::endl;
    std::cout << "  2 - Side View" << std::endl;
    std::cout << "  3 - Front View" << std::endl;
    std::cout << "  0 - Free Camera (Mouse control)" << std::endl;
    std::cout << "  Z/X/C/V - Toggle animations (after collecting)" << std::endl;
    std::cout << "  R - Restart game" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "=============================" << std::endl;
    
    glutMainLoop();
    return 0;
}