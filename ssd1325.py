import time
from os import path
from font import font, lookup
import math
import spidev
import sys

SSD1325_CS	= 14#chip select display
SSD1325_DCX	= 67#data comand pin
SSD1325_RST	= 68#rst pin

DCX_GPIO = 'C3'
RST_GPIO = 'C4'

SSD1325_SETCOLADDR= 0x15
SSD1325_SETROWADDR= 0x75
SSD1325_SETCONTRAST= 0x81
SSD1325_SETCURRENT= 0x84
SSD1325_SETREMAP= 0xA0
SSD1325_set_start_line= 0xA1
SSD1325_SETOFFSET= 0xA2
SSD1325_NORMALDISPLAY= 0xA4
SSD1325_DISPLAYALLON= 0xA5
SSD1325_DISPLAYALLOFF= 0xA6
SSD1325_INVERTDISPLAY= 0xA7
SSD1325_SETMULTIPLEX= 0xA8# =
SSD1325_MASTERCONFIG= 0xAD
SSD1325_DISPLAYOFF= 0xAE#   =
SSD1325_DISPLAYON= 0xAF#	=
SSD1325_SETPRECHARGECOMPENABLE= 0xB0
SSD1325_SETPHASELEN= 0xB1
SSD1325_SETROWPERIOD= 0xB2
SSD1325_SETCLOCK= 0xB3 #   != D5
SSD1325_SETPRECHARGECOMP= 0xB4
SSD1325_SETGRAYTABLE= 0xB8
SSD1325_SETPRECHARGEVOLTAGE= 0xBC
SSD1325_SETVCOMLEVEL= 0xBE
SSD1325_SETVSL= 0xBF
SSD1325_GFXACCEL= 0x23
SSD1325_DRAWRECT= 0x24
SSD1325_COPY= 0x25
SSD1325_DISPLAYOFF		 = 0xAE # Display OFF (sleep mode)

DISPLAY_WIDTH   = 128 # X
DISPLAY_HEIGHT  = 64  # Y fix to SSD1325
PAGEMAX		 = DISPLAY_HEIGHT/8 # DISPLAY_HEIGHT/8	# 48pixel/8pixel= 6 pages
GDRAM_WIDTH	 = 128
GDRAM_HEIGHT	= 80 # Screen is 64 pixels long, but the internal RAM has 80 rows
BUFFER_LENGTH   = (DISPLAY_WIDTH * GDRAM_HEIGHT) / 2

CHAR_WIDTH = 5
CHAR_HEIGHT = 7

DISPLAY_NORMAL  = 0  # Previously 0

DISPLAY_X_MIRROR	= 1
DISPLAY_Y_MIRROR	= 2
DISPLAY_XY_MIRROR   = 3
MAX_Y_CHAR		  = DISPLAY_HEIGHT / (CHAR_HEIGHT)   # DISPLAY_HEIGHT/CHAR7x5_HEIGHT
MAX_X_CHAR		  = DISPLAY_WIDTH / (CHAR_WIDTH + 1)# DISPLAY_WIDTH/(CHAR7x5_WIDTH+1)

"""
Overflow modes:

For X, on overflow we can truncate lines, cut and continue, cut from last space and continue, or create a loop-scroll.
The latter should only work if there's no Y overflow.

Default: 0

	0: Drop overflow pixels (truncate)
	1: Continue immediately on next line.
	2: Continue from last space.
	3: Loop scroll (pending)

For Y, the options are to drop the first lines, drop the last lines or loop-scroll.

	0: Drop first lines.
	1: Drop last lines.
	2: Scroll (limited to 80 rows).

Some maxX and maxY dimensions should be set to create an extended display_buffer to hold all the data.

"""

display_buffer = [0x00] * BUFFER_LENGTH

#display = [[0x00 for x in range(DISPLAY_WIDTH)] for y in range(DISPLAY_HEIGHT)] #Initialize a DWxDH 2d array of zeros


class spibus():

	#Set the bus, device and speed.
	#For this screen, speed is 4 MHz.
	def __init__(self, bus, device):
		self.spi = spidev.SpiDev()
		self.spi.open(bus, device)
		self.spi.max_speed_hz = 4000000
		self.x_cursor = 0
		self.y_cursor = 0
		self.OVERFLOW_X_MODE = 0
		self.OVERFLOW_Y_MODE = 0

	#Allow to change DCX and RST.
	def set_gpios(self, dcx, dcx_pio, rst, rst_pio):
		global SSD1325_DCX
		global SSD1325_RST
		global DCX_GPIO
		global RST_GPIO

		SSD1325_DCX	= dcx
		SSD1325_RST	= rst

		DCX_GPIO = dcx_pio
		RST_GPIO = dcx_rst

	#Set overflow modes.
	def set_overflow_mode(self, x_mode, y_mode):
		self.OVERFLOW_X_MODE = x_mode
		self.OVERFLOW_Y_MODE = y_mode

	#Send a single byte of data.
	def send_byte(self, data):
		self.spi.write_bytes([data])

	#Delay in milliseconds.
	def delay(self, interval):
		dNum = float(interval) / 1000.0
		time.sleep(dNum)

	#Export which pin corresponds to a gpio.
	def gpio_export(self, pin, gpioid):
		if not path.isdir('/sys/class/gpio/pio' + gpioid):
			f = open("/sys/class/gpio/export","wb")
			f.write(str(pin))
			f.close()

	#Set communication direction for a gpio.
	def gpio_set_dir(self, gpioid, mode):
		f = open("/sys/class/gpio/pio" + gpioid + "/direction", "wb")
		f.write(mode)
		f.close()

	#Set high to gpio (1)
	def gpio_set_bits(self, gpioid):
		f = open("/sys/class/gpio/pio" + gpioid + "/value", "wb")
		f.write("1")
		f.close()

	#Set low to gpio (0)
	def gpio_clear_bits(self, gpioid):
		f = open("/sys/class/gpio/pio" + gpioid + "/value", "wb")
		f.write("0")
		f.close()

	#Write a single byte command:
	#To tell the screen that incoming info is a command, DCX must be set to low.
	def oled_write_command(self, command):
		self.gpio_clear_bits(DCX_GPIO)
		self.send_byte(command)

	#Write a single byte of data:
	#To tell the screen that incoming info is data, DCX must be set to high.
	def oled_write_data(self, data):
		self.gpio_set_bits(DCX_GPIO)
		self.send_byte(data)

	#Write the whole buffer to the screen.
	def spi_write_buffer(self):
		self.gpio_set_bits(DCX_GPIO)
		self.spi.write_bytes(display_buffer)

	def spi_write_80(self):
		self.gpio_set_bits(DCX_GPIO)
		self.spi.write_bytes(display_buffer[0:4096])
		self.spi.write_bytes(display_buffer[4096:5120])
		self.gpio_clear_bits(DCX_GPIO)

	#Clear the screen by setting the buffer to 0x00s and writing it.
	def oled_clear_display(self):
		display_buffer = [0x00] * BUFFER_LENGTH
		self.spi_write_buffer()

	#Get the bit at bitPos from byte.
	def get_bit(self, byte, bitPos):
		return ((byte&(1<<bitPos))!=0);

	#Write a paragraph starting from the cursor's position.
	#Change line whenever \n is found.
	def write_paragraph(self, paragraph):
		words = paragraph.split("\n")
		for word in words:
			self.x_cursor = 0
			if self.y_cursor > 0:
				self.y_cursor += 3
			self.write_string(self.x_cursor, self.y_cursor, word)

	#Write a whole string by writing every char.
	#Handle overflows.
	def write_string(self, X, Y, word):

		w_length = len(word)
		start_pos = X

		if self.OVERFLOW_X_MODE == 0:

			for k in range(0, w_length):
				self.write_char(start_pos, Y, word[k])
				start_pos += CHAR_WIDTH + 1

		elif self.OVERFLOW_X_MODE == 1:

			current_y = Y
			for k in range(0, w_length):
				self.write_char(start_pos, current_y, word[k])
				start_pos += CHAR_WIDTH + 1
				if start_pos > DISPLAY_WIDTH - CHAR_WIDTH - 1:

					start_pos = X
					current_y += CHAR_HEIGHT + 2


		elif self.OVERFLOW_X_MODE == 2:
			current_y = Y
			current_x_space = -1
			current_k_space = -1
			k = 0
			while k < w_length:
				if word[k] == " ":
					current_x_space = start_pos
					current_k_space = k
				self.write_char(start_pos, current_y, word[k])
				start_pos += CHAR_WIDTH + 1
				if start_pos > DISPLAY_WIDTH - CHAR_WIDTH - 1:
					start_pos = X
					current_y += CHAR_HEIGHT + 2
					#Check if there was a space and if current or next chars are spaces.
					if current_x_space > 0:
						if word[k] != " " and (k+1) < w_length and word[k+1] != " ":

							#Go back to the space, clear extra chars and continue writing on next line from word[k+1].
							#Also, uncheck current_x_space

							k = current_k_space + 1

							#Clear everything from current_x_space forward on previous line:
							previous_y = current_y - (CHAR_HEIGHT + 2)
							while current_x_space < DISPLAY_WIDTH:

								self.write_char(current_x_space, previous_y, ' ')
								current_x_space += CHAR_WIDTH + 1

							current_x_space = -1

						else:
							k += 1
					else:
						k += 1
				else:
					k += 1

	def write_char(self, X, Y, ch):
		if ((X >= DISPLAY_WIDTH - 5) or (Y >= GDRAM_HEIGHT - 7)):
			return

		for i in range (0, CHAR_WIDTH):
			c = font[lookup[ch] * CHAR_WIDTH + i]

			for j in range(0, 8):
				if self.get_bit(c, j) == 1:
					self.write_pixel(X+i, Y+j, 'F')
				else:
					self.write_pixel(X+i, Y+j, '0')
				j += 1

			i += 1


	#Given coordinates X and Y, and a gray level between 0..15 given by scale, write
	#the pixel in that position with the given tone.
	def write_pixel(self, X, Y, scale):
		if X < 0 or X > DISPLAY_WIDTH or Y < 0 or Y > GDRAM_HEIGHT:
			return

		x = float(X)
		y = float(Y)

		#Every pixel consists of 4 bits (16 gray levels) and is set in the 4 most representative bits
		#for odd X, and the less representative bits for even X.
		#Thus, buffer position should be (x + y*width)/2.
		pos = int(math.floor((x/2 + (y*DISPLAY_WIDTH)/2)))

		#We assume X even and set the lowest bits [3:0].
		#If X is odd, we set the highest bits [7:4].
		#The remaining half is set to 0 so it doesn't change anything when an OR operation is executed.
		set_str = '0x0' + scale
		if X%2 == 1:
			set_str = '0x' + scale + '0'

		ch_byte = int(set_str, 16)

		#As we want to write, we first must clear the position:
		clStr = '0xF0'
		if X%2 == 1:
			clStr = '0x0F'

		clByte = int(clStr, 16)

		#An AND will clear the relevant half.
		display_buffer[pos] &= clByte

		#A simple OR with the current byte will set the relevant half.
		display_buffer[pos] |= ch_byte

		self.x_cursor = X
		self.y_cursor = Y

	#Sets start line (given by byte) for scrolling.
	def set_start_line(self, byte):
		self.oled_write_command(0xA1)
		self.oled_write_command(byte)


	#Scroll an amount of rows vertically in a given direction every "interval" milliseconds.
	def set_vertical_scroll(self, direction, rows, interval, delayTime):
		#Directions:
		# 0x00: Bottom to top.
		# 0x01: Top to bottom.
		if direction == 0x00:
			i = 0
			while i < 64:
				self.set_start_line(i)
				for j in range(0, interval):
					self.delay(delayTime)
				i += rows
		else:
			i = 0
			while i < 64:
				self.set_start_line(64-i)
				for j in range(0, interval):
					self.delay(delayTime)
				i += rows
		self.set_start_line(0x00)

	def set_horizontal_scroll(self, cols, rows, interval, delayTime):
		self.ga_option(0x03)
		self.oled_write_command(0x26)
		self.oled_write_command(cols)
		self.oled_write_command(rows)
		self.oled_write_command(interval)
		self.oled_write_command(0x2F)
		self.delay(delayTime)

	def ga_option(self, opt):
		self.oled_write_command(0x23)
		self.oled_write_command(opt)

	#Stop scroll mode.
	def stop_scroll(self):
		self.oled_write_command(0x2E)

	#Initialization commands for ssd1325 oled screen. Please check datasheet for details.
	def oled_init(self):

		self.gpio_export(SSD1325_DCX, DCX_GPIO);	#EXPORT DATA COMAND OLED
		self.gpio_export(SSD1325_RST, RST_GPIO);	#EXPORT RESET OLED
		self.gpio_set_dir(DCX_GPIO,"out");  #DATA COMAND OUTPUTPIN
		self.gpio_set_dir(RST_GPIO,"out");  #RESET	 OUTPUTPIN

		#LCD_DC = 0;
		self.gpio_clear_bits(DCX_GPIO); #DATA COMAND LOW 
	  	#LCD_CS = 0;
		#gpio_clear_bits(SSD1325_CS);  #CHIP SELECT LOW
		#LCD_RS = 0;
		self.gpio_clear_bits(RST_GPIO); #RESET INIT
		self.delay(100); #delay for the reset
		#LCD_RS = 1;
		self.gpio_set_bits(RST_GPIO);   #RESET FINISH
		self.oled_write_command(SSD1325_DISPLAYOFF); #ok
		self.oled_write_command(SSD1325_SETCLOCK); # set osc division */
		self.oled_write_command(0xF1); # 145 */
		self.oled_write_command(SSD1325_SETMULTIPLEX ); # multiplex ratio */
		self.oled_write_command(0x3f); # duty = 1/64 */
		self.oled_write_command( SSD1325_SETOFFSET); # set display offset --- */
		self.oled_write_command(0x4C); # 76 */
		self.oled_write_command(SSD1325_set_start_line); #set start line */
		self.oled_write_command(0x00); # ------ */
		self.oled_write_command(SSD1325_MASTERCONFIG); #Set Master Config DC/DC Converter*/
		self.oled_write_command(0x02);
		self.oled_write_command(SSD1325_SETREMAP); # set segment remap------ */
		self.oled_write_command(0x50);
		#command(0x52);

		self.oled_write_command(SSD1325_SETCURRENT + 0x2); # Set Full Current Range */
		self.oled_write_command(SSD1325_SETGRAYTABLE);
		self.oled_write_command(0x01);
		self.oled_write_command(0x11);
		self.oled_write_command(0x22);
		self.oled_write_command(0x32);
		self.oled_write_command(0x43);
		self.oled_write_command(0x54);
		self.oled_write_command(0x65);
		self.oled_write_command(0x76);


		self.oled_write_command(SSD1325_SETCONTRAST); # set contrast current */
		self.oled_write_command(0xFF);  # max!

		self.oled_write_command(SSD1325_SETROWPERIOD);
		self.oled_write_command(0x51);
		self.oled_write_command(SSD1325_SETPHASELEN);
		self.oled_write_command(0x55);
		self.oled_write_command(SSD1325_SETPRECHARGECOMP);
		self.oled_write_command(0x02);
		self.oled_write_command(SSD1325_SETPRECHARGECOMPENABLE);
		self.oled_write_command(0x28);
		self.oled_write_command(SSD1325_SETVCOMLEVEL); # Set High Voltage Level of COM Pin
		self.oled_write_command(0x1C); #?
		self.oled_write_command(SSD1325_SETVSL); # set Low Voltage Level of SEG Pin 
		self.oled_write_command(0x0F);
		  
		self.oled_write_command(SSD1325_NORMALDISPLAY); # set display mode */
		self.delay(100);

		self.oled_write_command(SSD1325_DISPLAYON);