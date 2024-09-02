#include "include/raylib.h"
#include "include/rlgl.h"
#include "include/FluidSimulation.hpp"
#include <iostream>

const int SCREEN_WIDTH=1280;
const int SCREEN_HEIGHT=720;
bool simulationPaused=true;

int main() {
	// Initialization
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fluid Simulation");

    // Setup camera
    Camera2D camera = {0};
    camera.offset=(Vector2){SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f};
    camera.zoom=1.0f;

    // Setup simulation
    FluidSimulation sim;
    sim.gravity=0.f;
    sim.collisionDamping=0.95f;
    sim.numParticles=3025;
    sim.smoothingRadius=80;
    sim.pressureMultiplier=800.f;
    sim.targetDensity=0.0004f;
    sim.particleSize=2.5f;
    sim.particleSpacing=4.9f;
    sim.boundsSize=(Vector2){SCREEN_WIDTH, SCREEN_HEIGHT};
    sim.Start();

    while (!WindowShouldClose()) {
    	std::cout<<"FPS: "<<GetFPS()<<"\n";
     	if (IsKeyPressed(KEY_SPACE)) simulationPaused=!simulationPaused;
     	if (!simulationPaused) sim.Update(GetFrameTime());
    	rlSetCullFace(RL_CULL_FACE_FRONT);
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
        rlPushMatrix();
        rlScalef(1.0f, -1.0f, 1.0f);
        sim.Render();
        rlPopMatrix();
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
