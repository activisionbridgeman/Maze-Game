#include "LargeHealth.h"
#include <iostream>

// Large healthpoint class (gives 3 health points)
LargeHealth::LargeHealth(int x, int y, int deltaX, int deltaY)
	: Health(x, y, deltaX, deltaY) 
{ 
	setHealthPoints(3);
}

void LargeHealth::Draw()
{
	std::cout << (char)51;
}