//////////////////////////////////////////////////////////////
// C 표준 라이브러리 헤더 파일
//////////////////////////////////////////////////////////////
#include <stdio.h> //printf(), scanf()
#include <stdlib.h> //srand(), rand(), malloc()
#include <string.h> //문자열
#include <ctype.h> //문자
#include <time.h> //time(), clock()
#include <math.h> //수학함수
//////////////////////////////////////////////////////////////
// OS, Hardware 종속 라이브러리 헤더 파일 
//////////////////////////////////////////////////////////////
//#include <windows.h> //윈도우즈 API
//SetConsoleCursorPosition(), GetStdHandle(), Sleep()
//#include <conio.h> //getch(), kbhit()
//////////////////////////////////////////////////////////////
// Board의 LCD, Joystick, Button를 사용하기 위한 헤더 파일
//////////////////////////////////////////////////////////////
#include <stm32f4xx.h>
#include "GLCD.h"
#include "JOY.h"
#include "I2C.h"
//////////////////////////////////////////////////////////////
// 게임에 자주 사용하는 키보드 상수 
//////////////////////////////////////////////////////////////
#define ESC 27 //게임 탈출 
#define ENTER 13
#define SPACE 32 //총알, 점프 
#define LEFT 75 //224 다음에 75
#define RIGHT 77 //224 다음에 77
#define UP 72 //224 다음에 72
#define DOWN 80 //224 다음에 80
//////////////////////////////////////////////////////////////
#define winX 30  //창의 시작 위치
#define winY 2  //창의 시작 위치
#define winWidth 20 //창의 폭
#define winHeight 26 //창의 높이
#define FREE_DROP 0 //아래 자유 낙하
#define MOVE_DOWN 1 //아래 강제 이동
#define MOVE_LEFT 2 //좌측 이동
#define MOVE_RIGHT 3 //우측 이동
#define MOVE_DROP 4 //떨어뜨리기
#define ROTATION 5 //떨어뜨리기
//////////////////////////////////////////////////////////////
#define WAKEUP ((GPIOA -> IDR & (1 << 0))==1)
#define TAMPER ((GPIOC -> IDR & (1<<13))==0)
#define USER ((GPIOG -> IDR & (1<<15))==0)
#define TH 100
#define J_INITIAL -1
#define J_DOWN 0
#define J_UP 1
#define TAMPERBUT ((GPIOC->IDR & (1 << 13))==0)
static int joycenter;
static int mode;
//////////////////////////////////////////////////////////////
// Button 초기화 함수 
//////////////////////////////////////////////////////////////
void BUT_Init (void) {
  
	RCC->AHB1ENR |= (1UL << 0) | (1UL << 2) | (1UL << 6) ;

  GPIOA->MODER &= ~(3UL << 2* 0);
  GPIOC->MODER &= ~(3UL << 2*13);
  GPIOG->MODER &= ~(3UL << 2*15);
}
//////////////////////////////////////////////////////////////
// 함수 프로토타입 
//////////////////////////////////////////////////////////////
//void gotoXY(int x, int y); //콘솔 화면 특정 위치로 이동
void checkKey(void); //키보드 처리 담당 
void Display(void); //화면 표시 담당, 1초에 25프레임 
void Update(void); //게임 객체 상태 업데이트, 시뮬레이션 
void Start(void); //게임 초기 상태 설정
int IsCollision(void); //충돌 검사
void FixBrick(void); //블록 고정하기
void NewBrick(void); //새 블록 만들기
void BarCheck(void); //누적된 막대 확인 제거, 점수 상승 
int getch(void); // 입력을 처리
//////////////////////////////////////////////////////////////
// 게임 객체의 구조체 
//////////////////////////////////////////////////////////////
int brick_x, brick_y; //객체의 윈도우 안의 위치 
int brick_shape, brick_rotation; //객체의 모양, 회전
int win[winHeight][winWidth]; //창의 내용물 
int brick_action; //객체의 행동
int free_drop_delay = 20; //낙하 시간 간격 
int free_drop_count; //낙하 시간 카운트  
// 객체의 모양 7개, 회전 4개, y, x
char brick[7][4][4][4]={
// ㅗ 회전 0
	0,1,0,0,
	1,1,1,0,
	0,0,0,0,
	0,0,0,0,
// ㅏ 회전 1
	0,1,0,0,
	0,1,1,0,
	0,1,0,0,
	0,0,0,0,
// ㅜ 회전 2
	0,0,0,0,
	1,1,1,0,
	0,1,0,0,
	0,0,0,0,
// ㅓ 회전 3
	0,1,0,0,
	1,1,0,0,
	0,1,0,0,
	0,0,0,0,
//Z 회전0
	0,1,1,0,
	1,1,0,0,
	0,0,0,0,
	0,0,0,0,
//Z 회전1
	0,1,0,0,
	0,1,1,0,
	0,0,1,0,
	0,0,0,0,
//Z 회전2
	0,1,1,0,
	1,1,0,0,
	0,0,0,0,
	0,0,0,0,
//Z 회전3
	0,1,0,0,
	0,1,1,0,
	0,0,1,0,
	0,0,0,0,
//Z 회전0
	1,1,0,0,
	0,1,1,0,
	0,0,0,0,
	0,0,0,0,
//Z 회전1
	0,1,0,0,
	1,1,0,0,
	1,0,0,0,
	0,0,0,0,
//Z 회전2
	1,1,0,0,
	0,1,1,0,
	0,0,0,0,
	0,0,0,0,
//Z 회전3
	0,1,0,0,
	1,1,0,0,
	1,0,0,0,
	0,0,0,0,
//ㄱ회전0
	1,1,0,0,
	0,1,0,0,
	0,1,0,0,
	0,0,0,0,
//ㄱ회전1
	0,0,1,0,
	1,1,1,0,
	0,0,0,0,
	0,0,0,0,
//ㄱ회전2
	0,1,0,0,
	0,1,0,0,
	0,1,1,0,
	0,0,0,0,
//ㄱ회전3
	0,0,0,0,
	1,1,1,0,
	1,0,0,0,
	0,0,0,0,
//ㄴ회전0
	0,1,1,0,
	0,1,0,0,
	0,1,0,0,
	0,0,0,0,
//ㄴ회전1
	0,0,0,0,
	1,1,1,0,
	0,0,1,0,
	0,0,0,0,
//ㄴ회전2
	0,1,0,0,
	0,1,0,0,
	1,1,0,0,
	0,0,0,0,
//ㄴ회전3
	1,0,0,0,
	1,1,1,0,
	0,0,0,0,
	0,0,0,0,
//직선 회전0
	0,1,0,0,
	0,1,0,0,
	0,1,0,0,
	0,1,0,0,
//직선 회전1
	0,0,0,0,
	1,1,1,1,
	0,0,0,0,
	0,0,0,0,
//직선 회전2
	0,1,0,0,
	0,1,0,0,
	0,1,0,0,
	0,1,0,0,
//직선 회전3
	0,0,0,0,
	1,1,1,1,
	0,0,0,0,
	0,0,0,0,
//상자 회전0
	1,1,0,0,
	1,1,0,0,
	0,0,0,0,
	0,0,0,0,
//상자 회전1
	1,1,0,0,
	1,1,0,0,
	0,0,0,0,
	0,0,0,0,
//상자 회전2
	1,1,0,0,
	1,1,0,0,
	0,0,0,0,
	0,0,0,0,
//상자 회전3
	1,1,0,0,
	1,1,0,0,
	0,0,0,0,
	0,0,0,0
};
int GameOver = 0;
int GamePoint = 0;
//////////////////////////////////////////////////////////////
// 함수 정의 부분 
//////////////////////////////////////////////////////////////
int tetrismain()
{
	int i;
	char gameover[128];
	char finalpoint [128];
	
	
	BUT_Init();
	JOY_Init();
	GLCD_Init();
	GLCD_Clear(Black);
	GLCD_SetTextColor(White);
	GLCD_SetBackColor(Black);
	
	Start();  //게임 초기 상태 설정
	
	
	
	while(!GameOver)
{
  Display(); //화면에 현재 상태 그리기
  checkKey(); //키보드 입력 처리
  Update(); //게임 객체 상태 업데이트 
  //Sleep(40); //40ms 잠자기
	for(i=0 ; i<4000000 ; i++);
	OSTaskChangePrio(12,10);
 }
     
	

  GLCD_Clear(Black); 
 	sprintf(gameover, "Game Over!");
	GLCD_DisplayString(13, 34, 0, gameover);
 	sprintf(finalpoint, "Your final point is %d", GamePoint);
	GLCD_DisplayString(15, 28, 0, finalpoint);
 
return 0;
}
//////////////////////////////////////////////////////////////
/*void gotoXY(int x, int y, String src) //콘솔 화면 특정 위치로 이동
{
 COORD Pos = {x, y};
 SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}
*/
//////////////////////////////////////////////////////////////
void Start() //게임 초기 상태 설정
{
	int x, y;
	NewBrick(); //새 개체 만들기
	free_drop_count = free_drop_delay; //20 프레임에 1회 다운 
	//테트리스 윈도우 초기화
	for (x=0; x<winWidth; x++)
	{
		for(y=0; y<winHeight; y++)
		{
			if (x == 0 || x == winWidth-1 ||
				y == 0 || y == winHeight-1)
			{
				win[y][x]=2;
			} else
			{
				win[y][x]=0;
			}
		}
	}
}
//////////////////////////////////////////////////////////////
int IsCollision() //게임 객체 충돌 검사
{
 int x, y;
 for (y=0; y<4; y++)
 {
	for (x=0; x<4; x++)
	{
		if (brick[brick_shape][brick_rotation][y][x] ==1 )
		{
			if (win[brick_y+y][brick_x+x] != 0)
			{
				return 1; //충돌 있음
			}
		}	
	}
 }
 return 0; //충돌 없음
}
//////////////////////////////////////////////////////////////
void FixBrick() //게임 객체 고정
{
 int x, y;
 for (y=0; y<4; y++)
 {
	for (x=0; x<4; x++)
	{
		if (brick[brick_shape][brick_rotation][y][x] ==1 )
		{
			win[brick_y+y][brick_x+x] = 1;
		}	
	}
 }
}
//////////////////////////////////////////////////////////////
void NewBrick() //새로운 객체 만들기
{
	//srand(time(NULL)); //난수 발생 시작점 초기화 
	brick_x = winWidth/2; //객체의 x 위치
	brick_y = 1; //객체의 y 위치
	brick_shape = rand()%7; //모양 0 ~ 6 
	brick_rotation = 0; //회전 없음 
	brick_action = FREE_DROP;
}
//////////////////////////////////////////////////////////////
void BarCheck() //누적 블록 제거 점수 올리기
{
	int x, y, bar, i, j;
	for(y=1; y<winHeight-1; y++)
	{
		bar = 0;
		for (x=1; x<winWidth-1; x++)
		{
			bar += win[y][x];
		}
		if (bar == winWidth-2)
		{
			GamePoint++;
			if(GamePoint % 20 == 0) free_drop_delay--;
			if(free_drop_delay < 0) free_drop_delay = 0;
			for (i=y-1; i>0; i--)
			{
				for (j=1; j<winWidth-1; j++)
				{
					win[i+1][j] = win[i][j];
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////
void Display() //화면에 현재 상태 그리기
{
	int x, y;
	char info1[128], info2[128];
	char point[128];
 //창 그리기 
for (y=0; y<winHeight; y++)
 {
	//gotoXY(winX, winY+y);
	 for (x=0; x<winWidth; x++)
	 {
		if(win[y][x]==1) {
			//printf("■");
			GLCD_DisplayChar(winY+y, winX+x, 0, '@');
		}		
		else if(win[y][x]==2) {
			//printf("□");
			GLCD_DisplayChar(winY+y, winX+x, 0, '#');
		}
		else {
		  //printf("·");
			GLCD_DisplayChar(winY+y, winX+x, 0, '.');
		}
	 }
	//printf("\n");
	 GLCD_DisplayChar(winY+y, winX+x, 0, ' ');	 
	 }
 
 //블록 그리기
 for (y=0; y<4; y++)
 {
	for (x=0; x<4; x++)
	{
		if (brick[brick_shape][brick_rotation][y][x] ==1 )
		{
			 //gotoXY(winX + (brick_x + x)*2, winY + brick_y + y);
			 //printf("■");
			GLCD_DisplayChar(winY + brick_y + y, winX + brick_x + x, 0, '@');
		}
	}
}
 //점수 표시
	//gotoXY(30, 25);
	//printf("Point = %d",GamePoint);
	  sprintf(point, "Point = %d", GamePoint);
	GLCD_DisplayString(30, 27, 0, point);
}
//////////////////////////////////////////////////////////////
void Update() //게임 객체 상태 업데이트 
{
	if (brick_action == FREE_DROP) {
		free_drop_count--;
		if(free_drop_count < 0) 
		{
			free_drop_count = free_drop_delay; 
			brick_action = MOVE_DOWN;
		}	
	}
  switch (brick_action)
  {
   case MOVE_DROP: 
	   do{
		   brick_y++;
	   }while(!IsCollision());
	   brick_y--;
	   if (brick_y == 1) GameOver = 1;
		FixBrick(); //개체 고정
		BarCheck(); //누적 상태 확인 
		NewBrick(); //새 개체 만들기
		free_drop_count = free_drop_delay; //20 프레임에 1회 다운 
	  brick_action = FREE_DROP;
      break;
   case MOVE_LEFT: 
	   brick_x--;
	   if (IsCollision()) brick_x++;
	  brick_action = FREE_DROP;
      break;
   case MOVE_RIGHT: 
	   brick_x++;
	   if (IsCollision()) brick_x--;
	  brick_action = FREE_DROP;
      break;
   case MOVE_DOWN: 
	   brick_y++;
	   if (IsCollision())
	   {
		   brick_y--;
		   if (brick_y == 1) GameOver = 1;
			FixBrick(); //개체 고정
			BarCheck(); //누적 상태 확인 
			NewBrick(); //새 개체 만들기
			free_drop_count = free_drop_delay; //20 프레임에 1회 다운 
	   }
	  brick_action = FREE_DROP;
      break;
   case FREE_DROP: 
		free_drop_count--;
		if(free_drop_count < 0) 
		{
			free_drop_count = free_drop_delay; 
			brick_action = MOVE_DOWN;
		}
      break;
   case ROTATION: 
	  brick_rotation++;
	  if(brick_rotation > 3) brick_rotation = 0;
	   if (IsCollision())
	   {
		   brick_rotation--;
			if(brick_rotation < 0) brick_rotation = 3;
	   }
	  brick_action = FREE_DROP;
      break;
   default :
      break;
  }
}
//////////////////////////////////////////////////////////////
void checkKey() //키보드 처리 담당 
{
 int key;
	key=getch(); //입력이 된 값을 key 저장
	if (key == 224) //키보드 스캔 코드 224이면 기능키 
  {
   key=getch(); //다음 한 글자 추가 읽기
  }
  switch (key)
  {
   case ESC:
	   GameOver = 1;
      break;
   case ENTER: 
      break;
   case SPACE: 
	  brick_action = MOVE_DROP;
      break;
   case DOWN: 
	  brick_action = MOVE_LEFT;
      break;
   case UP: 
	  brick_action = MOVE_RIGHT;
      break;
   case LEFT: 
	  brick_action = ROTATION;
      break;
   case RIGHT:
	  brick_action = MOVE_DOWN;
      break;
   default :
	  brick_action = FREE_DROP;
      break;
 }
}
///////////////////////////////////////////////////////////////
int getch(void)
{
	static int wakeup=0;
	static int tamper=0;
	static int user=0;
	
	static int joy_up_count=0;
	static int joy_down_count=0;
	static int joy_left_count=0;
	static int joy_right_count=0;
	
	
	static int joy_up_state=J_INITIAL;
	static int joy_down_state=J_INITIAL;
	static int joy_left_state=J_INITIAL;
	static int joy_right_state=J_INITIAL;
	
	if(joycenter==0 && JOY_GetKeys() == JOY_CENTER) {
				joycenter=1;
			} else if (joycenter==1 && JOY_GetKeys() == !JOY_CENTER) {
				if(mode == 0) mode = 1;
				else mode = 0;
				joycenter=0;
			} 
	
	if(mode ==0 ) return 0;

		if(wakeup==0 && WAKEUP) {
		wakeup=1;
	}
	else if(wakeup==1 && !WAKEUP) {
		wakeup=0;
		return ESC;
	}
		if(tamper==0 && !TAMPER) {
			tamper=1;
	}
	else if(tamper==1 && TAMPER) {
		tamper=0;
		return ENTER;
	}
		if(user==0 && !USER) {
		user=1;
	}
	else if(user==1 && USER) {
		user=0;
		return SPACE;
	}
	
		if (JOY_GetKeys() == JOY_LEFT) {
				if ((++joy_left_count) == TH) {
					joy_left_count= 0;
				}
				joy_left_state= DOWN;
			}

			else if (JOY_GetKeys() == JOY_RIGHT) {
				if ((++joy_right_count) == TH) {
					joy_right_count= 0;
				}
				joy_right_state= DOWN;
			}

			else if (JOY_GetKeys() == JOY_UP) {
				if ((++joy_up_count) == TH) {
					joy_up_count= 0;
				}
				joy_up_state= DOWN;
			}

			else if (JOY_GetKeys() == JOY_DOWN) {
				if ((++joy_down_count) == TH) {
					joy_down_count= 0;
				}
				joy_down_state= DOWN;
			}
			
			else {
				if (joy_left_state==DOWN) {
					joy_left_count= 0;

					joy_left_state= UP;
					return LEFT;
				}
				else if (joy_right_state==DOWN) {
					joy_right_count= 0;
					joy_right_state= UP;
					return RIGHT;
				}
				else if (joy_up_state==DOWN) {
					joy_up_count= 0;
					joy_up_state= UP;
					return UP;
				}
				else if (joy_down_state==DOWN) {
					joy_down_count= 0;
					joy_down_state= UP;
					return DOWN;
				}
			}

	

	
	return 5;
}
//////////////////////////////////////////////////////////////
