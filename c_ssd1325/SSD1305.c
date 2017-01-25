/*
 * File:   SSD1305.h
 * Author: mondul_air
 *
 * Created on 7 de noviembre de 2014, 15:31
 */

#include "SSD1305.h"



/*
  SSD1305 SPI Driver
  Defines:
    LCD_DC - DATA/!COMMAND
    LCD_CS - DEVICE Chip Select
  Functions:
    oled_init ---------- Oled display initialization
    oled_write_command - Writes aoled_write_command to the oled display
    oled_write_data ---- Write data to the oled display
 */
/************TEST SPI****************/
FILE *f;

int WriteSPI2(uint8_t data)
{
    char _data[] = {data};

    write(f,_data,1);
}

/*********GPIO CONTROL***exporta para uso***************************************************/
int gpioexport(int gpioid) 
{
    FILE *filestream;

    if ((filestream=fopen("/sys/class/gpio/export","w"))==NULL) {
        printf("Error on export GPIO\n");
        return -1;
    }   
    fprintf(filestream,"%d",gpioid);
    fclose(filestream);
    return 0;
}
/*********setea direccion del pin**************************************/
int gpiosetdir(int gpioid,char *mode) 
{
    FILE *filestream;
    char filename[50];

    sprintf(filename,"/sys/class/gpio/pioC%d/direction",gpioid);
    if ((filestream=fopen(filename,"w"))==NULL) {
        printf("Error on direction setup\n");
        return -1;
    }   
    fprintf(filestream,mode);
    fclose(filestream);
    return 0;
}
/*******set pin off****************************/
int gpiosetbits(int gpioid) 
{
    FILE *filestream;
    char filename[50];

    sprintf(filename,"/sys/class/gpio/pioC%d/value",gpioid);
    if ((filestream=fopen(filename,"w"))==NULL) {
        printf("Error on setbits %d\n",gpioid);
        return -1;
    }   
    fprintf(filestream,"1");
    fclose(filestream);
    return 0;
}

/*******set pin on******************************************/
int gpioclearbits(int gpioid) 
{
    FILE *filestream;
    char filename[50];

    sprintf(filename,"/sys/class/gpio/pioC%d/value",gpioid);
    if ((filestream=fopen(filename,"w"))==NULL) {
        printf("Error on clearbits %d\n",gpioid);
        return -1;
    }   
    fprintf(filestream,"0");
    fclose(filestream);
    return 0;
}
/*******FIN control GPIO***********************************************************/
static void delay_100ms() {
    for (uint8_t i = 0; i < 10; i++) sleep(0.01);
}

/******************************************************************************/
static void oled_write_command(uint8_t command) {

    //LCD_CS = 0; 
    //gpioclearbits(SSD1325_CS);  //CHIP SELECT LOW
    //LCD_DC = 0;
    gpioclearbits(3); //DATA COMAND LOW 

    WriteSPI2(command);

    //LCD_CS = 1;
    //gpiosetbits(SSD1325_CS);   //CS UP
}

/******************************************************************************/
static void oled_write_data(uint8_t data) {

    //LCD_CS = 0;
    //gpioclearbits(SSD1325_CS);  //CHIP SELECT LOW
    //LCD_DC = 1;
     gpiosetbits(3);   //DCX UP

    WriteSPI2(data);

    //LCD_CS = 1;
    //gpiosetbits(SSD1325_CS);   //CS UP
}

/******************************************************************************/
static void oled_init(void) {

    //gpioexport(SSD1325_CS);     //EXPORT CHIPSELECT OLED
    gpioexport(SSD1325_DCX);    //EXPORT DATA COMAND OLED
    gpioexport(SSD1325_RST);    //EXPORT RESET OLED

    //gpiosetdir(SSD1325_CS,1);   //CHIP SELECT OUTPUTPIN
    gpiosetdir(3,"out");  //DATA COMAND OUTPUTPIN
    gpiosetdir(4,"out");  //RESET     OUTPUTPIN

    //LCD_DC = 0;
    gpioclearbits(3); //DATA COMAND LOW 
    //LCD_CS = 0;
    //gpioclearbits(SSD1325_CS);  //CHIP SELECT LOW
    //LCD_RS = 0;
    gpioclearbits(4); //RESET INIT
    delay_100ms(); //delay for the reset
    //LCD_RS = 1;
    gpiosetbits(4);   //RESET FINISH




    oled_write_command(SSD1305_DISPLAYOFF); //ok
    /*
    oled_write_command(SSD1305_DISPLAYOFF); //ok

    //oled_write_command(SSD1305_SETLOWCOLUMN + 0x00); // 0x00-0x0F set low column address
    //oled_write_command(SSD1305_SETHIGHCOLUMN + 0x00);// 0x10-0x1F set high column address
    // Set column Adress 34 = 0x02 low + 0x12 high

    oled_write_command(SSD1305_SETDISPLAYCLOCKDIV);
    oled_write_command(0xA0); //Set to 130Hz

    oled_write_command(SSD1305_SETMULTIPLEX);
    //oled_write_command(0x2F);                     // set for 1/48, from 16MUX to 64MUX, RESET=111111b (i.e. 64MUX)
    //oled_write_command(0x3F);
    oled_write_command(0x1F); //1/32 Duty (0x0F~0x3F)

    oled_write_command(SSD1305_SETDISPLAYOFFSET); // Set vertical shift by COM from 0~63.
    oled_write_command(0x00); // 48px by 0xC8 = (COM[N-1] - 0)
    // 0px by (0xC0 = (0 - COM[N-1]

    oled_write_command(SSD1305_SETSTARTLINE); // Set display start line (0x40 = start at line 0)
    oled_write_command(SSD1305_NORMALDISPLAY);

    oled_write_command(SSD1305_DCVOLTCONVERT);
    oled_write_command(0x8E); // 0x8E = Select external VCC supply (RESET)

    oled_write_command(SSD1305_AREACOLOR_POWERMODE);
    oled_write_command(0x00); // monochrome mode & Normal power mode
    //oled_write_command(0x05);            // monochrome mode & low power mode

    oled_write_command(SSD1305_SEGREMAP0); //Column address 0 is mapped to SEG0 (normal)
    //oled_write_command(SSD1305_SEGREMAP1); //Column address 0x83 is mapped to SEG0 (reversed)

    oled_write_command(SSD1305_COMSCANDEC); // set com output scan direction

    oled_write_command(SSD1305_SETCOMPINS);
    oled_write_command(0x12); // Disable COM Left/Right remap

    oled_write_command(SSD1305_LOOK_UP_TABLE); // Set current drive pulse width of BANK0,
    oled_write_command(0x3F); // BANK0: X[5:0] = 31? 63;
    oled_write_command(0x3F); // Color A: A[5:0] same as above (RESET = 111111b)
    oled_write_command(0x3F); // Color B: B[5:0] same as above (RESET = 111111b)
    oled_write_command(0x3F); // Color C: C[5:0] same as above (RESET = 111111b)

    oled_write_command(SSD1305_SETCONTRAST); //contrast control mode
    oled_write_command(0xAA);
    //oled_write_command(0x8F);

    oled_write_command(SSD1305_SETBRIGHTNESS); // Set Brightness For Area Color Banks
    oled_write_command(0x5F); // select 1 out of 256 brightness steps.

    oled_write_command(SSD1305_SETPRECHARGE); //Set discharge/precharge period
    oled_write_command(0xD2); // (A3 - A0 pre charge, A7 - A4 dis charge) period adjustment
    // Set Pre-Charge as 13 Clocks & Discharge as 2 Clock
    oled_write_command(SSD1305_SETVCOMDETECT);
    oled_write_command(0x34); // ~ 0.77 x VCC (RESET)

    oled_write_command(SSD1305_DISPLAYALLON_RESUME);
    oled_write_command(SSD1305_NORMALDISPLAY);

    oled_write_command(SSD1305_MEMORYMODE);
    oled_write_command(0x00); //0x00, Horizontal Addressing Mode
    //oled_write_command(0x01); //0x01, Vertical Addressing Mode
    //oled_write_command(0x02); //0x02, Page Addressing Mode (RESET)*/
     oled_write_command(SSD1325_SETCLOCK); /* set osc division */
  
     oled_write_command(0xF1); /* 145 */
     oled_write_command(SSD1325_SETMULTIPLEX ); /* multiplex ratio */
     oled_write_command(0x3f); /* duty = 1/64 */
     oled_write_command( SSD1325_SETOFFSET); /* set display offset --- */
     oled_write_command(0x4C); /* 76 */
     oled_write_command(SSD1325_SETSTARTLINE); /*set start line */
     oled_write_command(0x00); /* ------ */
     oled_write_command(SSD1325_MASTERCONFIG); /*Set Master Config DC/DC Converter*/
     oled_write_command(0x02);
     oled_write_command(SSD1325_SETREMAP); /* set segment remap------ */
     oled_write_command(0x56);
      //command(0x52);

     oled_write_command(SSD1325_SETCURRENT + 0x2); /* Set Full Current Range */
     oled_write_command(SSD1325_SETGRAYTABLE);
     oled_write_command(0x01);
     oled_write_command(0x11);
     oled_write_command(0x22);
     oled_write_command(0x32);
     oled_write_command(0x43);
     oled_write_command(0x54);
     oled_write_command(0x65);
     oled_write_command(0x76);


     oled_write_command(SSD1325_SETCONTRAST); /* set contrast current */
     oled_write_command(0x7F);  // max!
      
     oled_write_command(SSD1325_SETROWPERIOD);
     oled_write_command(0x51);
     oled_write_command(SSD1325_SETPHASELEN);
     oled_write_command(0x55);
     oled_write_command(SSD1325_SETPRECHARGECOMP);
     oled_write_command(0x02);
     oled_write_command(SSD1325_SETPRECHARGECOMPENABLE);
     oled_write_command(0x28);
     oled_write_command(SSD1325_SETVCOMLEVEL); // Set High Voltage Level of COM Pin
     oled_write_command(0x1C); //?
     oled_write_command(SSD1325_SETVSL); // set Low Voltage Level of SEG Pin 
     oled_write_command(0x0D|0x02);
      
     oled_write_command(SSD1325_NORMALDISPLAY); /* set display mode */
      /* Clear Screen */  
     //oled_write_command(0x23); /*set graphic acceleration commmand */
     //oled_write_command(SSD1325_GFXACCEL);
     //oled_write_command(SSD1325_DRAWRECT); /* draw rectangle */
     //oled_write_command(0x00); /* Gray Scale Level 1 */
     //oled_write_command(0x00); /* Gray Scale Level 3 & 2 */
     //oled_write_command(0x3f); /* Gray Scale Level 3 & 2 */
     //oled_write_command(0x3f); /* Gray Scale Level 3 & 2 */
 //oled_write_command(0x00); /* Gray Scale Level 3 & 2 */
    delay_100ms();

    //oled_clear_display();

    oled_write_command(SSD1305_DISPLAYON);
}

/******************************************************************************/
static uint_fast16_t get_buffer_position(uint8_t X, uint8_t Y, uint8_t i) {
    // GDRAM_WIDTH * (MAX_Y_CHAR - 1) = 396
    return (X * (CHAR7x5_WIDTH + 1)) + i + 396 - (Y * GDRAM_WIDTH);
}

/******************************************************************************/
void oled_buffer_putc(uint8_t character, uint8_t X, uint8_t Y, bool hL) {

    uint8_t i, c;

    if ((X >= MAX_X_CHAR) || (Y >= MAX_Y_CHAR)) return;

    for (i = 0; i < CHAR7x5_WIDTH; i++) {
        c = font[((uint_fast16_t) character * CHAR7x5_WIDTH) + i];
        if (hL) c = ~c;
        display_buffer[get_buffer_position(X, Y, i)] = c;
    }
    // Print a blank 6th column according to the highlight status
    display_buffer[get_buffer_position(X, Y, i)] = hL ? 0xFF : 0;
}

/******************************************************************************/
void oled_buffer_puts(const uint8_t* string, uint8_t X, uint8_t Y, bool hL) {

    uint8_t i;

    for (i = 0; i < strlen(string); ++i) {
        oled_buffer_putc(string[i], X + i, Y, hL);
    }
    if (hL) {
        // Fill with spaces to get the whole line colored
#if DISPLAY_WIDTH != GDRAM_WIDTH
        for (; i <= MAX_X_CHAR; i++) {
#else
        for (; i < MAX_X_CHAR; i++) {
#endif
            oled_buffer_putc(0x20, X + i, Y, true);
        }
    }
}

/******************************************************************************/
void oled_buffer_printf(uint8_t* string, uint8_t X, uint8_t Y, bool hL) {

    uint8_t i;

    for (i = 0; i < strlen(string); ++i) {
        oled_buffer_putc(string[i], X + i, Y, hL);
    }
    if (hL) {
        // Fill with spaces to get the whole line colored
#if DISPLAY_WIDTH != GDRAM_WIDTH
        for (; i <= MAX_X_CHAR; i++) {
#else
        for (; i < MAX_X_CHAR; i++) {
#endif
            oled_buffer_putc(0x20, X + i, Y, true);
        }
    }
}

/******************************************************************************/
void oled_buffer_update() {

    uint_fast16_t i;

    if (buffer_update) {
        for (i = 0; i < BUFFER_LENGTH; ++i) {
            oled_write_data(display_buffer[i]);
        }
#if BUFFER_LENGTH == 528
        for (i = 0; i < BUFFER_LENGTH; ++i) {
            oled_write_data(0);
        }
#endif
        buffer_update = false;
    }
}

int main(int argc, char const *argv[])
{
    printf("start");
    fflush(stdout);
    char spidata[] = {'a','b','c'};

    f = open("/dev/spidev32766.0","r+");
    printf("ssinit");
    fflush(stdout);
    oled_init();
    printf("eeinit");
    fflush(stdout);
    int index = 0;
    while(true)
    {
      printf("Trying to write.");
      fflush(stdout);
      oled_buffer_putc('A',0,0,true);
      oled_buffer_update();
      sleep(1);
      index++;
      //if(index > 20)
      //{
      //  break;
      //}
    }
    printf("end");
    fflush(stdout);
    return 0;
}
