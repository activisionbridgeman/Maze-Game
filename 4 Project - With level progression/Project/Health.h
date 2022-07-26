#pragma once
#include "PlacableActor.h"

class Health : public PlacableActor
{
public:
	Health(int x, int y, int deltaX = 0, int deltaY = 0);

	virtual ActorType GetType() override { return ActorType::Health; }
	virtual void Draw() override;

private:
	int healthPoints;

};
