#pragma once
#include "Health.h"

// Large healthpoint class (gives 3 health points)
class LargeHealth : public Health
{
public:
	LargeHealth(int x, int y, int deltaX = 0, int deltaY = 0);

	// REFACTORED: Removed GetType(), since it is already inherited from Health
	virtual void Draw() override;
};
