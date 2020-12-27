//////////////////////////////////////////////////////////////////////////////
// Project: Redpants                                                        //
// Date:    11-30-2000                                                      //
// Module:  tetris.c                                                        //
// Purpose: This module contains a simple tetris game for a novelty and for //
//          testing some features of the kernel. It was originally written  //
//          by Chris Giese to run on top of Cosmos by Chris Giese. I have   //
//          rewritten portions of the code and ported it to redpants.       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                    Created 2000, by Ben L. Titzer                        //
//                       original by Chris Giese                            //
//////////////////////////////////////////////////////////////////////////////

#define SCR_HEIGHT 18
#define SCR_WIDTH  10
#define SCR_LEFT  32
#define SCR_TOP   1
#define BORDER_COLOR WHITE
#define BACK_COLOR   BLACK
#define FLASH_COLOR  YELLOW
#define SCORETEXT_COLOR LIGHTGRAY
#define SCORENUM_COLOR  LIGHTCYAN

#define DIR_UP		{ 0, -1 }
#define DIR_DN		{ 0, +1 }
#define DIR_LT		{ -1, 0 }
#define DIR_RT		{ +1, 0 }
#define DIR_UP2	{ 0, -2 }
#define DIR_DN2	{ 0, +2 }
#define DIR_LT2	{ -2, 0 }
#define DIR_RT2	{ +2, 0 }

#define KEY_d     'd'
#define KEY_D     'D'
#define KEY_f     'f'
#define KEY_F     'F'
#define KEY_q     'q'
#define KEY_Q     'Q'
#define KEY_UP    72<<8
#define KEY_DOWN  80<<8
#define KEY_LEFT  75<<8
#define KEY_RIGHT 77<<8


#include <base/shell/tetris.h>
#include <base/devices/kterm.h>
#include <base/devices/keyboard.h>
#include <base/devices/ktimer.h>

typedef struct
{
  u8i delta_x,
      delta_y;
} vector __attribute((packed));

typedef struct
{
  u8i shp_clockwise,  // current shape rotated clockwise
      shp_cclockwise; // current shape rotated counterclockwise
  u8i color;

  vector path[4]; // blocks
} shape __attribute__((packed));

shape shapes[] =
/* shape #0:			cube */
{
	{ 0, 0, LIGHTRED, { DIR_UP, DIR_RT, DIR_DN, DIR_LT }},
/* shapes #1 & #2:		bar */
	{ 2, 2, LIGHTGREEN, { DIR_LT, DIR_RT, DIR_RT, DIR_RT }},
	{ 1, 1, LIGHTGREEN, { DIR_UP, DIR_DN, DIR_DN, DIR_DN }},
/* shapes #3 & #4:		'Z' shape */
	{ 4, 4, LIGHTCYAN, { DIR_LT, DIR_RT, DIR_DN, DIR_RT }},
	{ 3, 3, LIGHTCYAN, { DIR_UP, DIR_DN, DIR_LT, DIR_DN }},
/* shapes #5 & #6:		'S' shape */
	{ 6, 6, LIGHTRED, { DIR_RT, DIR_LT, DIR_DN, DIR_LT }},
	{ 5, 5, LIGHTRED, { DIR_UP, DIR_DN, DIR_RT, DIR_DN }},
/* shapes #7, #8, #9, #10:	'J' shape */
	{ 8, 10, CYAN, { DIR_RT, DIR_LT, DIR_LT, DIR_UP }},
	{ 9, 7,  CYAN, { DIR_UP, DIR_DN, DIR_DN, DIR_LT }},
	{ 10, 8, CYAN, { DIR_LT, DIR_RT, DIR_RT, DIR_DN }},
	{ 7, 9,  CYAN, { DIR_DN, DIR_UP, DIR_UP, DIR_RT }},
/* shapes #11, #12, #13, #14:	'L' shape */
	{ 12, 14, LIGHTBLUE, { DIR_RT, DIR_LT, DIR_LT, DIR_DN }},
	{ 13, 11, LIGHTBLUE, { DIR_UP, DIR_DN, DIR_DN, DIR_RT }},
	{ 14, 12, LIGHTBLUE, { DIR_LT, DIR_RT, DIR_RT, DIR_UP }},
	{ 11, 13, LIGHTBLUE, { DIR_DN, DIR_UP, DIR_UP, DIR_LT }},
/* shapes #15, #16, #17, #18:	'T' shape */
	{ 16, 18, LIGHTGREEN, { DIR_UP, DIR_DN, DIR_LT, DIR_RT2 }},
	{ 17, 15, LIGHTGREEN, { DIR_LT, DIR_RT, DIR_UP, DIR_DN2 }},
	{ 18, 16, LIGHTGREEN, { DIR_DN, DIR_UP, DIR_RT, DIR_LT2 }},
	{ 15, 17, LIGHTGREEN, { DIR_RT, DIR_LT, DIR_DN, DIR_UP2 }}
};

u16i lines, level, speed;
u32i score;
u8i gamescreen[SCR_HEIGHT][SCR_WIDTH];
u8i rowchanged[SCR_HEIGHT];

u16i waitforkey(u16i *maxms); // waits for a key for some milliseconds
void cleargamescreen(void);
void printgamescreen(void);
void printgameborder(void);
void drawshape(u8i x, u8i y, u8i shp);
void eraseshape(u8i x, u8i y, u8i shp);
void drawblock(u8i x, u8i y, u8i color);
void refresh(void);
u8i collapse(void);
void flashrows(u8i *);
u8i blockhit(u8i x, u8i y);
u8i shapehit(u8i x, u8i y, u8i shp);
u32i random(void);

u32i seeda = 8237489; // random seed

//////////////////////////////////////////////////////////////////////////////
//  cleargamescreen: clear internal game screen                             //
//////////////////////////////////////////////////////////////////////////////
void cleargamescreen(void)
{
  u32i cntr;
  u8i *ptr = gamescreen[0];

  for ( cntr = 0; cntr < SCR_HEIGHT*SCR_WIDTH; cntr++ )
     ptr[cntr] = BACK_COLOR;
  for ( cntr = 0; cntr < SCR_HEIGHT; cntr++ )
     rowchanged[cntr] = 1;
}

//////////////////////////////////////////////////////////////////////////////
//  print internal game screen to display                                   //
//////////////////////////////////////////////////////////////////////////////
void printgamescreen(void)
{
  u32i row, col;
  u8i *rptr;

  movecursor(SCR_LEFT, SCR_TOP);

  for ( row = 0; row < SCR_HEIGHT; row++ ) // loop through rows
     {
     movecursor(SCR_LEFT, SCR_TOP+row);
     rptr = gamescreen[row];
     for ( col = 0; col < SCR_WIDTH; col++ )
        printstring("\xDB\xDB", rptr[col]);
     rowchanged[row] = 0;
     }

}

//////////////////////////////////////////////////////////////////////////////
//  print border to screen                                                  //
//////////////////////////////////////////////////////////////////////////////
void printgameborder(void)
{
  int cntr;

  movecursor(SCR_LEFT-2, SCR_TOP-1);
  putch(0xC9, BORDER_COLOR); // put upper left

  for ( cntr = 0; cntr < (SCR_WIDTH*2+2); cntr++ )
     putch(0xCD, BORDER_COLOR);

  putch(0xBB, BORDER_COLOR); // upper right

  for ( cntr = 0; cntr < SCR_HEIGHT; cntr++ )
     {
     movecursor(SCR_LEFT-2, SCR_TOP+cntr);
     putch(0xBA, BORDER_COLOR);
     putch(' ', BORDER_COLOR);
     movecursor(SCR_LEFT+SCR_WIDTH*2, SCR_TOP+cntr);
     putch(' ', BORDER_COLOR);
     putch(0xBA, BORDER_COLOR);
     }


  movecursor(SCR_LEFT-2, SCR_TOP+SCR_HEIGHT);
  putch(0xC8, BORDER_COLOR); // put lower left

  for ( cntr = 0; cntr < (SCR_WIDTH*2+2); cntr++ )
     putch(0xCD, BORDER_COLOR);

  putch(0xBC, BORDER_COLOR); // lower right
}

//////////////////////////////////////////////////////////////////////////////
//  print border to screen                                                  //
//////////////////////////////////////////////////////////////////////////////
void refresh(void)
{
  u32i row, col;
  u8i *rptr;

  movecursor(SCR_LEFT, SCR_TOP);

  for ( row = 0; row < SCR_HEIGHT; row++ ) // loop through rows
     {
     if ( !rowchanged[row] ) continue;
     movecursor(SCR_LEFT, SCR_TOP+row);
     rptr = gamescreen[row];
     for ( col = 0; col < SCR_WIDTH; col++ )
        printstring("\xDB\xDB", rptr[col]);
     rowchanged[row] = 0;
     }

}

//////////////////////////////////////////////////////////////////////////////
//  drawshape draws the specified shape onto the game screen                //
//////////////////////////////////////////////////////////////////////////////
void drawshape(u8i x, u8i y, u8i shp)
{
  u8i cntr;

  for ( cntr = 0; cntr < 4; cntr++ )
    {
    drawblock(x, y, shapes[shp].color);
    x += shapes[shp].path[cntr].delta_x;
    y += shapes[shp].path[cntr].delta_y;
    }
    drawblock(x, y, shapes[shp].color);
}

//////////////////////////////////////////////////////////////////////////////
//  eraseshape erases the shape from the game board                         //
//////////////////////////////////////////////////////////////////////////////
void eraseshape(u8i x, u8i y, u8i shp)
{
  u8i cntr;

  for ( cntr = 0; cntr < 4; cntr++ )
    {
    drawblock(x, y, BACK_COLOR);
    x += shapes[shp].path[cntr].delta_x;
    y += shapes[shp].path[cntr].delta_y;
    }
    drawblock(x, y, BACK_COLOR);
}

//////////////////////////////////////////////////////////////////////////////
//  drawblock draws a single block onto the screen                          //
//////////////////////////////////////////////////////////////////////////////
void drawblock(u8i x, u8i y, u8i color)
{
  gamescreen[y][x] = color;
  rowchanged[y] = true;
}

//////////////////////////////////////////////////////////////////////////////
//  detect if a shape hits something already on screen                      //
//////////////////////////////////////////////////////////////////////////////
u8i shapehit(u8i x, u8i y, u8i shp)
{
  u8i cntr;

  for ( cntr = 0; cntr < 4; cntr++ )
     {
     if ( blockhit(x, y) ) return true;
     x += shapes[shp].path[cntr].delta_x;
	  y += shapes[shp].path[cntr].delta_y;
     }

  return blockhit(x, y);
}

//////////////////////////////////////////////////////////////////////////////
//  detect if a block is used                                               //
//////////////////////////////////////////////////////////////////////////////
u8i blockhit(u8i x, u8i y)
{
  if ( y >= SCR_HEIGHT ) return true;
  else if ( x >= SCR_WIDTH ) return true;

  return gamescreen[y][x] != BACK_COLOR;
}

//////////////////////////////////////////////////////////////////////////////
//  collapse: collapses any complete lines                                  //
//////////////////////////////////////////////////////////////////////////////
u8i collapse(void)
{
  u8i solidrows = 0, solid[SCR_HEIGHT];
  u8i *row;
  s16i tmp, rcntr, ccntr, rsolid;

  for ( rcntr = 0; rcntr < SCR_HEIGHT; rcntr++ ) // find solid rows
     {
     row = gamescreen[rcntr]; // get pointer to current row
     rsolid = true;           // assume row is solid

     for ( ccntr = 0; ccntr < SCR_WIDTH; ccntr++ ) // look at each block in row
        {
        if ( row[ccntr] == BACK_COLOR )   // empty, row not solid
           { rsolid = false; break; }
        }

     solid[rcntr] = rsolid;     // remember this row is solid
     if ( rsolid ) solidrows++; // keep track of number of solid rows
     }

  if ( solidrows == 0 ) return 0;

  flashrows(solid); // flash the rows (just an effect)

  for ( rcntr = tmp = SCR_HEIGHT-1; rcntr >= 0; rcntr--, tmp-- )
     {
     while ( solid[tmp] && (tmp >= 0) ) tmp--;

     if ( tmp < 0)
        {
        for ( ccntr = 0; ccntr < SCR_WIDTH; ccntr++)
        gamescreen[rcntr][ccntr] = BACK_COLOR;
        rowchanged[rcntr] = true;
        }
     else if ( rcntr != tmp )
        {
        for ( ccntr = 0; ccntr < SCR_WIDTH; ccntr++)
        gamescreen[rcntr][ccntr] = gamescreen[tmp][ccntr];
        rowchanged[rcntr] = true;
        }
     }

  return solidrows;
}

void flashrows(u8i *which)
{
  u8i *row, rcntr, ccntr;
  u8i color;
  u8i colors[4] = { BLUE, RED, GREEN, WHITE };

  for ( color = 0; color < 12; color++ )
     {
     for ( rcntr = 0; rcntr < SCR_HEIGHT; rcntr++ )
        {
        if ( !which[rcntr] ) continue;

        row = gamescreen[rcntr];
        for ( ccntr = 0; ccntr < SCR_WIDTH; ccntr++ )
           row[ccntr] = colors[color%4];
        rowchanged[rcntr] = true;
        }
     refresh();
     delay(50);
     }
}

//////////////////////////////////////////////////////////////////////////////
//  waits for a keypress for a maximum amount of time                       //
//////////////////////////////////////////////////////////////////////////////
u16i waitforkey(u16i *maxms)
{
  u16i key;

  if ( kbhit() )
     {
     if ( (key = getch()) == 0 ) key = key || (((u16i)getch())<<8);
     return key;
     }

  while ( *maxms ) // count down to zero
     {
     delay(1);     // wait one millisecond
     *maxms = (*maxms)-1;  // decrement
     if ( kbhit() ) // if key hit, return it
        {
        if ( (key = getch()) == 0 ) key = key || (((u16i)getch())<<8);
        return key;
        }
     }

   return 0; // otherwise return 0;
}

//////////////////////////////////////////////////////////////////////////////
//  runs the game                                                           //
//////////////////////////////////////////////////////////////////////////////
void run_tetris(void)
{
  u16i dcount;
  u16i key;
  u8i shp, x, y;
  u8i new_shp, new_x, new_y;
  u8i quit = 0;
  u8i fell = 0;
  u8i newlines = 0;
  u16i scores[5] = { 0, 100, 400, 800, 2000 };

  dcount = speed = 500;
  lines = 0;
  level = 1;
  score = 0;

  clear();

  printstring("\n         T E T R I S\n", YELLOW);
  printstring("\nGame concept Alexey Pazhitnov", LIGHTRED);
  printstring("\n    Game code Chris Giese", LIGHTRED);
  printstring("\n Redpants port Ben L. Titzer\n\n", LIGHTRED);
  printstring("\n         Q = quit", LIGHTRED);
  printstring("\n    S = left, D = down", LIGHTRED);
  printstring("\n F = right, J, K = rotate", LIGHTRED);

/// test
  printstring("\xDB e", BACK_COLOR);
  printstring("ee", BACK_COLOR);

  cleargamescreen();
  printgameborder();
  printgamescreen();

  movecursor(SCR_LEFT+(SCR_WIDTH*2)+4, 2);
  printstring("Score: ", SCORETEXT_COLOR);
  printlong(score, SCORENUM_COLOR);
  movecursor(SCR_LEFT+(SCR_WIDTH*2)+4, 3);
  printstring("Level: ", SCORETEXT_COLOR);
  printlong(level, SCORENUM_COLOR);
  movecursor(SCR_LEFT+(SCR_WIDTH*2)+4, 4);
  printstring("Lines: ", SCORETEXT_COLOR);
  printlong(lines, SCORENUM_COLOR);

  movecursor(SCR_LEFT+4, SCR_TOP+SCR_HEIGHT/2-1);
  printstring("Press any key", LIGHTCYAN);

  if ( getch() == 0 ) getch();

  printgamescreen();
  seeda ^= get_timerticks()*1438840;

  shp = random() % 19;
  x = SCR_WIDTH/2;
  y = 1;

  drawshape(x, y, shp);
  refresh();

  while ( !quit )
     {
     fell = 0;
     new_shp = shp;
     new_x = x;
     new_y = y;
     key = waitforkey(&dcount); // wait for a key
     if ( key == 0 )            // no key hit
        {
        fell = 1;               // block fell a square
        new_y++;
        dcount = speed;
        }
     else switch ( key )
        {
        case 'd':              // move block down
        case 'D':              // move block down
          if ( new_y < SCR_HEIGHT-1 ) new_y++;
          dcount = speed; // reset waiting count
          fell = 1;
          break;
        case 's':              // move block left
        case 'S':              // move block left
          if ( new_x > 0 ) new_x--;
          break;
        case 'f':              // move block right
        case 'F':              // move block right
          if ( new_x < SCR_WIDTH-1 ) new_x++;
          break;
        case 'j':
        case 'J':                 // rotate clockwise
          new_shp = shapes[shp].shp_clockwise;
          break;
        case 'k':
        case 'K':                 // rotate counter clockwise
          new_shp = shapes[shp].shp_cclockwise;
          break;
        case 'q':
        case 'Q':                 //quit
          quit = 1;
          continue;
          break;
        }

      eraseshape(x, y, shp); // erase the old shape

      if ( shapehit(new_x, new_y, new_shp) )
        {
        if ( fell ) // block got stuck somewhere, spawn new
           {
           drawshape(x, y, shp);    // draw the old shape
           new_shp = random() % 19; // get new shape
           new_x = SCR_WIDTH/2;     // new x spawn
           new_y = 1;               // new y spawn
           newlines = collapse(); // collapse any lines

           if ( newlines > 0 )
              {
              lines += newlines;
              if ( level < lines/10+1 )
                { speed *= .95; score += 3000; }
              level = lines/10+1;
              score += scores[newlines];
              movecursor(SCR_LEFT+(SCR_WIDTH*2)+11, 2);
              printlong(score, SCORENUM_COLOR);
              movecursor(SCR_LEFT+(SCR_WIDTH*2)+11, 3);
              printlong(level, SCORENUM_COLOR);
              movecursor(SCR_LEFT+(SCR_WIDTH*2)+11, 4);
              printlong(lines, SCORENUM_COLOR);
              }

           if ( shapehit(new_x, new_y, new_shp) ) // if new shape hits something
              {
              drawshape(new_x, new_y, new_shp); // draw new position
              refresh();
              movecursor(SCR_LEFT+4, SCR_TOP+SCR_HEIGHT/2-1);
              printstring("* GAME OVER *", LIGHTCYAN); // game over, maaaan!
              movecursor(SCR_LEFT+5, SCR_TOP+SCR_HEIGHT/2);
              printstring("* Press Q *", LIGHTCYAN); // game over, maaaan!

              while ( 1 )
                 {
                 x = getch();
                 if ( (x == 'q') || (x == 'Q') ) break;
                 }

              break;
              }
           }
        else // otherwise user tried to move somewhere they cant
           {
           new_x = x;
           new_y = y;
           new_shp = shp;
           }
        }

     x = new_x; // update new position
     y = new_y;
     shp = new_shp;
     drawshape(x, y, shp); // draw new position
     refresh();            // refresh the screen
     }

//  drawshape(2, 2, 1);
//  drawshape(4, 6, 4);
//  drawshape(6, 9, 11);
//  printgamescreen();

  clear();
}

u32i random(void)
{
  u32i seedb = 134839845;

  seeda ^= ~((seeda*seedb)>>8 | seedb<<8);
  seedb ^= ~((seeda*seedb)>>8 | seeda<<8);

  return seedb;
}
