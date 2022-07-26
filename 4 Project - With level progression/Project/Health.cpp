#include "Health.h"
#include <iostream>

Health::Health(int x, int y, int deltaX, int deltaY)
	: PlacableActor(x, y), healthPoints(1)
{
	this->SetPosition(m_pPosition->x, m_pPosition->y);
}

void Health::Draw()
{
	std::cout << (char)72;
}
