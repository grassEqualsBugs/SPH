#include "include/FluidSimulation.hpp"
#include "include/raylib.h"
#include "include/rlgl.h"
#include <iostream>

const int SCREEN_WIDTH = 1470;
const int SCREEN_HEIGHT = 890;
const int NUM_SIM_STEPS_PER_FRAME = 4;
bool simulationPaused = true;

int main() {
	// Initialization
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fluid Simulation");

	// Setup camera
	Camera2D camera = {0};
	camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
	camera.zoom = 1.f;

	// Setup simulation
	FluidSimulation sim;
	sim.collisionDamping = 0.8f;
	sim.numParticles = 3600;
	sim.mouseRadius=180;
	sim.mouseFlag=false;
	sim.forceType=1;
	sim.viscosityStrength=1000.f;

	sim.gravity = 10.f;
	sim.pressureMultiplier = 6000.f;
	sim.targetDensity = 0.f;
	sim.smoothingRadius = 18;
	// sim.gravity=0.f;
    // sim.pressureMultiplier=1000.f;
    // sim.targetDensity=0.f;
    // sim.smoothingRadius = 24;

	sim.particleSize = 2.8f;
	sim.particleSpacing = 0.9f;
	sim.boundsSize = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT};
	sim.Start();

	while (!WindowShouldClose()) {
		std::cout<<"FPS: "<<GetFPS()<<"\n";
		if (IsKeyPressed(KEY_SPACE))
			simulationPaused = !simulationPaused;
		if (IsKeyPressed(KEY_R))
			sim.Start();
		if (IsKeyPressed(KEY_M))
			sim.forceType=-sim.forceType;
		sim.mouseFlag=0;
		if (IsKeyDown(KEY_N))
			sim.mouseFlag=1;
		if (!simulationPaused||(simulationPaused&&IsKeyPressed(KEY_RIGHT))) {
			for (int i = 0; i < NUM_SIM_STEPS_PER_FRAME; i++)
				sim.SimulationStep(GetFrameTime() / NUM_SIM_STEPS_PER_FRAME);
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
