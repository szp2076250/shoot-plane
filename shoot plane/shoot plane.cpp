// a very simple test by pSong
// last modify still have lots of bugs. 
// name: shoot plane(da fei ji)

#include "stdafx.h"

////////////////////////
#include <stdio.h>
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

CRITICAL_SECTION cs;

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

//是否越界
bool is_overstep(int x,int y)
{
	if(x>(row-1) || x<0 || y>(col-2) || y<0)
	{
		return true;
	}

	return false;
}
//子弹
class bullet
{
public:
	int x;
	int y;
	void Draw_bullet(int x, int y, bool is_attacker = false)
	{
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
		if(!is_attacker)
		{
			if(x>=1)
			{
				background[x][y] = ' ';
				x--;
				return true;
			}
			else
			{
				background[x][y] = ' ';
			}
		}
		else
		{
			//attacker
			if(x<=30)
			{
				background[x][y] = ' ';
				x++;
				return true;
			}
			else
			{
				background[x][y] = ' ';
			}
		}
		return false;
	}
	//delete
	bool isdead()
	{
		if(y<=31 && y>=1)
		{
			return false;
		}
		background[x][y] = ' ';
		return true;
	}
	void Remove()
	{
		background[x][y] = ' ';
	}
};

class plane
{
public:
	//position 中心坐标 +=+ 那个=号的坐标
	int x;
	int y;
	bullet * p_bt;
	int bullet_count;
	bool isdead;

	//LOCK
	CriticalLock * m_plock;
	//bullet
	vector<bullet> v_bt;


	plane(int x=20,int y=15)
	{
		this->x = x;
		this->y = y;
		bullet_count = 0;
		p_bt = NULL;
		isdead = false;
		//new Lock
		m_plock = new CriticalLock();
	}

	~plane() { x = y = 0; if (!p_bt)free(p_bt); }

	bool move_plane(int director)
	{
		//保留原始坐标
		int old_x = x;
		int old_y = y;
		int sign = 0;
		switch(director)
		{
			case KEY_A:
				if(y-1>=1)
				{
					y--;
					sign = 1;
				}
			break;
			case KEY_D:
				if(y+1<=31)
				{
					y++;
					sign = 1;
				}
			break;
			case KEY_W:
				if(x-1>=1)
				{
					x--;
					sign = 1;
				}
			break;
			case KEY_S:
				if(x+1<=21)
				{
					x++;
					sign = 1;
				}
			break;
		}

		//检查是否越界
		if(is_overstep(this->x,this->y))
		{
			this->x = old_x;
			this->y = old_y;
			old_x = old_y = 0;
			return false;
		}
		else
		{
		//delete 
		 this->Del_Plane(background,old_x,old_y);
		 old_x = old_y = 0;
		}


		return sign;
	}

	bool shoot(bool is_attacker=false)
	{
		
		if(!is_attacker)
		{
			if((this->y)-1<1)
				return false;

			//creat a bullet
			this->bullet_count++;
			bullet * pbu = new bullet();
	
	
			pbu->x = x-1;
			pbu->y = y;
			
			//insert into vector.
/////////////////////////////////////////////
			m_plock->Lock();;
			this->v_bt.push_back(*pbu);
			m_plock->Unlock();;
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
			(*it).Draw_bullet((*it).x,(*it).y);
		}
	}

	//del bullet
	void del_bullet(void)
	{
/////////////////////////////////////////////
		m_plock->Lock();
		for(auto it=v_bt.begin();it!=v_bt.end();)
		{
			if((*it).x == 0)
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

	//移动所有子弹
	void move_bullet(void)
	{
		//Not need LOCK
		for(auto it=v_bt.begin();it!=v_bt.end();it++)
		{
			if((*it).move());
		}

	}

	//把之前的飞机删了
	void Del_Plane(char (&buffer)[row][col])
	{
		buffer[x][y]   = ' ';
		buffer[x][y-1] = ' ';
		buffer[x][y+1] = ' ';
	}

	//version:2 not safe!
	void Del_Plane(char (&buffer)[row][col],int a,int b)
	{
		buffer[a][b]   = ' ';
		buffer[a][b-1] = ' ';
		buffer[a][b+1] = ' ';
	}

	//Draw Plane
	void DrawPlane(char  (&buffer)[row][col],int x,int y)
	{
		//先删除
		Del_Plane(buffer);

		//再画+-+
		buffer[x][y]   = '-';
		buffer[x][y-1] = '*';
		buffer[x][y+1] = '*';
	}
	//init
	void Init_Plane(char (&buffer)[row][col])
	{
		buffer[x][y]   = '-';
		buffer[x][y-1] = '*';
		buffer[x][y+1] = '*';
	}
	//Update 
	void Update_Plane()
	{
		this->DrawPlane(background,this->x,this->y);
	}

};

class Object
{
private:
public:
	int m_x; int m_y;
	virtual void init() = 0;
	virtual void update() = 0;
	virtual void draw(char(&buffer)[row][col]) = 0;
};

class AttackPlane : public Object
{
private:
	bullet * m_pbullet;
	bool dead;
	bool bullet_dead;
public:
	bool getState() { return dead; }
	bool CanGeneral(char(&buffer)[row][col], int x, int y)
	{
		auto kong = buffer[0][0];
		if(buffer[x][y]!= kong || buffer[x][y+1] != kong || buffer[x][y-1] != kong)
			return false;
		m_x = x; m_y = y;
		return true;
	}
	void init()
	{
		//-~-
		dead = false;
		bullet_dead = false;
		m_pbullet = new bullet();
		//find
		for (auto i = 1; i < 30; i++)
		{
			if (CanGeneral(background, 2, i))
			{
				return;
			}
		}
		//NO Attack plane can Construct 
		//视为 dead
		dead = true;
	}
	void update()
	{
		
		if (dead) return;

		if (!bullet_dead && m_pbullet->isdead())
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
		if (m_x> 30 || m_x< 0 || m_y<0 || m_y>20)
		{
			dead = true;
			return;
		}
		m_x++;
	}
	void draw(char(&buffer)[row][col])
	{
		if (dead) return;
		auto Clear = [&](char(&buffer)[row][col])
		{
			if (m_x - 1 < 0) return;
			buffer[m_x-1][m_y] = ' ';
			buffer[m_x-1][m_y + 1] = ' ';
			buffer[m_x-1][m_y - 1] = ' ';
		};
		//clear old

		Clear(background);
		//draw plane
		buffer[m_x][m_y] = '~';
		buffer[m_x][m_y-1] = '-';
		buffer[m_x][m_y+1] = '-';
		//draw bullet
		if(!bullet_dead)
		m_pbullet->Draw_bullet(m_x+1,m_y,true);
	}
	AttackPlane() { init(); }
	~AttackPlane() { delete m_pbullet; }
};



////////////move
DWORD WINAPI move(LPVOID lpParam)
{
	AttackPlane * ap = (AttackPlane *)lpParam;
	while (1)
	{
		Sleep(300);
		ap->draw(background);
		ap->update();
	}
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

void CleanScreen() { system("cls"); }

// play error message and end the gameloop
void Error(const char * _str)
{
	game_loop = false;
	system("cls");
	system("COLOR 4e");
	printf("---------------Error--------------\n");
	char buf[50];
	sprintf(buf,"%s\n",_str);
	printf(buf);
	printf("----------------------------------\n");
	getchar();
}

// change size
void change_window_size(HWND handle_console,int x,int y,int width,int height)
{
	MoveWindow(handle_console,x,y,width,height,true);
}

// draw background x replace row y replace col
void Draw_Background(char (&buffer)[row][col])
{
	for(int i=0;i<row;i++)
	{
		for(int j=0;j<col;j++)
		{
			printf("%c",buffer[i][j]);
		}
		printf("\n");
	}
}

void Zero_Background(char (&buffer)[row][col])
{
	for(int i = 0;i<row;i++)
	{
		for(int j = 0;j<col;j++)
		{
			buffer[row][col] = ' ';
		}
	}
}

//固定
void SetPosition(int x,int y)
{
	COORD pos = {x,y};	
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	::SetConsoleCursorPosition(hOut,pos);
}

void Init_Game()
{
	Zero_Background(background);
	//test attack plane
	AttackPlane * ap = new AttackPlane();
	//MessageBoxA(NULL,"ASDW 控制方向 \n空格键开火!\nBug 巨多\n高能预警！","\'温馨\'提示=_=",MB_OK);
	//Create a Actor
	actor = new plane();
	game_loop = true;
	HWND cHwnd = GetConsoleWindow();
	if (cHwnd == NULL)
		Error("can not find the window!");

	Manager::CleanScreen();
	Manager::change_window_size(cHwnd, (SYSTEM_WIDTH - CONSOLE_WIDTH) / 2, (SYSTEM_HEIGHT - CONSOLE_HEIGHT) / 2, 290, 400);//21*31
	SetConsoleTitle(L"shoot plane!");
	Manager::Zero_Background(background);
	Manager::Draw_Background(background);

	actor->Init_Plane(background);
	//Thread Move
	HANDLE move_bullet = CreateThread(NULL, 0, move, ap, 0, NULL);
	CloseHandle(move_bullet);
	timeSetEvent(300, 1, (LPTIMECALLBACK)TimerProc, (DWORD_PTR)actor, TIME_PERIODIC);
	

}

void Game_Loop()
{
	if (!actor) game_loop = false;
	while (game_loop)
	{
		if (KEYDOWN(KEY_A))
		{
			actor->move_plane(KEY_A);
			actor->Update_Plane();
		}
		else if (KEYDOWN(KEY_D))
		{
			actor->move_plane(KEY_D);
			actor->Update_Plane();
		}
		else if (KEYDOWN(KEY_S))
		{
			actor->move_plane(KEY_S);
			actor->Update_Plane();
		}
		else if (KEYDOWN(KEY_W))
		{
			actor->move_plane(KEY_W);
			actor->Update_Plane();
		}
		else if (KEYDOWN(VK_SPACE))
		{
			actor->shoot();
		}
		Sleep(50);
		Manager::CleanScreen();
		Manager::Draw_Background(background);
		Manager::SetPosition(500, 500);
	}


}

//游戏结束
void Game_Over()
{
	KillTimer(NULL,1);
	//清空
	Zero_Background(background);
	Error("游戏结束！");
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

