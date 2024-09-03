#include "include/FluidSimulation.hpp"
#include "include/raylib.h"
#include "include/rlgl.h"
#include <iostream>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int NUM_SIM_STEPS_PER_FRAME = 3;
bool simulationPaused = true;

int main() {
	// Initialization
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fluid Simulation");

	// Setup camera
	Camera2D camera = {0};
	camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
	camera.zoom = 1.1f;

	// Setup simulation
	FluidSimulation sim;
	sim.gravity = 15.f;
	sim.collisionDamping = 0.4f;
	sim.numParticles = 4096;
	sim.smoothingRadius = 23;
	sim.mouseRadius=80;
	sim.pressureMultiplier = 2700.f;
	sim.targetDensity = 0.007f;
	sim.particleSize = 3.1f;
	sim.particleSpacing = 1.9f;
	sim.boundsSize = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT};
	sim.Start();

	while (!WindowShouldClose()) {
		std::cout<<"FPS: "<<GetFPS()<<"\n";
		if (IsKeyPressed(KEY_SPACE))
			simulationPaused = !simulationPaused;
		if (IsKeyPressed(KEY_R))
			sim.Start();
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
