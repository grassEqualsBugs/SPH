#pragma once
#include "raylib.h"
#include "raymath.h"
#include "parallel.hpp"
#include "SpatialLookup.hpp"

#include <algorithm>
#include <climits>
#include <vector>
#include <iostream>
#include <limits>

class FluidSimulation {
	private:
		void initParticlesRandomly();
		void initParticlesInSquare();

		void resolveCollisions(Vector2& position, Vector2& velocity);
		std::vector<Vector2> positions;
		std::vector<Vector2> predictedPositions;
		std::vector<Vector2> velocities;
		std::vector<float> densities;
		float mass;
		SpatialLookup spatialLookup;

		float smoothingKernel(float distance);
		float smoothingKernelDerivative(float distance);
		float calculateDensity(Vector2 particle);
		float densityToPressure(float density);
		Vector2 calculatePressureForce(int sampleParticleIdx);

		int findClosestParticle();
		Vector2 calculateMouseForce(int particleIdx, Vector2 mousePos, float strength);
	public:
		float targetDensity;
		float pressureMultiplier;
		float gravity;
		int forceType;
		bool mouseFlag;
		float collisionDamping;
		float mouseRadius;
		float particleSize;
		float particleSpacing;
		float smoothingRadius;
		unsigned int numParticles;
		Vector2 boundsSize;
		void Start();
		void Reset();
		void SimulationStep(float deltaTime);
		void Render();
};
