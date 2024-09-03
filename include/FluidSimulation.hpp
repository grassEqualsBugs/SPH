#pragma once
#include "raylib.h"
#include "raymath.h"
#include "parallel.hpp"
#include "SpatialLookup.hpp"

#include <algorithm>
#include <climits>
#include <vector>
#include <iostream>

class FluidSimulation {
	private:
		void initParticlesRandomly();
		void initParticlesInSquare();

		void resolveCollisions(Vector2& position, Vector2& velocity);
		std::vector<Vector2> positions;
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
	public:
		float targetDensity;
		float pressureMultiplier;
		float gravity;
		float collisionDamping;
		float particleSize;
		float particleSpacing;
		float smoothingRadius;
		unsigned int numParticles;
		Vector2 boundsSize;
		void Start();
		void Update(float deltaTime);
		void Render();
};
