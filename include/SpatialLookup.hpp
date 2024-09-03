#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include "raymath.h"
#include "parallel.hpp"

typedef struct SpatialLookupEntry {
	int particleIndex;
	unsigned int cellKey;
} SpatialLookupEntry;

typedef struct CellCoord {
	int x;
	int y;
} CellCoord;

class SpatialLookup {
	private:
		std::vector<SpatialLookupEntry> spatialLookup;
		std::vector<int> startIndices;
		float radius;
		std::vector<Vector2> points;
		std::vector<CellCoord> cellOffsets;

		CellCoord positionToCellCoord(Vector2 position);
		unsigned int hashCell(CellCoord cell);
		unsigned int getKeyFromHash(unsigned int hash);
	public:
		SpatialLookup();
		void Resize(int size);
		void UpdateSpatialLookup(std::vector<Vector2> newPoints, float newRadius);
		std::vector<int> GetPointsWithinRadius(Vector2 point);
};
