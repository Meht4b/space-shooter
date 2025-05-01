#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <Windows.h>

using namespace std;

int nScreenWidth = 120;			// Console Screen Size X (columns)
int nScreenHeight = 40;			// Console Screen Size Y (rows)


int nFakeUnitAlive = -1000;
float fPlayerX = 20.0f;
float fPlayerY = 20.0f;
float fSpeed = 40.0f;			// Walking Speed
float fBulletSpeed = 70.0f;		//bullet speed
float fBulletFreq = 0.03f;		//bullet frequence in seconds
int nMaxBulletCount = 100;		//max bullet count 
float fBulletLifeTime = 3.0f;	//bullet life time in seconds

//function to log to the screen
void logScreen(wchar_t* screen, int nScreenWidth, string str, int x, int y, int runInf = 1, float timeBegin = 0, float lifeTime = 0, float currentTime = 0) {

	if (currentTime > timeBegin and currentTime < timeBegin + lifeTime or runInf)
	{
		for (int i = x; i < x + str.length(); i++) {
			screen[y * nScreenWidth + i] = str[i - x];
		}
	}
}

//function to clear the screen
void ClearScreen(int nScreenWidth, int nScreenHeight, wchar_t* screen) {
	for (int x = 0; x < nScreenWidth; x++)
	{
		for (int y = 0; y < nScreenHeight; y++) {
			screen[y * nScreenWidth + x] = ' ';

		}
	}
}

//function to clear the screen and draw the backgrounds
void ClearScreenDrawBg(int nScreenWidth, int nScreenHeight, wchar_t* screen) {
	for (int x = 0; x < nScreenWidth; x++)
	{
		for (int y = 0; y < nScreenHeight; y++) {
			screen[y * nScreenWidth + x] = ' ';
			if (rand() % 10000 < 2) {
				screen[y * nScreenWidth + x] = '.';
			}
		}
	}
}

//Bullet class
class Bullet
{
	float fPosX, fPosY, fSpeed;
	float fBirthTime,fLifeTime;
	char cShape;
	int* nBulletsActive; //current amount of units on screen
	float fCoolDownTime;
	bool flag;

public:
	Bullet() {
		fPosX = 0; fPosY = 0; fSpeed = 0; cShape = ' '; fBirthTime = 0;
	}

	void set(float posx, float posy, float birthtime,float lifetime, float speed, char shape,int* bulletsactive, float cooldowntime) {
		fPosX = posx;
		fPosY = posy;
		fSpeed = speed;
		cShape = shape;
		fBirthTime = birthtime;
		fLifeTime = lifetime;
		fCoolDownTime = cooldowntime;
		nBulletsActive = bulletsactive;
		flag = 0;

	}

	int update(float fElapsedTime, float fCurrentTime, int nScreenWidth, int nScreenHeight, wchar_t* screen) {
		fPosX += fSpeed * fElapsedTime;					//move the bullet sideways

		//if the bullet is in bounds
		if (fPosY < nScreenHeight and fPosY>=0 and fPosX < nScreenWidth and fPosX >= 0)		
		{											
			draw(nScreenWidth, screen);						//draw the bullet
		}

		if (fCurrentTime - fBirthTime > fCoolDownTime) {	//if the bullets cooldown has ender
			if (flag) (*nBulletsActive)--;
			flag = 0;
		}
		else {
			flag = 1;
		}

		if (fCurrentTime - fBirthTime > fLifeTime) {		//if the bullets life has ended
			return 0;										//mark the bullet as free
		}
		else
		{
			return 1;									//mark the bullet as used
		}
	}

	void draw(int nScreenWidth, wchar_t* screen) {
		screen[(int)fPosX + (int)fPosY * nScreenWidth] = cShape; //draw the bullet on the screen
	}

};

//Data structure which lets us manage multiple units of the same type with least memory & least memory operations
template <typename Unit>
class SwarmManager
{
	Unit* arrUnit; //array of given units
	int* arrUnitFree;
	int nMaxUnit; //max amount of units there can be on screen
	int* nUnitsAlive; //current amount of units on screen
	int nUnitSwarmCount; //number of units in one swarm
	float fBulletCooldown; //cooldown of the units bw swarms
	

public:
	SwarmManager() {
		nMaxUnit = 0;
		arrUnit = nullptr; //initialize the array of units
		arrUnitFree = nullptr; //initialize the array of indices of free units

	}

	SwarmManager(int maxunit, int* unitsalive = &nFakeUnitAlive,int unitswarmcount = 100, float fbulletcooldown = 0)
	{
		set(maxunit,unitsalive,unitswarmcount,fbulletcooldown);
	}

	~SwarmManager()
	{
		delete[] arrUnit;
		delete[] arrUnit; //delete the array of units
	}

	void set(int maxunit,int* unitsalive,int unitswarmcount,float fbulletcooldown) {

		delete[] arrUnit;
		delete[] arrUnitFree;
		

		nMaxUnit = maxunit;
		arrUnit = new Unit[nMaxUnit]; //create the array of units
		arrUnitFree = new int[nMaxUnit]; //create the array of indices of free units
		for (int i = 0; i < nMaxUnit; i++) {
			arrUnitFree[i] = 0; //initialize the array of indices of free units
		}

		nUnitSwarmCount = unitswarmcount;
		fBulletCooldown = fbulletcooldown;
		nUnitsAlive = unitsalive; //initialize the number of units on screen
	}

	Unit* add() {
		for (int i = 0; i < nMaxUnit; i++) {
			if (arrUnitFree[i] == 0 and (*nUnitsAlive)< nUnitSwarmCount) {
				arrUnitFree[i] = 1; //mark the unit as used
				(*nUnitsAlive)++;
				return &arrUnit[i]; //return the unit
			}
		}
		return nullptr; //if there are no free units, return null
	}

	void addMultiple(int n, void (*f)(Unit*, int) ){
		Unit* temp;
		for (int i = 0; i < n; i++) {
			temp = add();
			if (temp) {
				f(temp, i); //call the function to set the unit
			}
		}
	}
	

	void update(float fElapsedTime, float fCurrentTime, int nScreenWidth, int nScreenHeight, wchar_t* screen) {
		for (int i = 0; i < nMaxUnit; i++) {
			if (arrUnitFree[i]) {
				//the update function determines whether to be marked as in service or out of service
				arrUnitFree[i] = arrUnit[i].update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);

			}
		}
		
	}
};

//Base class for other classes
class Entity
{
protected:
	float fPosX, fPosY, fSpeed;
	float fBulletFreq; //how often the entity can shoot bullets
	float fBulletSpeed; //speed of the bullet
	float fBirthTime; //time which the object was created
	float fLifeTime; //how long the object will live
	float fBulletLifeTime; //how long the bullet will live
	int nMaxBullet; //max amount of bullet there can be on screen
public:
	Entity() {
		fPosX = 0;
		fPosY = 0;
		fSpeed = 0;
		fBulletFreq = 0;
		fBulletSpeed = 0;
		nMaxBullet = 0;
		fBirthTime = 0;
		fLifeTime = 0;
	}

	void reset() {
		fPosX = 0;
		fPosY = 0;
		fSpeed = 0;
		fBulletFreq = 0;
		fBulletSpeed = 0;
		nMaxBullet = 0;
		fBirthTime = 0;
	}

	void set(float posx, float posy, float birthtime, float lifetime, float speed, float bulletf, float bulletspeed, int maxbullet) {
		fPosX = posx;
		fPosY = posy;
		fSpeed = speed;
		fBirthTime = birthtime;
		fBulletFreq = bulletf;
		fBulletSpeed = bulletspeed;
		nMaxBullet = maxbullet;
		fLifeTime = lifetime;
	}

};

//Player class
class Player : private Entity
{

	SwarmManager<Bullet> smBullets; //array of bullets
	float fPrevBulletSpawnTime; //time of the last bullet spawn
	int nBulletActive; //current amount of units on screen
	float fBulletCooldown; //cooldown of the bullets
	int nBulletSwarmCount;

public:
	Player() {
		fPosX = 0;
		fPosY = 0;
		fSpeed = 0;
		fBulletFreq = 0;
		fBulletSpeed = 0;
		nMaxBullet = 0;
		fPrevBulletSpawnTime = 0;
	}

	Player(float posx, float posy, float speed, float bulletf, float bulletspeed, int maxbullet, float bulletlifetime,float bulletcooldown, int bulletswarmcount)
	{
		fPosX = posx; fPosY = posy; fBulletFreq = bulletf; fBulletSpeed = bulletspeed; nMaxBullet = maxbullet;
		fSpeed = speed;
		fPrevBulletSpawnTime = 0;
		fBulletLifeTime = bulletlifetime;
		nBulletActive = 0;
		fBulletCooldown = bulletcooldown;
		nBulletSwarmCount = bulletswarmcount;
		smBullets.set(maxbullet, &nBulletActive, bulletswarmcount,fBulletCooldown); //create the array of bullets
	}

	void updateBullets(float fElapsedTime, float fCurrentTime, int nScreenWidth, int nScreenHeight, wchar_t* screen) {
		smBullets.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen); //update the bullets
	}

	int update(float fElapsedTime, float fCurrentTime, int nScreenWidth, int nScreenHeight, wchar_t* screen)
	{
		//handle wsad movement while checking if player is in bounds
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000 and fPosX > 2)
			fPosX -= fSpeed * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000 and fPosX < nScreenWidth - 1)
			fPosX += fSpeed * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000 and fPosY > 2)
			fPosY -= fSpeed * fElapsedTime / 1.5;

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000 and fPosY < nScreenHeight - 1)
			fPosY += fSpeed * fElapsedTime / 1.5;

		//getting player back in bound if they exit the bounds
		if (fPosY <= 0)
			fPosY = 1;

		else if (fPosY >= nScreenHeight - 1)
			fPosY = nScreenHeight - 2;

		if (fPosX <= 0)
			fPosX = 1;

		else if (fPosX >= nScreenWidth - 1)
			fPosX = nScreenWidth - 2;

		//shooting bullets
		if (GetAsyncKeyState((unsigned short)' ') & 0x8000)				//if the space key is pressed
		{
			if (fCurrentTime - fPrevBulletSpawnTime > fBulletFreq)
			{															//if the bullet frequency has passed
				Bullet* pBullet = smBullets.add();						//add a bullet to the array
				if (pBullet != nullptr)
				{														//if there is a bullet available
																		//set the bullet position and speed

					pBullet->set(fPosX, fPosY, fCurrentTime, fBulletLifeTime, fBulletSpeed, '-',&nBulletActive,fBulletCooldown);

					fPrevBulletSpawnTime = fCurrentTime;				//reset the prev spawn time
				}
			}
		}
		//if the player is hit by an enemy	
		if (screen[(int)fPosY * nScreenWidth + (int)fPosX] == 11146 or screen[((int)fPosY + 1) * nScreenWidth + (int)fPosX] == 11146 or screen[((int)fPosY - 1) * nScreenWidth + (int)fPosX] == 11146 or
			screen[(int)fPosY * nScreenWidth + (int)fPosX] == '~' or screen[((int)fPosY + 1) * nScreenWidth + (int)fPosX] == '~' or screen[((int)fPosY - 1) * nScreenWidth + (int)fPosX] == '~') {
			return 0;
		}
		

		//drawing player if its in bounds
		if (fPosY < nScreenHeight and fPosY >= 0 and fPosX < nScreenWidth and fPosX >= 0)
		{
			draw(nScreenWidth, screen);
			
		}
		

		return 1;
	}

	void draw(int nScreenWidth, wchar_t* screen) {
		screen[((int)fPosY - 1) * nScreenWidth + (int)fPosX] = 11170;
		screen[((int)fPosY + 1) * nScreenWidth + (int)fPosX] = 11168;
		screen[(int)fPosY * nScreenWidth + (int)fPosX] = 11194;
		screen[(int)fPosY * nScreenWidth + (int)fPosX + 1] = '>';
		screen[(int)fPosY * nScreenWidth + (int)fPosX - 1] = '=';
		screen[(int)fPosY * nScreenWidth + (int)fPosX + 2] = 8674;

	}

	void drawGui(int nScreenWidth, wchar_t* screen)
	{
		logScreen(screen, nScreenWidth, string(nBulletSwarmCount - nBulletActive, '|'), (int)(nScreenWidth / 2) - (int)(nBulletSwarmCount / 2), 0);

	}
};

//Enemy type - 1 class (only moves forward)
class EnemyType1 : protected Entity
{

public:
	EnemyType1() {
		fPosX = 0;
		fPosY = 0;
		fSpeed = 0;

	}

	EnemyType1(float posx, float posy, float speed,float birthtime, float lifetime)
	{
		fPosX = posx; fPosY = posy;
		fSpeed = speed;
		fBirthTime = birthtime; fLifeTime = lifetime;

	}

	void set(float posx,float posy, float speed, float birthtime, float lifetime) {
		fPosX = posx;
		fPosY = posy;
		fSpeed = speed;
		fBirthTime = birthtime; fLifeTime = lifetime;
	}

	int update(float fElapsedTime, float fCurrentTime, int nScreenWidth, int nScreenHeight, wchar_t* screen)
	{
		
		if (fCurrentTime > fBirthTime and fCurrentTime < fBirthTime + fLifeTime) {
			//handle movement
			fPosX -= fSpeed * fElapsedTime;					//move the enemy left

			//checks if enemy is in bounds
			if (fPosX < 0 or fPosX >= nScreenWidth or fPosY < 0 or fPosY >= nScreenHeight) {
				return 0;
			}

			//checking if enemy is hit by a bullet
			if (screen[(int)fPosY * nScreenWidth + (int)fPosX] == '-') {
				return 0;
			}

			//drawing enemy if its in bounds
			draw(nScreenWidth, screen);
			return 1;
		
		} 
		if (fCurrentTime > fBirthTime + fLifeTime) {
			return 0; //mark the enemy as free
		}
		return 1;

		

	}

	void draw(int nScreenWidth, wchar_t* screen) {
		screen[(int)fPosY * nScreenWidth + (int)fPosX] = 11146;

	}
};

//Enemy type -2 class (can move forward and can shoot)
class EnemyType2 : private EnemyType1
{
	SwarmManager<Bullet> smBullets; //array of bullets
	float fPrevBulletSpawnTime; //time of the last bullet spawn
	Bullet* temp;
	int nBulletActive; //current amount of units on screen
	float fBulletCooldown; //cooldown of the bullets
	int nBulletSwarmCount;

public:
	EnemyType2() {
		EnemyType1();
	}

	void set(float posx, float posy, float speed, float birthtime, float lifetime, float bulletspeed, float bulletfreq, int maxbullet, float bulletcooldown, int bulletswarmcount,float bulletlifetime) {

		fPosX = posx; fPosY = posy;
		fSpeed = speed;
		fBirthTime = birthtime; fLifeTime = lifetime;
		fBulletLifeTime = bulletlifetime;
										//set the position and speed of the enemy
		fBulletSpeed = bulletspeed; //set the bullet speed
		fBulletFreq = bulletfreq; //set the bullet frequency
		nMaxBulletCount = maxbullet; //set the max bullet count

		fBulletSpeed = bulletspeed;

		nBulletActive = 0;
		fBulletCooldown = bulletcooldown;
		nBulletSwarmCount = bulletswarmcount;
		smBullets.set(maxbullet, &nBulletActive, bulletswarmcount, fBulletCooldown);

		fPrevBulletSpawnTime = 0; //reset the prev spawn time
	}

	int update(float fElapsedTime, float fCurrentTime, int nScreenWidth, int nScreenHeight, wchar_t* screen)
	{


		if (fCurrentTime > fBirthTime and fCurrentTime < fBirthTime + fLifeTime) {
			//handle movement
			fPosX -= fSpeed * fElapsedTime;					//move the enemy left
			
			if (fCurrentTime - fPrevBulletSpawnTime > fBulletFreq	) {
				logScreen(screen, nScreenWidth, "Enemy is shooting", 50, 20, 0, fCurrentTime, 1, fCurrentTime);
				temp = smBullets.add(); //add a bullet to the array
				if (temp) {
					temp->set(fPosX, fPosY, fCurrentTime, fBulletLifeTime, -fBulletSpeed, '~',&nBulletActive,fBulletCooldown); //set the bullet position and speed
					fPrevBulletSpawnTime = fCurrentTime; //reset the prev spawn time
				}
				
			}
			//update the bullets
			smBullets.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen); 

			//checks if enemy is in bounds
			if (fPosX < 0 or fPosX >= nScreenWidth or fPosY < 0 or fPosY >= nScreenHeight) {
				return 0;
			}

			//checking if enemy is hit by a bullet
			if (screen[(int)fPosY * nScreenWidth + (int)fPosX] == '-') {
				return 0;
			}

			//drawing enemy if its in bounds
			draw(nScreenWidth, screen);
			return 1;

		}
		if (fCurrentTime > fBirthTime + fLifeTime) {
			return 0; //mark the enemy as free
		}
		return 1;


	}

};


int main() {

	bool config = 1;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns, rows;
	char choice;

	while (config) {

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

		printf("columns: %d\n", columns);
		printf("rows: %d\n", rows);
		
		cout << "if you want to resize press y else press n \n you can't resize after you press n";
		cin >> choice;
		if (choice == 'n') {
			config = 0;
		}

	}
	nScreenHeight = rows;
	nScreenWidth = columns;

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//initialises the time variables
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();
	wchar_t c = 0;
	float fElapsedTime = 0;
	float fCurrentTime = 0;

	//initialises the player variable
	Player player(fPlayerX, fPlayerY, fSpeed*3, fBulletFreq, fBulletSpeed, nMaxBulletCount,fBulletLifeTime,1,5);

	//initialises the enemy variable
	
	SwarmManager<EnemyType1> smEnemiesT1(20); //create the array of enemies
	smEnemiesT1.addMultiple(20, [](EnemyType1* e, int i)
		{
			e->set(nScreenWidth-1, i * 2 + 1, 60,8+ 0.1*i , 100); //set the enemy position and speed
		});
	SwarmManager<EnemyType1> smEnemiesT12(20); //create the array of enemies
	smEnemiesT12.addMultiple(20, [](EnemyType1* e, int i)
		{
			e->set(nScreenWidth - 1, i * 2, 60, 8+4-0.1 * i, 100); //set the enemy position and speed
		});
	SwarmManager<EnemyType1> smEnemiesT13(20); //create the array of enemies
	smEnemiesT13.addMultiple(20, [](EnemyType1* e, int i)
		{
			e->set(nScreenWidth - 1, i * 2 + 1, 60, 8+5+ 0.1 * i, 100); //set the enemy position and speed
		});
	//EnemyType2 e;
	//e.set(nScreenWidth-1, 20, 30, 0, 100, 40, 0.3f, 6,2,3,3); //set the enemy position and speed

	SwarmManager<EnemyType2> smEnemiesT21(20); //create the array of enemies
	smEnemiesT21.addMultiple(20, [](EnemyType2* e, int i)
		{
			e->set(nScreenWidth - 1, i * 2, 30, 8+5+2 + 0.1 * i, 100, 50, 0.3f, 6, 2, 3, 3); //set the enemy position and speed
		}); 
	SwarmManager<EnemyType2> smEnemiesT22(20); //create the array of enemies
	smEnemiesT22.addMultiple(20, [](EnemyType2* e, int i)
		{
			e->set(nScreenWidth - 1, i * 2+1, 30, 8 + 10 + 2 - 0.1 * i, 100, 50, 0.3f, 6, 2, 3, 3); //set the enemy position and speed
		});
	SwarmManager<EnemyType2> smEnemiesT23(20); //create the array of enemies
	smEnemiesT23.addMultiple(20, [](EnemyType2* e, int i)
		{
			e->set(nScreenWidth - 1, i * 2, 30, 8 + 12 + 2 + 0.1 * i, 100, 50, 0.3f, 6, 2, 3, 3); //set the enemy position and speed
		});

	int alive = 1;
	float fDeathTime = 0;
	//game loop
	while (1)
	{
		//gets the elapsed time
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		//declares the time variables
		fElapsedTime = elapsedTime.count();
		fCurrentTime +=fElapsedTime;

		//clears the screen
		ClearScreenDrawBg(nScreenWidth, nScreenHeight, screen);

		if (alive) {
			logScreen(screen, nScreenWidth, "You are the lone suvivor of your fleet", (int)(nScreenWidth/2.4), 20, 0, 0, 4, fCurrentTime);
			logScreen(screen, nScreenWidth, "But you are not alone.", (int)(nScreenWidth / 2.4), 20, 0, 5, 4, fCurrentTime);

			player.drawGui(nScreenWidth, screen); //draw the player gui
			player.updateBullets(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen); //update the bullets

			//updates enemies
			smEnemiesT1.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);
			smEnemiesT12.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);
			smEnemiesT13.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);
			smEnemiesT21.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);
			smEnemiesT22.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);
			smEnemiesT23.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);

			//updates player
			alive = player.update(fElapsedTime, fCurrentTime, nScreenWidth, nScreenHeight, screen);



			fDeathTime = fCurrentTime;
		}
		else {
			logScreen(screen, nScreenWidth, "You failed to survive.", (int)(nScreenWidth / 2.4), 20, 0, fDeathTime, fDeathTime + 10, fCurrentTime);
		}

		nFakeUnitAlive = -1000;

		// Display Frame
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

}