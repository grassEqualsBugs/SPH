#include "include/raylib.h"
#include "include/rlgl.h"
#include "include/FluidSimulation.hpp"
#include <iostream>

const int SCREEN_WIDTH=1280;
const int SCREEN_HEIGHT=720;
const int NUM_SIM_STEPS_PER_FRAME=3;
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
    sim.numParticles=3249;
    sim.smoothingRadius=24;
    sim.pressureMultiplier=1200.f;
    sim.targetDensity=0.001f;
    sim.particleSize=3.2f;
    sim.particleSpacing=2.5f;
    sim.boundsSize=(Vector2){SCREEN_WIDTH, SCREEN_HEIGHT};
    sim.Start();

    while (!WindowShouldClose()) {
     	if (IsKeyPressed(KEY_SPACE)) simulationPaused=!simulationPaused;
     	if (!simulationPaused) {
      		for (int i=0; i<NUM_SIM_STEPS_PER_FRAME; i++)
      			sim.SimulationStep(GetFrameTime()/NUM_SIM_STEPS_PER_FRAME);
      	}
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
