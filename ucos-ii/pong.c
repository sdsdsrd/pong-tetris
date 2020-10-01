//임베디드 시스템 및 실험_과제2 모범 답안

/* 
 * Brian Chrzanowski's Terminal Pong
 * Fri Dec 02, 2016 17:00
 */
#include <ucos_ii.h>
//#include <ncurses.h>
#include <string.h>
#include <stdio.h>
//#include <unistd.h>
//////////////////////////////////////////////////////////////
#include <stm32f4xx.h>
#include "GLCD.h"
#include "JOY.h"
#include "I2C.h"

//#define DELAY 30000
//#define ERR 20 //게임 탈출 
//////////////////////////////////////////////////////////////
#define J_INITIAL -1
#define J_DOWN 0
#define J_UP 1
#define TH 100
//////////////////////////////////////////////////////////////
static int joycenter;
static int mode;

typedef struct paddle {
	/* paddle variables */
	int x;
	int y;    /* y is the 'top' of the paddle */
	int len;
	int score;
} paddle_t;

typedef struct ball {
	/* ball variables */
	int x;
	int y;
	int next_x;
	int next_y;
	int x_vel;
	int y_vel;
} ball_t;

typedef struct dimensions {
	int x;
	int y;
} dimensions_t;

void draw_ball(ball_t *input);
void draw_paddle(paddle_t *paddle);
void draw_score(paddle_t *inpt_paddle, dimensions_t *wall);
void paddle_collisions(ball_t *inpt_ball, paddle_t *inpt_paddle);
void paddle_pos(paddle_t *pddl, dimensions_t *wall, int dir);

int wall_collisions(ball_t *usr_ball, dimensions_t *walls);
int kbdhit();
int getch(void);

int pongmain(int argc, char **argv){
		
	  int i;
		char buf [200];
	  char gameover [200];
		char gamepause [200];

	/* initialize curses */
	dimensions_t walls = { 0 };
		
	/* set the paddle variables */
	paddle_t usr_paddle = { 0 };
	
		/* set the ball */
	ball_t usr_ball = { 0 };
	
	/* we actually have to store the user's keypress somewhere... */
	int keypress;
	int run = 1;

	JOY_Init();
	GLCD_Init();
	GLCD_Clear(Black);

	GLCD_SetTextColor(White);
	GLCD_SetBackColor(Black);
	
	walls.y = 26;
	walls.x = 20;
	
	//getmaxyx(stdscr, walls.y, walls.x); /* get dimensions */
	usr_paddle.x = 5;
	usr_paddle.y = 11;
	usr_paddle.len = walls.y / 10; 
	usr_paddle.score = 0; 

	usr_ball.x = walls.x / 2;
	usr_ball.y = walls.y / 2;
	usr_ball.next_x = 0;
	usr_ball.next_y = 0;
	usr_ball.x_vel = 1;
	usr_ball.y_vel = 1;

	

	while (run) {
		
		//while (getch()==1) {
			//getmaxyx(stdscr, walls.y, walls.x);
	  walls.y = 26;
	  walls.x = 20;
			
			GLCD_Clear(Black); /* clear screen of all printed chars */

			draw_ball(&usr_ball);
			draw_paddle(&usr_paddle);
			draw_score(&usr_paddle, &walls);

			for(i=0 ; i<100000 ; i++);
			
			if(joycenter==0 && JOY_GetKeys() ==JOY_CENTER) {
				joycenter=1;
			} else if (joycenter==1 && JOY_GetKeys() ==!JOY_CENTER) {
				if(mode == 0) mode = 1;
				else mode = 0;
				joycenter=0;
			}

		/* we fell out, get the key press */
			keypress = JOY_GetKeys();
		if(mode==1) keypress = NULL;

		switch (keypress) {
			
		case JOY_LEFT:
		case JOY_RIGHT:
			paddle_pos(&usr_paddle, &walls, keypress);
			break;

		case JOY_DOWN: 
			sprintf(gamepause, "PAUSE - press any key to resume");
			GLCD_DisplayString(15, 15, 0, gamepause);
			while (1) {
				keypress = JOY_GetKeys();
				if (keypress == JOY_LEFT) {
					break;
				}
			}
			break;
		
		case JOY_UP:
			run = 0;
			break;
		}
		 
		

		
				
					/* set next positions */
			usr_ball.next_x = usr_ball.x + usr_ball.x_vel;
			usr_ball.next_y = usr_ball.y + usr_ball.y_vel;

			/* check for collisions */
			paddle_collisions(&usr_ball, &usr_paddle);
			if (wall_collisions(&usr_ball, &walls)) {
				run = 0;
				break;
			}
			OSTaskChangePrio(10,12);
	}
//}

	//endwin();
	GLCD_Clear(Black);
	//printf("GAME OVER\nFinal Score: %d\n", usr_paddle.score);
	sprintf(gameover, "GAME OVER");
	GLCD_DisplayString(13, 7, 0, gameover);
	sprintf(gameover, "Final Score: %d", usr_paddle.score);
	GLCD_DisplayString(15, 5, 0, gameover);

	return 0;
}

/*
 * function : paddle_pos
 * purpose  : have a function that will return a proper 'y' value for the paddle
 * input    : paddle_t *inpt_paddle, dimensions_t *wall, char dir
 * output   : void
 */

void paddle_pos(paddle_t *pddl, dimensions_t *wall, int dir)
{
	if (dir == JOY_RIGHT) { /* moving down */
		if (pddl->y + pddl->len + 1 <= wall->y)
			pddl->y++;
	} 	else if (dir == JOY_LEFT){          /* moving up (must be 'k') */
		if (pddl->y - 1 >= 0)
			pddl->y--;
	}

	return;
}

/*
 * function : wall_collisions
 * purpose  : to check for collisions on the terminal walls
 * input    : ball_t *, dimensions_t *
 * output   : nothing (stored within the structs)
 */
int wall_collisions(ball_t *usr_ball, dimensions_t *walls)
{
	/* check if we're supposed to leave quick */
	if (usr_ball->next_x < 0) {
		return 1;
	}

	/* check for X */
	if (usr_ball->next_x >= walls->x) {
		usr_ball->x_vel *= -1;
	} else {
		usr_ball->x += usr_ball->x_vel;
	}

	/* check for Y */
	if (usr_ball->next_y >= walls->y || usr_ball->next_y < 0) {
		usr_ball->y_vel *= -1;
	} else {
		usr_ball->y += usr_ball->y_vel;
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

void paddle_collisions(ball_t *inpt_ball, paddle_t *inpt_paddle)
{
	/* 
	* simply check if next_% (because we set the next_x && next_y first) 
	* is within the bounds of the paddle's CURRENT position
	*/

	if (inpt_ball->next_x == inpt_paddle->x) {
		if (inpt_paddle->y <= inpt_ball->y &&
			inpt_ball->y <= 
			inpt_paddle->y + inpt_paddle->len) {

			inpt_paddle->score++;
			inpt_ball->x_vel *= -1;
		}
	}

	return;
}

/* -------------------------------------------------------------------------- */

/*
 * functions : draw_ball && draw_paddle
 * purpose   : condense the drawing functions to functions
 * input     : ball_t * && paddle_t *
 * output    : void
 */
void draw_ball(ball_t *input)
{
	//mvprintw(input->y, , "O");
	GLCD_DisplayChar(input->y, input->x, 0, 'O');
	return;
}

void draw_paddle(paddle_t *paddle)
{
	int i;

	for (i = 0; i < paddle->len; i++)
		//mvprintw(paddle->y + i, paddle->x, "|");
		GLCD_DisplayChar(paddle->y + i, paddle->x, 0, '|');

	return;
}

void draw_score(paddle_t *inpt_paddle, dimensions_t *wall)
{
		  char scorebuf [128];
	//mvprintw(0, wall->x / 2 - 7, "Score: %d", inpt_paddle->score);
	 	sprintf(scorebuf, "Score: %d", inpt_paddle->score);
	  GLCD_DisplayString(0, wall->x / 2 - 7, 0, scorebuf);


	return;
}

/* -------------------------------------------------------------------------- */

/*
 * function : kbdhit
 * purpose  : find out if we've got something in the input buffer
 * input    : void
 * output   : 0 on none, 1 on we have a key
 */
