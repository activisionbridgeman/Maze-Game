#include "Health.h"
#include <iostream>

// Healthpoint class (gives 1 health point by default)
Health::Health(int x, int y, int deltaX, int deltaY)
	: PlacableActor(x, y)
{
	this->SetPosition(m_pPosition->x, m_pPosition->y);
	setHealthPoints(1);
}

void Health::Draw()
{
	std::cout << (char)49;
}
