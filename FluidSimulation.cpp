#include "include/FluidSimulation.hpp"
#include "include/raymath.h"

void FluidSimulation::initParticlesRandomly() {
	Vector2 halfBoundsSize=Vector2SubtractValue(
		Vector2Scale(boundsSize, 0.5),
		particleSize);

	for (int i=0; i<numParticles; i++) {
		positions[i] = (Vector2){
			(float)GetRandomValue(-halfBoundsSize.x, halfBoundsSize.x),
			(float)GetRandomValue(-halfBoundsSize.y, halfBoundsSize.y)};
		velocities[i] = (Vector2){0, 0};
	}
}

void FluidSimulation::initParticlesInSquare() {
	int particlesPerRow=(int)sqrt(numParticles);
	int particlesPerCol=(numParticles - 1) / particlesPerRow + 1;
	float spacing = particleSize * 2 + particleSpacing;

	for (int i=0; i<numParticles; i++) {
		positions[i]=(Vector2){
			(i%particlesPerRow-particlesPerRow/2.f+0.5f)*spacing,
			(i/particlesPerRow-particlesPerCol/2.f+0.5f)*spacing
		};
		velocities[i]=(Vector2){0, 0};
	}
}

void FluidSimulation::Start() {
	positions.resize(numParticles);
	velocities.resize(numParticles);
	densities.resize(numParticles);
	spatialLookup.resize(numParticles);
	startIndices.resize(numParticles);
	mass=1.f;
	initParticlesInSquare();
	// initParticlesRandomly();
}

float FluidSimulation::densityToPressure(float density) {
	float densityError = density - targetDensity;
	return densityError * pressureMultiplier;
}

void FluidSimulation::resolveCollisions(Vector2& position, Vector2& velocity) {
	Vector2 halfBoundsSize=Vector2SubtractValue(
		Vector2Scale(boundsSize, 0.5),
		particleSize);
	if (abs(position.x)>halfBoundsSize.x) {
		position.x=halfBoundsSize.x*(2*(position.x>=0)-1);
		velocity.x *= -1 * collisionDamping;
	}
	if (abs(position.y)>halfBoundsSize.y) {
		position.y=halfBoundsSize.y*(2*(position.y>=0)-1);
		velocity.y *= -1 * collisionDamping;
	}
}

float FluidSimulation::smoothingKernelDerivative(float distance) {
	if (distance>=smoothingRadius) return 0;
	return -30.f*pow(smoothingRadius-distance,2)/(PI*pow(smoothingRadius,5));
}

float FluidSimulation::smoothingKernel(float distance) {
	float volume=PI*pow(smoothingRadius,5)/10;
	return pow(fmax(0, smoothingRadius-distance),3)/volume;
}

float FluidSimulation::calculateDensity(Vector2 sampleParticle) {
	float density=0.f;

	std::vector<int> particlesWithinRadius=getPointsWithinRadius(sampleParticle);
	for (int i : particlesWithinRadius) {
		float distance=Vector2Distance(sampleParticle, positions[i]);
		float influence=smoothingKernel(distance);
		density+=influence*mass;
	}

	return density;
}

Vector2 FluidSimulation::calculatePressureForce(int sampleParticleIdx) {
	Vector2 pressureForce=(Vector2){0, 0};
	std::vector<int> particlesWithinRadius=getPointsWithinRadius(positions[sampleParticleIdx]);
	for (int i:particlesWithinRadius) {
		if (i==sampleParticleIdx) continue;
		Vector2 difference=Vector2Subtract(positions[i],positions[sampleParticleIdx]);
		float distance=Vector2Length(difference);
		Vector2 direction=distance==0?(Vector2){1.0f, 0.0f}:Vector2Scale(difference,1.f/distance);
		float influenceMagnitude=smoothingKernelDerivative(distance);
		float density=densities[i];
		float pressure=densityToPressure(density);
		float otherPressure=densityToPressure(densities[sampleParticleIdx]);
		float sharedPressure=(pressure+otherPressure)/2;
		float scalar=sharedPressure*influenceMagnitude*mass/density;
		pressureForce=Vector2Add(pressureForce, Vector2Scale(direction,scalar));
	}
	return pressureForce;
}

int FluidSimulation::findClosestParticle() {
	int j=0;
	float bestDst=100000;
	for (int i=0; i<numParticles; i++) {
		Vector2 mousePosition=Vector2Subtract(
			GetMousePosition(),
			Vector2Scale(boundsSize, 0.5f)
		);
		mousePosition.y=-mousePosition.y;
		float distanceToParticle=Vector2Distance(mousePosition,positions[i]);
		if (distanceToParticle<bestDst) {
			j=i;
			bestDst=distanceToParticle;
		}
	}
	return j;
}

CellCoord FluidSimulation::positionToCellCoord(Vector2 position) {
	return (CellCoord){
		(int)(position.x/smoothingRadius),
		(int)(position.y/smoothingRadius)
	};
}

unsigned int FluidSimulation::hashCell(CellCoord cell) {
	unsigned int a = (unsigned int)cell.x*15823;
	unsigned int b = (unsigned int)cell.y*9737333;
	return a+b;
}

unsigned int FluidSimulation::getKeyFromHash(unsigned int hash) {
	return hash%(unsigned int)(spatialLookup.size());
}

bool compareByCellKey(const SpatialLookupEntry& a, const SpatialLookupEntry& b) {
	return a.cellKey < b.cellKey;
}

void FluidSimulation::updateSpatialLookup() {
	PARALLEL_FOR_BEGIN(numParticles) {
		spatialLookup[i]=(SpatialLookupEntry){
			i, getKeyFromHash(hashCell(positionToCellCoord(positions[i])))
		};
		startIndices[i]=INT_MAX;
	}PARALLEL_FOR_END();
	std::sort(spatialLookup.begin(), spatialLookup.end(), compareByCellKey);
	PARALLEL_FOR_BEGIN(numParticles) {
		unsigned int key=spatialLookup[i].cellKey;
		unsigned int keyPrev=i==0?2*INT_MAX:spatialLookup[i-1].cellKey;
		if (key!=keyPrev) {
			startIndices[key]=i;
		}
	}PARALLEL_FOR_END();
}

void FluidSimulation::Update(float deltaTime) {
	updateSpatialLookup();
	PARALLEL_FOR_BEGIN(numParticles) {
		velocities[i].y-=gravity*deltaTime;
		densities[i]=calculateDensity(positions[i]);
	}PARALLEL_FOR_END();

	PARALLEL_FOR_BEGIN(numParticles) {
		Vector2 pressureForce=calculatePressureForce(i);
		Vector2 acceleration=Vector2Scale(pressureForce,1.f/densities[i]);
		velocities[i]=Vector2Add(velocities[i], Vector2Scale(acceleration,deltaTime));
	}PARALLEL_FOR_END();

	PARALLEL_FOR_BEGIN(numParticles) {
		positions[i] = Vector2Add(positions[i], velocities[i]);
		resolveCollisions(positions[i], velocities[i]);
	}PARALLEL_FOR_END();
}

std::vector<int> FluidSimulation::getPointsWithinRadius(Vector2 point) {
	std::vector<CellCoord> cellOffsets = {
		(CellCoord){-1,-1},
		(CellCoord){-1,0},
		(CellCoord){-1,1},
		(CellCoord){0,-1},
		(CellCoord){0,0},
		(CellCoord){0,1},
		(CellCoord){1,-1},
		(CellCoord){1,0},
		(CellCoord){1,1},
	};

	CellCoord coord=positionToCellCoord(point);
	float sqrSmoothingRadius=smoothingRadius*smoothingRadius;
	std::vector<int> points;

	for (CellCoord offset : cellOffsets) {
		unsigned int key=getKeyFromHash(hashCell((CellCoord){
			offset.x+coord.x,
			offset.y+coord.y
		}));
		for (int i=startIndices[key]; i<spatialLookup.size(); i++) {
			if (spatialLookup[i].cellKey!=key) break;
			int particleIdx=spatialLookup[i].particleIndex;
			float sqrDist=Vector2DistanceSqr(positions[particleIdx],point);
			if (sqrDist<sqrSmoothingRadius)
				points.push_back(particleIdx);
		}
	}
	return points;
}

void FluidSimulation::Render() {
	for (int i=0; i<numParticles; i++) {
		DrawCircleV(positions[i], particleSize, (Color){
			63, 120, 244, 255
		});
	}
}
