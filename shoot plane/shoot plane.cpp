// a very simple test
// last modify still have lots of bugs. 
// name: shoot plane(da fei ji)

#include "stdafx.h"
/*使用printf比使用cout<<要快=_=*/
////////////////////////
#include <stdio.h>
///////////////////////
#include <cmath>
#include <windows.h>
#include <vector>
using namespace std;

# pragma comment(lib,"winmm.lib")


bool game_loop = true;
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


//检查是否越界
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

	void Draw_bullet(int x ,int y,bool is_attacker=false)
	{
		if(!is_attacker)
		{
			if(x>=1)
			{
				background[x][y] = '|';
			}
		}
		else
		{
			//画敌机子弹
			if(x<=20)
			{
				background[x][y] = '.';
			}
		}

	}

	//子弹飞行（大汗==）
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
			if(x<=20)
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

	//子弹是否还需要存在啊
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
	//position 准确的说是飞机的中心坐标 +=+ 那个=号的坐标
	int x;
	int y;
	bullet * p_bt;
	int bullet_count;
	bool isdead;

	//bullet
	vector<bullet> v_bt;

	//default 
	plane(int x=20,int y=15)
	{
		this->x = x;
		this->y = y;
		bullet_count = 0;
		p_bt = NULL;
		isdead = false;
	}

	//
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
			//init some bullet date
	
			pbu->x = x-1;
			pbu->y = y;
			
			//insert into vector.
/////////////////////////////////////////////
			::EnterCriticalSection(&cs);
			this->v_bt.push_back(*pbu);
			LeaveCriticalSection(&cs);
/////////////////////////////////////////////
			
			free(pbu);
		}

		return true;
	}

	//画子弹
	void Draw_bullet(void)
	{
		vector<bullet>::iterator it;

		//if(!v_bt.empty())
///////////////////////////////////////////
		EnterCriticalSection(&cs);
		for(it=v_bt.begin();it!=v_bt.end();it++)
		{
		
			(*it).Draw_bullet((*it).x,(*it).y);
		
		}
		LeaveCriticalSection(&cs);
/////////////////////////////////////////
	}

	//删除子弹---非常简单的回收机制(=_=!)
	void del_bullet(void)
	{
		vector<bullet>::iterator it;
		bool ishav = false;

/////////////////////////////////////////////
		EnterCriticalSection(&cs);
		for(it=v_bt.begin();it!=v_bt.end();)
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
		LeaveCriticalSection(&cs);
////////////////////////////////////////////
	}

	//移动所有子弹
	void move_bullet(void)
	{
		vector<bullet>::iterator it;

		//if(!v_bt.empty())
/////////////////////////////////////////////////
		EnterCriticalSection(&cs);
		for(it=v_bt.begin();it!=v_bt.end();it++)
		{
			if((*it).move());
		}
		LeaveCriticalSection(&cs);
////////////////////////////////////////////////
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

	//初始化
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

	//析构
	~plane()
	{
		x = 0;
		y = 0;

		if(p_bt!=NULL)
		free(p_bt);
	}


};

//Attack plane class
class AttackPlane
{
	//为了省事 都声明为public的
	public:

		int x;
		int y; // position
		
		vector<AttackPlane> ap;
		vector<bullet> vt;
		//是否死亡
		bool isdead;

		//init
		AttackPlane()
		{
			isdead = false;
			x = 1;
		}

		//Create a plane
		bool Create()
		{
			AttackPlane * p = new AttackPlane();
			int sign = 0;
			for(int i =0;i<30;i++)
			{
				//该位置为空 可以用
				if(background[1][i]==' ')
				{
					p->y = i;
					sign = 1;
					break;
				}
			}
			
			if(!sign)
			ap.push_back(*p);

			free(p);

			return sign;
		}

		//move
		void ap_move()
		{
			if(x<=20)
			{
				x++;
			}

		}

		//shoot
		void shoot()
		{
			if(x==20)
			{
				return;
			}

			bullet * bt = new bullet();
			bt->x = x+1;
			bt->y = y;
			this->vt.push_back(*bt);
		}

		//"collision"
		//分为两个 一个是子弹的碰撞 一个是飞机的碰撞 
		//分开写便与以后的子类的重写--虽然几率不是太大
		//bullet collison
		bool bullet_collision(int x,int y)
		{
			
			//bullet collision
			vector<bullet>::iterator it;
			for(it=this->vt.begin();it!=vt.end();)
			{
				if((*it).x==x && (*it).y==y)
				{
					it = vt.erase(it);
					//发生了碰撞游戏结束
					return true;
				}
				else
				{
					it++;
				}
			}

			return false;

		}

		//plane collision
		bool plane_collision(int x,int y)
		{
			vector<AttackPlane>::iterator it;
			
			for(it=this->ap.begin();it!=ap.end();)
			{
				//距离小于等于1 
				if(sqrt(float((abs((*it).x-x)*abs((*it).x-x)+abs((*it).y-y)*abs((*it).y-y))<=1)))
				{
					it = ap.erase(it);
					//发生了碰撞游戏结束
					return true;
				}
				else
				{
					it++;
				}
			}

			return false;
		}

		//collision
		bool collision(int x,int y)
		{
			if(bullet_collision(x,y)||plane_collision(x,y))
			{
				return true;
			}
			
			return false;
		}

		//画飞机
		virtual void DrawPlane()
		{
			if(y<0 || y>=31)
				return;
			 
			if(!isdead)
			{
				//@-@
				/*vector<AttackPlane>::iterator it;
				if(!ap.empty())
				for(it = ap.begin();it!=ap.end();)
				{
					background[(*it).x][(*it).y]   = '-';
					background[(*it).x][(*it).y-1] = '@';
					background[(*it).x][(*it).y+1] = '@';

					it++;
				}*/

				background[x][y]   = '-';
				background[x][y-1] = '@';
				background[x][y+1] = '@';
			}
		}


		~AttackPlane()
		{
			x=0;
			y=0;
		}
		
	
};

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
			//printf("\n");
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
	MessageBoxA(NULL,"ASDW 控制方向 \n空格键开火!\nBug 巨多\n高能预警！","\'温馨\'提示=_=",MB_OK);
}

//游戏结束
void Game_Over()
{
	KillTimer(NULL,1);
	//清空
	::Zero_Background(background);
	Error("游戏结束！");
	system("pause");

}

////////////////////////////////////////////move
DWORD WINAPI move(LPVOID lpParam)
{
	plane * p = (plane *)lpParam;

	while(1)
	{
	 	Sleep(300);

	}
}

void CALLBACK TimerProc(UINT uTimerID,UINT uMsg,DWORD_PTR dwUser,DWORD_PTR dw1,DWORD_PTR dw2)
{
	
	auto * p = (plane *)LOWORD(dwUser);
	int a;


	/*

	p->move_bullet();
	p->del_bullet();

	pa->Create();
	if(pa->collision(p->x,p->y))
	{
		Game_Over();
	}
	pa->ap_move();*/
	

}

int _tmain(int argc, _TCHAR* argv[])
{

	plane * p = new plane();
	AttackPlane * pa = new AttackPlane();


	HWND cHwnd = GetConsoleWindow();
	if(cHwnd==NULL) 
		Error("can not find the window!");

	change_window_size(cHwnd,(SYSTEM_WIDTH-CONSOLE_WIDTH)/2,(SYSTEM_HEIGHT-CONSOLE_HEIGHT)/2,1024,768);
	system("cls");

	change_window_size(cHwnd,(SYSTEM_WIDTH-CONSOLE_WIDTH)/2,(SYSTEM_HEIGHT-CONSOLE_HEIGHT)/2,290,400);//21*31
	SetConsoleTitle(L"shoot plane!");
	Zero_Background(background);

	p->Init_Plane(background);
	Draw_Background(background);

	//创建线程
	HANDLE move_bullet = CreateThread(
						NULL,
						0,
						move,  //线程函数
						p,
						0, // 立即激活该线程
						NULL);

	CloseHandle(move_bullet);


	timeSetEvent(300,1,(LPTIMECALLBACK)TimerProc,MAKELPARAM(pa,p),TIME_PERIODIC);

	//pa->DrawPlane();


	while(game_loop)
	{
		//函数内容
		if(KEYDOWN(KEY_A))
		{
			p->move_plane(KEY_A);
			p->Update_Plane();
		}
		else if(KEYDOWN(KEY_D))
		{
			p->move_plane(KEY_D);
			p->Update_Plane();
		}
		else if(KEYDOWN(KEY_S))
		{
			p->move_plane(KEY_S);
			p->Update_Plane();
		}
		else if(KEYDOWN(KEY_W))
		{
			p->move_plane(KEY_W);
			p->Update_Plane();
		}
		else if(KEYDOWN(VK_SPACE))
		{
			p->shoot();
		}
			
		


		Sleep(50);
		system("cls");
		Draw_Background(background);
		SetPosition(500,500);
	}


	system("pause");
	return 0;
}

