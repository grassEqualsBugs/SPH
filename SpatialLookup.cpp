#include "include/SpatialLookup.hpp"

SpatialLookup::SpatialLookup() {
	cellOffsets = {
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
}

void SpatialLookup::Resize(int size) {
	spatialLookup.resize(size);
	startIndices.resize(size);
}

bool compareByCellKey(const SpatialLookupEntry& a, const SpatialLookupEntry& b) {
	return a.cellKey < b.cellKey;
}

void SpatialLookup::UpdateSpatialLookup(std::vector<Vector2> newPoints, float newRadius) {
	points=newPoints;
	radius=newRadius;
	PARALLEL_FOR_BEGIN(points.size()) {
		spatialLookup[i]=(SpatialLookupEntry){
			i, getKeyFromHash(hashCell(positionToCellCoord(points[i])))
		};
		startIndices[i]=INT_MAX;
	}PARALLEL_FOR_END();
	std::sort(spatialLookup.begin(), spatialLookup.end(), compareByCellKey);
	PARALLEL_FOR_BEGIN(points.size()) {
		unsigned int key=spatialLookup[i].cellKey;
		unsigned int keyPrev=i==0?2*INT_MAX:spatialLookup[i-1].cellKey;
		if (key!=keyPrev) {
			startIndices[key]=i;
		}
	}PARALLEL_FOR_END();
}

std::vector<int> SpatialLookup::GetPointsWithinRadius(Vector2 point) {
	CellCoord coord=positionToCellCoord(point);
	float sqrSmoothingRadius=radius*radius;
	std::vector<int> pointsWithinRadius;

	for (CellCoord offset : cellOffsets) {
		unsigned int key=getKeyFromHash(hashCell((CellCoord){
			offset.x+coord.x,
			offset.y+coord.y
		}));
		for (int i=startIndices[key]; i<spatialLookup.size(); i++) {
			if (spatialLookup[i].cellKey!=key) break;
			int particleIdx=spatialLookup[i].particleIndex;
			float sqrDist=Vector2DistanceSqr(points[particleIdx],point);
			if (sqrDist<sqrSmoothingRadius)
				pointsWithinRadius.push_back(particleIdx);
		}
	}
	return pointsWithinRadius;
}

CellCoord SpatialLookup::positionToCellCoord(Vector2 position) {
	return (CellCoord){
		(int)(position.x/radius),
		(int)(position.y/radius)
	};
}

unsigned int SpatialLookup::hashCell(CellCoord cell) {
	unsigned int a = (unsigned int)cell.x*15823;
	unsigned int b = (unsigned int)cell.y*9737333;
	return a+b;
}

unsigned int SpatialLookup::getKeyFromHash(unsigned int hash) {
	return hash%(unsigned int)(spatialLookup.size());
}
