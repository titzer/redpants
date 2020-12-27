//////////////////////////////////////////////////////////////////////////////
// Project: Redpants Kernel                                                 //
// Date:    12-18-2001                                                      //
// Module:  ia32/devices/kterm.c                                            //
// Purpose: This is the standard VGA terminal for output.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                   Copyright 1999-2002, Ben L. Titzer                     //
//////////////////////////////////////////////////////////////////////////////

#include <base/configure.h>
#include <ia32/devices/io.h>

// define ports
#define VGA_CURSOR_CMND_PORT  0x3d4
#define VGA_CURSOR_DATA_PORT  0x3d5

// text buffer constants
#define VGA_MEMORY       0xB8000
#define VGA_CHAR_SIZE    2
#define VGA_LINES        25
#define VGA_COLS         80
#define VGA_LINE_SIZE    (VGA_COLS*VGA_CHAR_SIZE)
#define VGA_SIZE         (VGA_LINES*VGA_LINE_SIZE)

void vgaterm_updatecursor(byte x, byte y);
void vgaterm_readcursorpos(void);

u16i *vga_memory  = (u16i *)VGA_MEMORY; // video memory buffer

/////////////////////////////////////////////
// The VGATERM terminal structure
/////////////////////////////////////////////
struct terminal_vtbl vgaterm_vtbl = {
  TERMINAL_VTBL_ENTRIES(vgaterm)
};
terminal vgaterm = {
  &vgaterm_vtbl,
  13
};

//////////////////////////////////////////////////////////////////////////////
//  initialize the standard VGA console                                     //
//////////////////////////////////////////////////////////////////////////////
word vgaterm_init(terminal *)
{
  u16i textpos;

  vga_memory  = (u16i *)VGA_MEMORY; // are initialized

  fullbox.attr = TEXT_NORMAL;      // default attribute
  fullbox.left = 0;                // start a left
  fullbox.top = 0;                 // start at top
  fullbox.right = VGA_COLS - 1;   // use whole width
  fullbox.bottom = VGA_LINES - 1; // use whole height

  // get cursor position and convert it to an offset into memory:

  outportb(CURSOR_COMMAND_PORT, 0x0e);	// get cursor Y
  textpos = inportb(CURSOR_DATA_PORT);
  textpos <<= 8;
  outportb(CURSOR_COMMAND_PORT, 0x0f);	// get cursor X
  textpos += inportb(CURSOR_DATA_PORT);
  textpos <<= 1;                       // each character is 2 bytes

  fullbox.curx = (textpos % LINE_SIZE)>>1;
  fullbox.cury = (textpos / LINE_SIZE);
}

word vgaterm_open(terminal *t)
{
}

word vgaterm_puts(terminal *t, cstr s)
{
}

word vgaterm_putch(terminal *t, char c)
{
}

word vgaterm_clear(terminal *t)
{
}

word vgaterm_movecursor(terminal *t, word col, word row){
}

word vgaterm_setattr(terminal *t, word attr)
{
}

word vgaterm_close(terminal *t)
{
}

//////////////////////////////////////////////////////////////////////////////
//  clear contents of box                                                   //
//////////////////////////////////////////////////////////////////////////////
void vgaterm_clear_box(textbox *t)
{
  u32i row, col;
  u16i *text_mem;
  u16i junk;
  
  if ( t == null ) { vgaterm_clear(); return; }

  junk = (((u16i)t->attr)<<8) | 0x20;

  text_mem = t->left + vga_memory;
  text_mem += t->top*VGA_LINES;

  for ( row = t->top; row <= t->bottom; row++ )
    {
      for ( col = t->left; col <= t->right; col++ )
	{
	  *text_mem = junk;
	  text_mem++;
	}

      text_mem += VGA_COLS;
    }

  vgaterm_movecursor_box(t, 0, 0);
}


//////////////////////////////////////////////////////////////////////////////
//  print string into the default box using attribute                       //
//////////////////////////////////////////////////////////////////////////////
void vgaterm_puts_box(textbox *t, cstr str, byte attr)
{
  u16i *lptr;               // line pointer
  u16i *bptr;
  u16i atmp;
  byte col, rtmp, ltmp;

  if ( t == null ) t = &fullbox;

  col = t->curx + t->left;   // column counter
  rtmp = t->right;  // right temp
  ltmp = t->left;   // left temp

  if ( attr == TEXT_DEFAULT )
    attr = t->attr; // default attribute
  else attr = (attr&0x0f)|(t->attr&0xf0);
  
  atmp = ((u16i)attr) << 8;

  lptr = vga_memory + ((t->cury+t->top)*VGA_COLS); // point to current line
  bptr = vga_memory + ((t->bottom)*VGA_COLS);    // point to last line

  while ( *str )
    {
     switch ( *str )
        {
        case '\n': lptr += VGA_COLS; col = ltmp; break; // new line
        case '\r': col = ltmp; break;                    // carriage return
        case '\b': if ( --col < ltmp ) col = ltmp; break; // backspace
        default:
          lptr[col] = ((byte)(*str)) | atmp;   // default, just copy
          col++;             // increment column
        }

     if ( col > rtmp )         // if we went beyond the end
        {
	lptr += VGA_COLS;      // go to next line
        col = ltmp;            // go to left
        }

     if ( lptr > bptr )    // if we went beyond the bottom
        {
        scrollup_box(t, 1);     // scroll box
        lptr -= VGA_COLS;      // decrement line pointer
        }

     str++;
#if ( TYPEWRITER )
     delay(TYPEDELAY);
     vgaterm_movecursor_box(t, col - t->left,
                 (lptr-vga_memory)/VGA_COLS - t->top);
#endif
     }

  vgaterm_movecursor_box(t, col - t->left,
              (lptr-vga_memory)/VGA_COLS - t->top);

}


//////////////////////////////////////////////////////////////////////////////
//  update the cursor on screen                                             //
//////////////////////////////////////////////////////////////////////////////
void vgaterm_updatecursor(byte x, byte y) // update the cursor after printing
{
  u16i offset;  // divide by two

  offset = (y * VGA_COLS) + x;

  outportb(CURSOR_COMMAND_PORT, 0x0f);           // output command port
  outportb(CURSOR_DATA_PORT,    offset & 0x0ff);
  outportw(CURSOR_COMMAND_PORT, 0x0e);
  outportb(CURSOR_DATA_PORT,    offset >> 8);
}

//////////////////////////////////////////////////////////////////////////////
//  clear entire vgaterm, not just default box                              //
//////////////////////////////////////////////////////////////////////////////
void vgaterm_clear(void)
{
  u32i cntr;  // temporary counter
  u16i junk;

  junk = fullbox.attr;
  junk = (junk << 8)|0x20;

  for ( cntr = 0; cntr < (VGA_LINES*VGA_COLS); cntr++ )
     {
       vga_memory[cntr] = junk;
     }

  updatecursor(0, 0); // update cursor position

  fullbox.curx = 0; // reset default box too
  fullbox.cury = 0;
}

//////////////////////////////////////////////////////////////////////////////
//  get default text box                                                    //
//////////////////////////////////////////////////////////////////////////////
textbox *vgaterm_getfullbox(void)
{
  return &fullbox; // simply return the base text box
}

//////////////////////////////////////////////////////////////////////////////
//  put character into default text box                                     //
//////////////////////////////////////////////////////////////////////////////
void vgaterm_putch_box(textbox *t, byte ch, byte attr)
{
  u16i offset;
  byte line, col;
  u16i result;

  if ( t == null ) t = &fullbox;

  line = t->cury + t->top; // starting line on screen
  col = t->curx + t->left; // starting column on screen

  if ( attr == TEXT_DEFAULT ) attr = t->attr;
  else attr = (attr&0x0f)|(t->attr&0xf0);

  switch ( ch )
     {
     case '\n': line++; col = t->left; break; // go to next line
     case '\r': col = t->left; break;         // go to beginning of line
     case '\b': if ( --col < t->left ) col = t->left; break; // backspace
     default:
       offset = line*VGA_COLS + col; // calculate offset
       result = ch | (((u16i)attr)<<8);
       vga_memory[offset] = result;           // set video memory
       col++;
     }

  if ( col > t->right ) // went off the right
     {
     line++;
     col = t->left;
     }

  if ( line > t->bottom ) // went off the bottom
     {
     scrollup_box(t, 1);
     line--;
     }

  vgaterm_movecursor_box(t, col - t->left, line - t->top); // update cursor
}


//////////////////////////////////////////////////////////////////////////////
//  scroll up contents of box                                               //
//////////////////////////////////////////////////////////////////////////////
void vgaterm_scrollup_box(textbox *t, byte num)
{
  u16i *cur, *rlimit, *dend, *end, atmp = (t->attr)<<8;
  u16i noffset, skip;

  end = 1+vga_memory + t->right + t->bottom*VGA_COLS;
  cur = vga_memory + t->left + t->top*VGA_COLS;
  rlimit = cur + (t->right - t->left + 1);
  skip = VGA_COLS-1 - (t->right - t->left); 
  noffset = VGA_COLS*num;
  dend = end - noffset;

  while ( cur < dend ) // copy lines
     {
     while ( cur < rlimit )
        {
        *cur = *(cur+noffset);
        cur++;
        }
     cur += skip;
     rlimit += VGA_COLS;
     }
  
  while ( cur < end )
     {
     while ( cur < rlimit )
        {
        *cur = atmp;
        cur++;
        }
     cur += skip;
     rlimit += VGA_COLS;
     }
}

//////////////////////////////////////////////////////////////////////////////
//  move cursor within box                                                  //
//////////////////////////////////////////////////////////////////////////////
void vgaterm_movecursor_box(textbox *t, byte x, byte y)
{
  if ( x > (t->right - t->left) ) return; // went past right edge
  if ( y > (t->bottom - t->top) ) return; // went past bottom

  t->curx = x; // set the structure accordingly
  t->cury = y;

  vgaterm_updatecursor(x + t->left, y + t->top); // move onscreen cursor
}
