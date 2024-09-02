#pragma once
#include "raylib.h"
#include "raymath.h"
#include "parallel.hpp"

#include <algorithm>
#include <climits>
#include <vector>
#include <iostream>

typedef struct SpatialLookupEntry {
	int particleIndex;
	unsigned int cellKey;
} SpatialLookupEntry;

typedef struct CellCoord {
	int x;
	int y;
} CellCoord;

class FluidSimulation {
	private:
		void initParticlesRandomly();
		void initParticlesInSquare();

		void resolveCollisions(Vector2& position, Vector2& velocity);
		std::vector<Vector2> positions;
		std::vector<Vector2> velocities;
		std::vector<float> densities;
		std::vector<SpatialLookupEntry> spatialLookup;
		std::vector<int> startIndices;
		float mass;

		float smoothingKernel(float distance);
		float smoothingKernelDerivative(float distance);
		float calculateDensity(Vector2 particle);
		float densityToPressure(float density);
		Vector2 calculatePressureForce(int sampleParticleIdx);

		int findClosestParticle();

		CellCoord positionToCellCoord(Vector2 position);
		unsigned int hashCell(CellCoord cell);
		unsigned int getKeyFromHash(unsigned int hash);
		void updateSpatialLookup();
		std::vector<int> getPointsWithinRadius(Vector2 point);
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
