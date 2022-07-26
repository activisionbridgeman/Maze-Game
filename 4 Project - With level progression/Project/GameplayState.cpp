#include "GameplayState.h"

#include <iostream>
#include <conio.h>
#include <windows.h>
#include <assert.h>
#include <thread>

#include "Enemy.h"
#include "Key.h"
#include "Door.h"
#include "Money.h"
#include "Goal.h"
#include "AudioManager.h"
#include "Utility.h"
#include "StateMachineExampleGame.h"
#include "Health.h"
#include "LargeHealth.h"

using namespace std;

constexpr int kArrowInput = 224;
constexpr int kLeftArrow = 75;
constexpr int kRightArrow = 77;
constexpr int kUpArrow = 72;
constexpr int kDownArrow = 80;
constexpr int kEscapeKey = 27;

std::thread UpdateActorsThread;
std::thread DecreaseTimeThread;
std::vector<std::thread> EnemyRespawnThreads;

bool gameOver;
time_t currTime;

static const int maxTime = 30; // in seconds
static const int delayTime = 500000; // in microseconds
static const int enemyRespawnTime = 3; // in seconds

GameplayState::GameplayState(StateMachineExampleGame* pOwner)
	: m_pOwner(pOwner)
	, m_beatLevel(false)
	, m_skipFrameCount(0)
	, m_currentLevel(0)
	, m_pLevel(nullptr)
{
	m_LevelNames.push_back("Level4.txt");
	m_LevelNames.push_back("Level1.txt");
	m_LevelNames.push_back("Level2.txt");
	m_LevelNames.push_back("Level3.txt");
}

// Terminate threads by setting passing variable to true and joining them
void TerminateThreads() 
{
	gameOver = true;
	UpdateActorsThread.join();
	DecreaseTimeThread.join();

	for (int i = 0; i < EnemyRespawnThreads.size(); i++) 
	{
		EnemyRespawnThreads[i].join();
	}
}

GameplayState::~GameplayState()
{
	TerminateThreads();
	delete m_pLevel;
	m_pLevel = nullptr;
}

// Move actors as long as game is active
void GameplayState::UpdateActors()
{
	while (!gameOver)
	{
		m_pLevel->MoveActors();
	}
}

// Decrease time by one second at a time
void GameplayState::DecreaseTime() 
{
	while (!gameOver) 
	{
		currTime--;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

bool GameplayState::Load()
{
	if (m_pLevel)
	{
		delete m_pLevel;
		m_pLevel = nullptr;
	}

	m_pLevel = new Level();

	gameOver = false;

	// Initialize threads
	UpdateActorsThread = std::thread (&GameplayState::UpdateActors, this);
	DecreaseTimeThread = std::thread(&GameplayState::DecreaseTime, this);
	EnemyRespawnThreads.clear();

	currTime = maxTime;
	
	return m_pLevel->Load(m_LevelNames.at(m_currentLevel), m_player.GetXPositionPointer(), m_player.GetYPositionPointer());
}

void GameplayState::Enter()
{
	Load();
}

bool GameplayState::Update(bool processInput)
{
	// Lose game if time hits 0
	if (currTime <= 0)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(delayTime));
		AudioManager::GetInstance()->PlayLoseSound();
		m_pOwner->LoadScene(StateMachineExampleGame::SceneName::Lose);
	}

	if (processInput && !m_beatLevel)
	{
		int input = 0;

		// User kbhit to continuously monitor player input
		if (_kbhit()) {
			input = _getch();
		}
		int arrowInput = 0;
		int newPlayerX = m_player.GetXPosition();
		int newPlayerY = m_player.GetYPosition();

		// One of the arrow keys were pressed
		if (input == kArrowInput)
		{
			arrowInput = _getch();
		}

		if ((input == kArrowInput && arrowInput == kLeftArrow) ||
			(char)input == 'A' || (char)input == 'a')
		{
			newPlayerX--;
		}
		else if ((input == kArrowInput && arrowInput == kRightArrow) ||
			(char)input == 'D' || (char)input == 'd')
		{
			newPlayerX++;
		}
		else if ((input == kArrowInput && arrowInput == kUpArrow) ||
			(char)input == 'W' || (char)input == 'w')
		{
			newPlayerY--;
		}
		else if ((input == kArrowInput && arrowInput == kDownArrow) ||
			(char)input == 'S' || (char)input == 's')
		{
			newPlayerY++;
		}
		else if (input == kEscapeKey)
		{
			m_pOwner->LoadScene(StateMachineExampleGame::SceneName::MainMenu);
		}
		else if ((char)input == 'Z' || (char)input == 'z')
		{
			m_player.DropKey();
		}

		HandleCollision(newPlayerX, newPlayerY);
	}
	if (m_beatLevel)
	{
		++m_skipFrameCount;
		if (m_skipFrameCount > kFramesToSkip)
		{
			m_beatLevel = false;
			m_skipFrameCount = 0;
			++m_currentLevel;
			if (m_currentLevel == m_LevelNames.size())
			{
				Utility::WriteHighScore(m_player.GetMoney());

				AudioManager::GetInstance()->PlayWinSound();
				
				m_pOwner->LoadScene(StateMachineExampleGame::SceneName::Win);
			}
			else
			{
				// On to the next level
				Load();
			}

		}
	}

	return false;
}

// Respawn enemy if it is destroyed after enemyRespawnTime seconds
void GameplayState::RespawnEnemy(Enemy* collidedEnemy, int x, int y)
{
	collidedEnemy->Remove();

	time_t prevTime = time(NULL);
	while (!gameOver && ((time(NULL) - prevTime) < enemyRespawnTime)) { }

	if (!gameOver)
	{
		collidedEnemy->Place(x, y);
	}
}

void GameplayState::HandleCollision(int newPlayerX, int newPlayerY)
{
	PlacableActor* collidedActor = m_pLevel->UpdateActors(newPlayerX, newPlayerY);
	if (collidedActor != nullptr && collidedActor->IsActive())
	{
		switch (collidedActor->GetType())
		{
		case ActorType::Enemy:
		{
			Enemy* collidedEnemy = dynamic_cast<Enemy*>(collidedActor);
			assert(collidedEnemy);
			AudioManager::GetInstance()->PlayLoseLivesSound();
			EnemyRespawnThreads.push_back(std::thread(&GameplayState::RespawnEnemy, this, collidedEnemy, collidedEnemy->GetXPosition(), collidedEnemy->GetYPosition()));
			m_player.SetPosition(newPlayerX, newPlayerY);

			// Breakpoint placed here to check that it is hit when player collides with enemy
			// and to ensure that m_lives decrements by one
			m_player.DecrementLives();
			if (m_player.GetLives() < 0)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(delayTime));
				AudioManager::GetInstance()->PlayLoseSound();
				m_pOwner->LoadScene(StateMachineExampleGame::SceneName::Lose);
			}
			break;
		}
		case ActorType::Money:
		{
			Money* collidedMoney = dynamic_cast<Money*>(collidedActor);
			assert(collidedMoney);
			AudioManager::GetInstance()->PlayMoneySound();
			collidedMoney->Remove();
			m_player.AddMoney(collidedMoney->GetWorth());
			m_player.SetPosition(newPlayerX, newPlayerY);
			break;
		}
		case ActorType::Key:
		{
			Key* collidedKey = dynamic_cast<Key*>(collidedActor);
			assert(collidedKey);
			if (!m_player.HasKey())
			{
				m_player.PickupKey(collidedKey);
				collidedKey->Remove();
				m_player.SetPosition(newPlayerX, newPlayerY);
				AudioManager::GetInstance()->PlayKeyPickupSound();
			}
			break;
		}
		case ActorType::Door:
		{
			Door* collidedDoor = dynamic_cast<Door*>(collidedActor);
			assert(collidedDoor);
			if (!collidedDoor->IsOpen())
			{
				if (m_player.HasKey(collidedDoor->GetColor()))
				{
					collidedDoor->Open();
					collidedDoor->Remove();
					m_player.UseKey();
					m_player.SetPosition(newPlayerX, newPlayerY);
					AudioManager::GetInstance()->PlayDoorOpenSound();
				}
				else
				{
					AudioManager::GetInstance()->PlayDoorClosedSound();
				}
			}
			else
			{
				m_player.SetPosition(newPlayerX, newPlayerY);
			}
			break;
		}
		case ActorType::Goal:
		{
			Goal* collidedGoal = dynamic_cast<Goal*>(collidedActor);
			assert(collidedGoal);
			collidedGoal->Remove();
			m_player.SetPosition(newPlayerX, newPlayerY);
			m_beatLevel = true;
			TerminateThreads();
			break;
		}
		// REFACTORED: Only one case is needed for Health and LargeHealth, since they are both of type Health
		case ActorType::Health:
		{
			Health* collidedHealth = dynamic_cast<Health*>(collidedActor);
			assert(collidedHealth);
			AudioManager::GetInstance()->PlayGainLivesSound();
			collidedHealth->Remove();
			m_player.SetPosition(newPlayerX, newPlayerY);

			if (m_player.GetLives() < 3) 
			{
				m_player.IncrementLives(collidedHealth->getHealthPoints());
			}
			break;
		}
		default:
			break;
		}
	}
	else if (m_pLevel->IsSpace(newPlayerX, newPlayerY)) // no collision
	{
		m_player.SetPosition(newPlayerX, newPlayerY);
	}
	else if (m_pLevel->IsWall(newPlayerX, newPlayerY))
	{
		// wall collision, do nothing
	}
}

void GameplayState::Draw()
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	system("cls");

	m_pLevel->Draw();

	// Set cursor position for player 
	COORD actorCursorPosition;
	actorCursorPosition.X = m_player.GetXPosition();
	actorCursorPosition.Y = m_player.GetYPosition();
	SetConsoleCursorPosition(console, actorCursorPosition);
	m_player.Draw();

	// Set the cursor to the end of the level
	COORD currentCursorPosition;
	currentCursorPosition.X = 0;
	currentCursorPosition.Y = m_pLevel->GetHeight();
	SetConsoleCursorPosition(console, currentCursorPosition);

	DrawHUD(console);
}

void GameplayState::DrawHUD(const HANDLE& console)
{
	cout << endl;

	// Top Border
	for (int i = 0; i < m_pLevel->GetWidth(); ++i)
	{
		cout << Level::WAL;
	}
	cout << endl;

	// Left Side border
	cout << Level::WAL;

	cout << " wasd-move " << Level::WAL << " z-drop key " << Level::WAL;

	cout << " $:" << m_player.GetMoney() << " " << Level::WAL;
	cout << " lives:" << m_player.GetLives() << " " << Level::WAL;
	cout << " time:" << currTime << " " << Level::WAL;
	cout << " key:";
	if (m_player.HasKey())
	{
		m_player.GetKey()->Draw();
	}
	else
	{
		cout << " ";
	}

	// RightSide border
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console, &csbi);

	COORD pos;
	pos.X = m_pLevel->GetWidth() - 1;
	pos.Y = csbi.dwCursorPosition.Y;
	SetConsoleCursorPosition(console, pos);

	//cout << Level::WAL;
	cout << endl;

	// Bottom Border
	for (int i = 0; i < m_pLevel->GetWidth(); ++i)
	{
		cout << Level::WAL;
	}
	cout << endl;
}