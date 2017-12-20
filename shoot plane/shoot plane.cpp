// a very simple test
// last modify still have lots of bugs. 
// name: shoot plane(da fei ji)

#include "stdafx.h"
/*ʹ��printf��ʹ��cout<<Ҫ��=_=*/
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
# define row 21  //��
# define row_ 123 
# define col 31  //��
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


//����Ƿ�Խ��
bool is_overstep(int x,int y)
{
	if(x>(row-1) || x<0 || y>(col-2) || y<0)
	{
		return true;
	}

	return false;
}

//�ӵ�
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
			//���л��ӵ�
			if(x<=20)
			{
				background[x][y] = '.';
			}
		}

	}

	//�ӵ����У���==��
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

	//�ӵ��Ƿ���Ҫ���ڰ�
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
	//position ׼ȷ��˵�Ƿɻ����������� +=+ �Ǹ�=�ŵ�����
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

		//����ԭʼ����
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

		//����Ƿ�Խ��
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

	//���ӵ�
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

	//ɾ���ӵ�---�ǳ��򵥵Ļ��ջ���(=_=!)
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

	//�ƶ������ӵ�
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

	//��֮ǰ�ķɻ�ɾ��
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
		//��ɾ��
		Del_Plane(buffer);

		//�ٻ�+-+
		buffer[x][y]   = '-';
		buffer[x][y-1] = '*';
		buffer[x][y+1] = '*';
	}

	//��ʼ��
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

	//����
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
	//Ϊ��ʡ�� ������Ϊpublic��
	public:

		int x;
		int y; // position
		
		vector<AttackPlane> ap;
		vector<bullet> vt;
		//�Ƿ�����
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
				//��λ��Ϊ�� ������
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
		//��Ϊ���� һ�����ӵ�����ײ һ���Ƿɻ�����ײ 
		//�ֿ�д�����Ժ���������д--��Ȼ���ʲ���̫��
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
					//��������ײ��Ϸ����
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
				//����С�ڵ���1 
				if(sqrt(float((abs((*it).x-x)*abs((*it).x-x)+abs((*it).y-y)*abs((*it).y-y))<=1)))
				{
					it = ap.erase(it);
					//��������ײ��Ϸ����
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

		//���ɻ�
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

//�̶�
void SetPosition(int x,int y)
{
	COORD pos = {x,y};	
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	::SetConsoleCursorPosition(hOut,pos);
}

void Init_Game()
{
	MessageBoxA(NULL,"ASDW ���Ʒ��� \n�ո������!\nBug �޶�\n����Ԥ����","\'��ܰ\'��ʾ=_=",MB_OK);
}

//��Ϸ����
void Game_Over()
{
	KillTimer(NULL,1);
	//���
	::Zero_Background(background);
	Error("��Ϸ������");
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

	//�����߳�
	HANDLE move_bullet = CreateThread(
						NULL,
						0,
						move,  //�̺߳���
						p,
						0, // ����������߳�
						NULL);

	CloseHandle(move_bullet);


	timeSetEvent(300,1,(LPTIMECALLBACK)TimerProc,MAKELPARAM(pa,p),TIME_PERIODIC);

	//pa->DrawPlane();


	while(game_loop)
	{
		//��������
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

