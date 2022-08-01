#pragma once
#include "PlacableActor.h"

// Healthpoint class (gives 1 health point by default)
class Health : public PlacableActor
{
public:
	Health(int x, int y, int deltaX = 0, int deltaY = 0);

	virtual void Draw() override;
	virtual ActorType GetType() override { return ActorType::Health; }

	void setHealthPoints(int healthPoints)
	{
		this->healthPoints = healthPoints;
	}

	int getHealthPoints() 
	{
		return healthPoints;
	}

protected:
	int healthPoints;
};
