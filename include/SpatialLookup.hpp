#pragma once
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
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
		const std::vector<Vector2>* points;
		std::vector<CellCoord> cellOffsets;

		CellCoord positionToCellCoord(Vector2 position);
		unsigned int hashCell(CellCoord cell);
		unsigned int getKeyFromHash(unsigned int hash);
	public:
		SpatialLookup();
		void Resize(int size);
		void UpdateSpatialLookup(const std::vector<Vector2>& newPoints, float newRadius);
		void ForEachPointWithinRadius(Vector2 point, const std::function<void(int)>& callback);
};
