// a very simple test by pSong
// last modify still have lots of bugs. 
// name: shoot plane(da fei ji)

#include "stdafx.h"
////////////////////////
#include <stdio.h>
#include <iostream>
///////////////////////
#include <cmath>
#include <windows.h>
#include <vector>
using namespace std;
# pragma comment(lib,"winmm.lib")

int  score     = 0;
# define row 21  //行
# define row_ 123 
# define col 31  //列
char background[row][col];

# define SYSTEM_WIDTH   GetSystemMetrics(SM_CXSCREEN)
# define SYSTEM_HEIGHT  GetSystemMetrics(SM_CYSCREEN)
# define CONSOLE_WIDTH  1024
# define CONSOLE_HEIGHT 768


# define KEYDOWN(vk_key) ((GetAsyncKeyState(vk_key)&0x8000)?1:0)
# define KEYUP(vk_key)   ((GetAsyncKeyState(vk_key)&0x8000)?0:1)

# define KEY_A 65
# define KEY_D 68
# define KEY_S 83
# define KEY_W 87
# define SAFE_DELETE(p) if(p!=NULL) delete p;

//是否越界
bool is_overstep(int x, int y)
{
	if (x > (row - 1) || x<0 || y>(col - 2) || y < 0)
	{
		return true;
	}
	return false;
}


//Lock
class CriticalLock
{
private:
	CRITICAL_SECTION mLock;
public:
	void Lock() { EnterCriticalSection(&mLock); }
	void Unlock() { LeaveCriticalSection(&mLock); }
	CriticalLock() { InitializeCriticalSection(&mLock); }
	~CriticalLock() { DeleteCriticalSection(&mLock); }
};

class Object
{
private:
public:
	int m_x; int m_y;
	bool ObjectDead;
	virtual void init() {} 
	virtual void update() {}
	virtual void draw(char(&buffer)[row][col]) {}
	virtual ~Object() {}
};

class ObjectManager
{
private:
	vector<Object *>  m_vo;
	CriticalLock * mpObjectLock;
	ObjectManager() { mpObjectLock = new CriticalLock(); }
public:
	static ObjectManager * pInstance;
	static ObjectManager * getInstance();
	// [out] nearest
	double Calc_near_Lu(int x, int y,Object * pnearest,bool & state)
	{
		auto tempState = false;
		auto PingFang = [&](double x) {return x*x; };
		Object * pob = NULL;
		//无穷远
		double near_most = 999999999999;
		//////////////////////////////////////////////////////////////////////////LOCK  关于m_vo的操作
		mpObjectLock->Lock();
		if (m_vo.size() == 0)[&] {tempState = false; return near_most; };
		for (auto it = m_vo.begin(); it != m_vo.end(); it++)
		{
			//no sqrt
			auto distance = PingFang(abs((*it)->m_x - x)) + PingFang(abs((*it)->m_y - y));
			if (distance < near_most)
			{
				near_most = distance;
				pob = (*it);
			}
		}
		//dead put here
		if (near_most<=1)
		{
			tempState = true;
			pob->ObjectDead = true;
		}
		mpObjectLock->Unlock();
		//////////////////////////////////////////////////////////////////////////UNLOCK
		pnearest = pob;
		state = tempState;
		return near_most;
	}
	void AddObject(Object * pObject)
	{
		mpObjectLock->Lock();
		m_vo.push_back(pObject);
		mpObjectLock->Unlock();
	}
	void RemoveObject()
	{
		mpObjectLock->Lock();
			for (auto it = m_vo.begin(); it != m_vo.end(); it++)
				if ((*it)->ObjectDead)
				{ 
					delete *it;
					it = m_vo.erase(it);
				}
					
		mpObjectLock->Unlock();
	}
	void DrawObject()
	{
		mpObjectLock->Lock();
			for (auto it = m_vo.begin(); it != m_vo.end(); it++)
				(*it)->draw(background);
		mpObjectLock->Unlock();
	}
	void UpdateObject()
	{
		mpObjectLock->Lock();
			for (auto it = m_vo.begin(); it != m_vo.end(); it++)
				(*it)->update();
		mpObjectLock->Unlock();
	}
	~ObjectManager() { SAFE_DELETE(mpObjectLock); }
};
ObjectManager * ObjectManager::pInstance = NULL;

ObjectManager * ObjectManager::getInstance()
{
	if (pInstance == NULL)
		pInstance = new ObjectManager();
	return pInstance;
}

class bullet_base :public Object
{
public:
	char bullet_kind;
	void SetState(bool state) { ObjectDead = state; }
	bool GetState(void) { return ObjectDead; }
	void Clear() { background[m_x][m_y] = ' '; }
	virtual void draw() {};
	virtual void init() { bullet_kind = '.'; };
	virtual void update() {};
	bullet_base() { ObjectDead = false; }
	~bullet_base() { Clear(); }
};

//子弹
class bullet : public Object
{
public:
	void SetState(bool state) { ObjectDead = state; }
	bool GetState(void) { return ObjectDead; }

	bullet() { ObjectDead = false; }
	~bullet() {}
	void Draw_bullet(int x, int y, bool is_attacker = false)
	{
		if (ObjectDead) return;
		if (!is_attacker)
		{
			if (x >= 1)
			{
				background[x][y] = '|';
			}
		}
		else
		{
			if (x <= 20)
			{
				background[x][y] = '.';
			}
		}
	}
	//move bullet
	bool move(bool is_attacker = false)
	{
		if (ObjectDead) return false;
		if(!is_attacker)
		{
			if(m_x>=1)
			{
				background[m_x][m_y] = ' ';
				m_x--;
				return true;
			}
			else
			{
				background[m_x][m_y] = ' ';
			}
		}
		else
		{
			//attacker
			if(m_x<=30)
			{
				background[m_x][m_y] = ' ';
				m_x++;
				return true;
			}
			else
			{
				background[m_x][m_y] = ' ';
			}
		}

		if(is_overstep(m_x, m_y)) ObjectDead = true;
		return false;
	}

	void Remove()
	{
		background[m_x][m_y] = ' ';
	}
};

class plane:public Object
{
public:
	//position 中心坐标 +=+ 那个=号的坐标
	bullet * p_bt;
	int bullet_count;

	//LOCK
	CriticalLock * m_plock;
	//bullet
	vector<bullet> v_bt;
	plane(int x=20,int y=15)
	{
		m_x = x;
		m_y = y;
		bullet_count = 0;
		p_bt = NULL;
		ObjectDead = false;
		//new Lock
		m_plock = new CriticalLock();
	}
	~plane() { m_x = m_y = 0; if (!p_bt)free(p_bt); }
	void move_plane(int director)
	{
		//保留原始坐标
		int old_x = m_x;
		int old_y = m_y;
		switch(director)
		{
			case KEY_A:
				if(m_y-1>=1)  m_y--;
			break;
			case KEY_D:
				if(m_y+1<=31) m_y++;
			break;
			case KEY_W:
				if(m_x-1>=1)  m_x--;
			break;
			case KEY_S:
				if(m_x+1<=21) m_x++;
			break;
		}

		if(is_overstep(m_x, m_y))
		{
			m_x = old_x;
			m_y = old_y;
			old_x = old_y = 0;
			return ;
		}
		else
		{
		//delete 
		 this->Del_Plane(background,old_x,old_y);
		 old_x = old_y = 0;
		}
		return ;
	}

	bool shoot(bool is_attacker=false)
	{
		if(!is_attacker)
		{
			if(m_y-1<0)
				return false;
			//creat a bullet
			this->bullet_count++;
			bullet * pbu = new bullet();
			pbu->m_x = m_x -1;
			pbu->m_y = m_y;
			//insert into vector.
/////////////////////////////////////////////
			m_plock->Lock();
			this->v_bt.push_back(*pbu);
			m_plock->Unlock();
/////////////////////////////////////////////
			free(pbu);
		}
		return true;
	}

	//draw bullet
	void Draw_bullet(void)
	{
		for(auto it=v_bt.begin();it!=v_bt.end();it++)
		{
			(*it).Draw_bullet((*it).m_x,(*it).m_y);
		}
	}

	//del bullet
	void del_bullet(void)
	{
/////////////////////////////////////////////
		m_plock->Lock();
		for(auto it=v_bt.begin();it!=v_bt.end();)
		{
			if((*it).m_x == 0||(*it).GetState())
			{
				(*it).Remove();
				it = v_bt.erase(it);
			}
			else
			{
				it++;
			}
		}
		m_plock->Unlock();
////////////////////////////////////////////
	}

	void move_bullet(void)
	{
		//Not need LOCK
		for(auto it=v_bt.begin();it!=v_bt.end();it++)
		{
			if((*it).move());
			Object * p=NULL;
			bool state;
			auto distance = ObjectManager::getInstance()->Calc_near_Lu((*it).m_x, (*it).m_y, p, state);
			(*it).SetState(state);
		}
	}

	//clear
	void Del_Plane(char (&buffer)[row][col],int a,int b)
	{
		buffer[a][b]   = ' ';
		buffer[a][b-1] = ' ';
		buffer[a][b+1] = ' ';
	}

	//Draw Plane
	void draw(char  (&buffer)[row][col])
	{
		if (ObjectDead) return;

		//Del_Plane(buffer);
		buffer[m_x][m_y]   = '-';
		buffer[m_x][m_y-1] = '*';
		buffer[m_x][m_y+1] = '*';
	}

};

class AttackPlane : public Object
{
private:
	bullet * m_pbullet;
	bool bullet_dead;
	char * kind ="-~-";
	char kind1[4] = "-=-";
	char kind2[4] = "&=&";
	char kind3[4] = "%=%";
	char kind4[4] = "$-$";
	char kind5[4] = "-~-";
	char kind6[4] = "@-@";
public:
	bool getState() { return ObjectDead; }
	void setState(bool state) { ObjectDead = state;}
	void Remove(char(&buffer)[row][col], UINT x, UINT y)
	{
		if (x - 1 < 0) return;
		buffer[x][y] = ' ';
		buffer[x][y + 1] = ' ';
		buffer[x][y - 1] = ' ';
	}
	bool CanGeneral(char(&buffer)[row][col], int x, int y)
	{
		srand(GetTickCount());
		y = rand()%30;
		auto varis = rand() % 7;

		switch (varis)
		{
		case 1:
			kind = kind1;
			break;
		case 2:
			kind = kind2;
			break;
		case 3:
			kind = kind3;
			break;
		case 4:
			kind = kind4;
			break;
		case 5:
			kind = kind5;
			break;
		case 6:
			kind = kind6;
			break;
		default:
			break;
		}
	
		m_x = x; m_y = y;
		return true;
	}
	void init()
	{
		//-~-
		m_x = m_y = 0;
		ObjectDead = false;
		bullet_dead = false;
		m_pbullet = new bullet();
		m_pbullet->SetState(true);
		//find
		if (CanGeneral(background, 1, 0))
		{
			return;
		}
		//NO Attack plane can Construct 
		//视为 dead
		ObjectDead = true;
	}
	void update()
	{
		bullet_dead = m_pbullet->GetState();
		if (ObjectDead)
		{
			Remove(background,m_x-1,m_y);
			return;
		}
		if (!bullet_dead && m_pbullet->GetState())
		{
			bullet_dead = true;
			delete(m_pbullet);
			m_pbullet = NULL;
		}
		if (!bullet_dead)
		{
			m_pbullet->move(true);
		}
		//move plane
		auto temp = m_x;
		if (m_x> 30 || m_x< 0 || m_y<0 || m_y>30)
		{
			ObjectDead = true;
			return;
		}
		m_x++;
	}
	void draw(char(&buffer)[row][col])
	{
		if (ObjectDead)
		{
			Remove(background, m_x-1, m_y);
			return;
		}

		//clear old
		Remove(background,m_x-1,m_y);
		//draw plane
		buffer[m_x][m_y] = kind[1];
		buffer[m_x][m_y-1] = kind[0];
		buffer[m_x][m_y+1] = kind[2];
		//draw bullet
		if(!bullet_dead)
		m_pbullet->Draw_bullet(m_x+1,m_y,true);
	}
	AttackPlane() { init(); }
	~AttackPlane() {
		//clear
		Remove(background, m_x - 1, m_y);
		delete m_pbullet;
	}
};

////////////move
DWORD WINAPI move(LPVOID lpParam)
{
	AttackPlane * ap = (AttackPlane *)lpParam;
	while (1)
	{
		Sleep(300);
		Object * p = new AttackPlane();
		ObjectManager::getInstance()->AddObject(p);
		ObjectManager::getInstance()->RemoveObject();
		ObjectManager::getInstance()->DrawObject();
		ObjectManager::getInstance()->UpdateObject();
	}
}

DWORD WINAPI draw(LPVOID lpParam)
{
	plane * actor = (plane *)lpParam;
	
}


void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	auto * p = (plane *)dwUser;
	p->move_bullet();
	p->Draw_bullet();
	p->del_bullet();
}

namespace Manager {

	//Actor
	plane * actor = NULL;
	//condition
	bool game_loop = false;

	void Zero_Background(char(&buffer)[row][col])
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				buffer[row][col] = ' ';
			}
		}
}

	void CleanScreen() { Zero_Background(background); ("cls"); /*Manager::cls();*/ }

	// play error message and end the gameloop
	void Error(const char * _str)
	{
		game_loop = false;
		system("cls");
		system("COLOR 4e");
		printf("---------------Error--------------\n");
		char buf[50];
		sprintf_s(buf,"%s\n",_str);
		printf(buf);
		printf("----------------------------------\n");
		getchar();
	}

	// draw background x replace row y replace col
	void Draw_Background(char(&buffer)[row][col], HANDLE first_hwnd, HANDLE second_hwnd)
	{
		if (first_hwnd == NULL || second_hwnd == NULL) [] {Error("Buffer Error"); return; };
		//SetConsoleActiveScreenBuffer(second_hwnd);
		SetConsoleActiveScreenBuffer(first_hwnd);
		PCONSOLE_SCREEN_BUFFER_INFO info1=NULL, info2=NULL;
		GetConsoleScreenBufferInfo(first_hwnd, info1);
		GetConsoleScreenBufferInfo(first_hwnd, info2);
		CONSOLE_CURSOR_INFO cci;
		cci.dwSize = 1;
		cci.bVisible = FALSE;
		SetConsoleCursorInfo(first_hwnd,&cci);
		SetConsoleCursorInfo(second_hwnd, &cci);
		char buf[row*col];
		DWORD bytes = 0;
		CleanScreen();
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				if(background[i][j]!=0)
				{ 
				COORD cd = {j,i};
				SetConsoleCursorPosition(first_hwnd,cd);
				cout << buffer[i][j];
				}
			}
			cout << endl;
		}
		COORD cd = {0,0};
		ReadConsoleOutputCharacterA(first_hwnd,buf, row*col,cd,&bytes);
		WriteConsoleOutputCharacterA(second_hwnd,buf, row*col,cd,&bytes);
	}

	void Init_Game()
	{
		//MessageBoxA(NULL,"ASDW 控制方向 \n空格键开火!\nBug 巨多\n高能预警！","\'温馨\'提示=_=",MB_OK);
		//Create a Actor
		actor = new plane();
		game_loop = true;
		HWND cHwnd = GetConsoleWindow();
		if (cHwnd == NULL)
			Error("can not find the window!");
		SetConsoleTitle(L"shoot plane!");


		SMALL_RECT rc = {0,0,31,24};
		SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, &rc);
		COORD cd = { 0,0 };
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cd);

		Manager::CleanScreen();
		Manager::Zero_Background(background);

		//Thread Move
		HANDLE move_bullet = CreateThread(NULL, 0, move, NULL, 0, NULL);
		CloseHandle(move_bullet);
		timeSetEvent(300, 1, (LPTIMECALLBACK)TimerProc, (DWORD_PTR)actor, TIME_PERIODIC);
	}

	void Game_Loop()
	{
		auto secondBuf = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CONSOLE_TEXTMODE_BUFFER,
			NULL);

		auto drawThread = CreateThread(NULL, 0,draw,actor, 0, NULL);
		CloseHandle(drawThread);

		if (!actor) game_loop = false;
		while (game_loop)
		{
			while (1)
			{
				if (KEYDOWN(KEY_A))
				{
					actor->move_plane(KEY_A);
				}
				else if (KEYDOWN(KEY_D))
				{
					actor->move_plane(KEY_D);
				}
				else if (KEYDOWN(KEY_S))
				{
					actor->move_plane(KEY_S);
				}
				else if (KEYDOWN(KEY_W))
				{
					actor->move_plane(KEY_W);
				}
				else if (KEYDOWN(VK_SPACE))
				{
					actor->shoot();
				}
			}
			actor->draw(background);
			Manager::Draw_Background(background, GetStdHandle(STD_OUTPUT_HANDLE), secondBuf);
		}
	}

	void Game_Over()
	{
		KillTimer(NULL,1);
		//清空
		Zero_Background(background);
		Error("Game Over！"); 
		system("pause");
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	Manager::Init_Game();
	Manager::Game_Loop();
	Manager::Game_Over();
	system("STAR pause");
	return 0;
}

