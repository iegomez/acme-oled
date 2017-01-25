#ifndef SSD1305_C
#define SSD1305_C
#include "glcdfont.c"

#ifdef CATALYST_HW
//Catalyst connections
#define LCD_DC  PIN_D1//Data/!Command
#define LCD_CS  PIN_D3//Chip Select
#define LCD_RS  PIN_D2//Reset
#define MF_CS   PIN_D7
#else
//Display connections:
#define LCD_DC  PIN_C2//Data/!Command
#define LCD_CS  PIN_C1//Chip Select
#define LCD_RS  PIN_A6//Reset
#define MF_CS   PIN_C0
#endif

// From SSD1305.h by "Andrea" with a few additions
#define SSD1305_SETLOWCOLUMN 				0x00 // 0x00 - 0F Set the lower nibble of the column start address register for Page Addressing Mode 
#define SSD1305_SETHIGHCOLUMN 			0x10 //0x10 - 0x1F Set the higher nibble of the column start address register for Page Addressing Mode.  
#define SSD1305_MEMORYMODE 					0x20 //00b Hor Add, 01b Vert Add, 10b Page Add, 11b NA
#define SSD1305_SETSTARTCOLUMN 			0x21 //Column Start Address 0 - 0x83, Column End Address 0 - 0x83
#define SSD1305_SETSTARTPAGE 				0x22 //Page Start Address 0 - 0x07, Page End Address 0 - 0x07
#define SSD1305_SETSTARTLINE 				0x40 //Set display start line register from 0 - 0x3F. 
#define SSD1305_SETCONTRAST 				0x81 //1 out of 256 contrast steps. Contrast increases as the value increases.
#define SSD1305_SETBRIGHTNESS 			0x82 //1 out of 256 contrast steps. Brightness increases as the value increases.
#define SSD1305_LOOK_UP_TABLE       0x91 //Set look-up table
#define SSD1305_SEGREMAP0 					0xA0 //Column address 0 is mapped to SEG0
#define SSD1305_SEGREMAP1 					0xA1 //Column address 0x83 is mapped to SEG0
#define SSD1305_DISPLAYALLON_RESUME 0xA4 //Resume RAM content display. Output follows RAM content
#define SSD1305_DISPLAYALLON 				0xA5 //Entire display ON. Output ignores RAM content.
#define SSD1305_NORMALDISPLAY 			0xA6 //Normal display
#define SSD1305_INVERTDISPLAY 			0xA7 //Inverse display
#define SSD1305_SETMULTIPLEX 				0xA8 //Set MUX ratio to N+1 MUX, from 16MUX to 64MUX (0 to 14 are invalid) 
#define SSD1305_DISPLAY_ON_DIM      0xAC //Display ON in dim mode
#define SSD1305_DCVOLTCONVERT 			0xAD //Selects external VCC supply 
#define SSD1305_DISPLAYOFF 					0xAE //Display OFF (sleep mode) 
#define SSD1305_DISPLAYON 					0xAF //Display ON in normal mode 
#define SSD1305_SET_PAGE 						0xB0 //Set GDRAM Page Start Address for Page Addressing Mode - PAGE0~PAGE7  
#define SSD1305_COMSCANDEC 					0xC8 //Set COM Output Scan Direction  Remapped mode. Scan from COM[N‐1] to COM0
#define SSD1305_SETDISPLAYOFFSET 		0xD3 //Set vertical shift by COM from 0~63.  
#define SSD1305_SETDISPLAYCLOCKDIV 	0xD5 //Set Display Clock Divide Ratio/Oscillator frequency
#define SSD1305_AREACOLOR_POWERMODE 0xD8 //Set Area Color Mode ON/OFF & Low Power Display Mode
#define SSD1305_SETPRECHARGE 				0xD9 //Set Pre‐charge Period up to 15DCLK. 0 is not valid.
#define SSD1305_SETCOMPINS 					0xDA //Sequential COM pin configuration 
#define SSD1305_SETVCOMDETECT 			0xDB //Set VCOMH Deselect Level 0.43*VCC, 0.77*VCC, 0.83*VCC
#define SSD1305_SET_MODIFY 					0xE0 //Enter the Read/Modify/Write mode
#define SSD1305_CLR_MODIFY 					0xEE //Exit the Read/Modify/Write mode. 
#define SSD1305_NOP 								0xE3 //Command for No Operation 
#define SSD1305_SETHORISCROLL 			0x26 // Horizontal scroll setup
#define SSD1305_SETVERTSCROLL 			0xA3 // Set vertical scroll area
#define SSD1305_SETCONTSCROLL 			0x29 // Continuous vertical & horizontal scroll setup
#define SSD1305_CLRSCROLL 					0x2E // Deactivate scroll
#define SSD1305_SETSCROLL 					0x2F // Activate scroll

#define DISPLAY_WIDTH	    128 //X
#define DISPLAY_HEIGHT	  32	//Y
#define PAGEMAX           DISPLAY_HEIGHT/8    // 48pixel/8pixel= 6 pages
#define GDRAM_WIDTH       132
#define GDRAM_HEIGHT      64
#define CHAR7x5_WIDTH     5 //7x5 char
#define CHAR7x5_HEIGHT    8 //7 + blank space
#define BUFFER_LENGTH     (GDRAM_WIDTH*GDRAM_HEIGHT)/8
#define DISPLAY_NORMAL    0
#define DISPLAY_X_MIRROR  1
#define DISPLAY_Y_MIRROR  2
#define DISPLAY_XY_MIRROR 3
#define MAX_Y_CHAR        DISPLAY_HEIGHT/CHAR7x5_HEIGHT   //4
#define MAX_X_CHAR        DISPLAY_WIDTH/(CHAR7x5_WIDTH+1) //21

int8 display_buffer[BUFFER_LENGTH];
int1 buffer_update;

/*
  SSD1305 SPI Driver
  Defines:
    LCD_DC - DATA/!COMMAND
    LCD_CS - DEVICE Chip Select
  Functions:
    spi_init ----------- SPI configuration, needs hardware SPI
    oled_init ---------- Oled display initialization
    oled_write_command - Writes a command to the oled display
    oled_write_data ---- Write data to the oled display
*/

void spi_init(void){

  /*
    8bit MSB First
    Data is shifted in on every rising edge
    CLK Idle HIGH
    Max CLK speed 10MHz
  */
  #ifdef CATALYST_HW
 	setup_spi(FALSE);
  setup_spi2(SPI_MASTER|SPI_CLK_DIV_16|SPI_SCK_IDLE_HIGH|SPI_XMIT_L_TO_H); //64MHz/16 = 4MHz
  #else
  setup_spi(SPI_MASTER|SPI_CLK_DIV_16|SPI_SCK_IDLE_HIGH|SPI_XMIT_L_TO_H); //64MHz/16 = 4MHz
  #endif
}

void oled_write_command(int8 command){

	output_low(LCD_CS);
	output_low(LCD_DC);

  #ifdef CATALYST_HW
	spi_write2(command);
  #else
  spi_write(command);
  #endif

	output_high(LCD_CS);
}

void oled_write_data(int8 data){

	output_low(LCD_CS);
	output_high(LCD_DC);

	#ifdef CATALYST_HW
  spi_write2(data);
  #else
  spi_write(data);
  #endif

	output_high(LCD_CS);
}

void oled_clear_display(int8 pixel){
  int16 i;

  for (i = 0; i < BUFFER_LENGTH; ++i){
    oled_write_data(0x00);
  }
}

void oled_init(void){

  output_low(LCD_DC);
  output_low(LCD_CS);
  output_low(LCD_RS);
  delay_ms(100); //delay for the reset
  output_high(LCD_RS);

  oled_write_command(SSD1305_DISPLAYOFF);

  //oled_write_command(SSD1305_SETLOWCOLUMN + 0x00); // 0x00-0x0F set low column address
  //oled_write_command(SSD1305_SETHIGHCOLUMN + 0x00);// 0x10-0x1F set high column address
                                                   // Set column Adress 34 = 0x02 low + 0x12 high
  
  oled_write_command(SSD1305_SETDISPLAYCLOCKDIV);
  oled_write_command(0xA0);                       //Set to 130Hz
  
  oled_write_command(SSD1305_SETMULTIPLEX);
  //oled_write_command(0x2F);                     // set for 1/48, from 16MUX to 64MUX, RESET=111111b (i.e. 64MUX)
  //oled_write_command(0x3F); 
  oled_write_command(0x1F);                       //1/32 Duty (0x0F~0x3F) 
  
  oled_write_command(SSD1305_SETDISPLAYOFFSET);   // Set vertical shift by COM from 0~63.
  oled_write_command(0x00);                       // 48px by 0xC8 = (COM[N-1] - 0)
                                                  // 0px by (0xC0 = (0 - COM[N-1]

  oled_write_command(SSD1305_SETSTARTLINE);       // Set display start line (0x40 = start at line 0)
  oled_write_command(SSD1305_NORMALDISPLAY);

  oled_write_command(SSD1305_DCVOLTCONVERT);
  oled_write_command(0x8E);                       // 0x8E = Select external VCC supply (RESET)
  
  oled_write_command(SSD1305_AREACOLOR_POWERMODE);
  oled_write_command(0x00);              // monochrome mode & Normal power mode
  //oled_write_command(0x05);            // monochrome mode & low power mode 
  
  oled_write_command(SSD1305_SEGREMAP0); //Column address 0 is mapped to SEG0 (normal)
  //oled_write_command(SSD1305_SEGREMAP1); //Column address 0x83 is mapped to SEG0 (reversed)

  oled_write_command(SSD1305_COMSCANDEC); // set com output scan direction

  oled_write_command(SSD1305_SETCOMPINS);
  oled_write_command(0x12);              // Disable COM Left/Right remap

  oled_write_command(SSD1305_LOOK_UP_TABLE); // Set current drive pulse width of BANK0,
  oled_write_command(0x3F);              // BANK0: X[5:0] = 31… 63;
  oled_write_command(0x3F);              // Color A: A[5:0] same as above (RESET = 111111b)
  oled_write_command(0x3F);              // Color B: B[5:0] same as above (RESET = 111111b)
  oled_write_command(0x3F);              // Color C: C[5:0] same as above (RESET = 111111b)

  oled_write_command(SSD1305_SETCONTRAST); //contrast control mode
  oled_write_command(0xAA);
  //oled_write_command(0x8F);

  oled_write_command(SSD1305_SETBRIGHTNESS); // Set Brightness For Area Color Banks
  oled_write_command(0x5F);              // select 1 out of 256 brightness steps.
    
  oled_write_command(SSD1305_SETPRECHARGE); //Set discharge/precharge period
  oled_write_command(0xD2);                 // (A3 - A0 pre charge, A7 - A4 dis charge) period adjustment
                                            // Set Pre-Charge as 13 Clocks & Discharge as 2 Clock
  oled_write_command(SSD1305_SETVCOMDETECT);
  oled_write_command(0x34);                // ~ 0.77 x VCC (RESET)
    
  oled_write_command(SSD1305_DISPLAYALLON_RESUME);
  oled_write_command(SSD1305_NORMALDISPLAY);

  oled_write_command(SSD1305_MEMORYMODE);
  oled_write_command(0x00); //0x00, Horizontal Addressing Mode
  //oled_write_command(0x01); //0x01, Vertical Addressing Mode
  //oled_write_command(0x02); //0x02, Page Addressing Mode (RESET)

  delay_ms(100);

  //oled_clear_display();

  oled_write_command(SSD1305_DISPLAYON);
}

int16 get_buffer_position(int8 X, int8 Y, int8 mode){

  if( (X>=DISPLAY_WIDTH) || (Y>=DISPLAY_HEIGHT) )return;

  /*
    Modes
      NORMAL    0
      X-MIRROR  1
      Y-MIRROR  2
      XY-MIRROR 3
  */
  switch (mode){

    case 0:
      X = GDRAM_WIDTH - X;
      return (int16)Y*GDRAM_WIDTH + X - CHAR7x5_WIDTH;

    case 1:
      return (int16)Y*GDRAM_WIDTH + X - CHAR7x5_WIDTH;

    case 2:
      Y = GDRAM_HEIGHT/2 - Y;
      X = GDRAM_WIDTH - X;
      return (int16)Y*GDRAM_WIDTH + X - CHAR7x5_WIDTH;

    case 3:
      Y = GDRAM_HEIGHT/2 - Y;
      return (int16)Y*GDRAM_WIDTH + X - CHAR7x5_WIDTH;

    default:
      X = GDRAM_WIDTH - X;
      return (int16)Y*GDRAM_WIDTH + X - CHAR7x5_WIDTH;
  }
}

void oled_draw_pixel (int8 X, int8 Y, int8 pixel){
    
  int16 buffer_position;

  if( (X>=DISPLAY_WIDTH) || (Y>=DISPLAY_HEIGHT) )return;

  //Each block of Y contains 8 pixels Y/8 to get the correct line of the pixel

  buffer_position = get_buffer_position(X,Y/8,0);
  
  if(pixel == 1) 
      display_buffer[buffer_position] |= 0x01 << (Y%8);
     //{data |= 0x01 << (Y%8);}// set dot
  else 
     display_buffer[buffer_position] &= ~(0x01 << (Y%8));;
     //{data &= ~(0x01 << (Y%8));}     // clear dot

   buffer_update = TRUE;
}

void oled_buffer_putc(char character, int8 X, int8 Y, char mode, int8 pixel){ //write 8x6 char
    
  int8 buffer_start;
  int16 buffer_position;
  int8 i;

  font_ptr = font;

  if( (X>=DISPLAY_WIDTH) || (Y>=DISPLAY_HEIGHT) )return;
  
  for (i = 0; i < CHAR7x5_WIDTH; ++i){
    //display_buffer[buffer_position+i] = *(&font[0] + (character*CHAR7x5_WIDTH) + i);
    //font_ptr = *(font_ptr+(character*CHAR7x5_WIDTH) + i);
    //display_buffer[buffer_position+i] = font[(character*CHAR7x5_WIDTH) + i];
    buffer_position = get_buffer_position(X+i,Y,mode);
    if (pixel)
      display_buffer[buffer_position] = font_ptr[((int16)character*CHAR7x5_WIDTH) + i];
    else
      display_buffer[buffer_position] = ~font_ptr[((int16)character*CHAR7x5_WIDTH) + i];
  } 
  buffer_position = get_buffer_position(X+CHAR7x5_WIDTH,Y,0);
  if (pixel)
    display_buffer[buffer_position] = 0x00;
  else
    display_buffer[buffer_position] = 0xFF;
  
  buffer_update = TRUE;
}

void oled_buffer_printf(char* string, int8 len, int8 X, int8 Y, char mode, int8 pixel){

  int8 i;

  for (i = 0; i < len; ++i)
  {
    oled_buffer_putc(string[i],X + i*(CHAR7x5_WIDTH+1),Y, mode, pixel); //+1 to add the extra 1pixel space
  }
}

void oled_buffer_update(){

  int16 i;

  if (buffer_update){
    for (i = 0; i < BUFFER_LENGTH; ++i){
      oled_write_data(display_buffer[i]);
    }
    buffer_update = FALSE;
  }
}

void oled_fill_buffer(int8 seg){

  int16 i;

  for (i = 0; i < BUFFER_LENGTH; ++i){
    display_buffer[i] = seg;
  }
  buffer_update = TRUE;
}
#endif