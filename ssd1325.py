import posix
import struct
from ctypes import addressof, create_string_buffer, sizeof, string_at
from fcntl import ioctl
from spi_ctypes import *
import time
from os import path
from font import font, lookup
import math
import spidev
import sys

SSD1325_CS	= 14#chip select display
SSD1325_DCX	= 67#data comand pin
SSD1325_RST	= 68#rst pin 

SSD1325_SETCOLADDR= 0x15
SSD1325_SETROWADDR= 0x75
SSD1325_SETCONTRAST= 0x81
SSD1325_SETCURRENT= 0x84
SSD1325_SETREMAP= 0xA0
SSD1325_SETSTARTLINE= 0xA1
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
GDRAM_HEIGHT	= 64 # Is really 64
BUFFER_LENGTH   = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2

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

For X, on overflow we can truncate lines, cut and continue, cut afrom last space and continue, or create a loop-scroll.
The latter should only work if there's no Y overflow.

For Y, the options are to drop the first lines, drop the last lines or loop-scroll.

Some maxX and maxY dimensions should be set to create an extended display_buffer to hold all the data.

"""

OVERFLOW_X_MODE = 0
OVERFLOW_Y_MODE = 0

display_buffer = [0x00] * BUFFER_LENGTH
buffer_update = True

#display = [[0x00 for x in range(DISPLAY_WIDTH)] for y in range(DISPLAY_HEIGHT)] #Initialize a DWxDH 2d array of zeros


class spibus():
	fd=None
	write_buffer=create_string_buffer(BUFFER_LENGTH)

	def __init__(self,bus,device):
		self.spi = spidev.SpiDev()
		self.spi.open(bus, device)
		self.spi.max_speed_hz = 4000000

	def send(self,len):
		self.ioctl_arg.len=len
		ioctl(self.fd, SPI_IOC_MESSAGE(1),addressof(self.ioctl_arg))

	def simplesend(self, data):
		self.spi.writebytes([data])

	def data(self, bytes):
		self.gpioclearbits(3)
		self.spi.writebytes(bytes)
		self.gpiosetbits(3)

	def delay100ms(self):
		time.sleep(0.1)

	def gpioexport(self, pin, gpioid):
		if not path.isdir('/sys/class/gpio/pioC' + str(gpioid)):
			f = open("/sys/class/gpio/export","wb")
			f.write(str(pin))
			f.close()

	def gpiosetdir(self, gpioid, mode):
		f = open("/sys/class/gpio/pioC" + str(gpioid) + "/direction", "wb")
		f.write(mode)
		f.close()

	def gpiosetbits(self, gpioid):
		f = open("/sys/class/gpio/pioC" + str(gpioid) + "/value", "wb")
		f.write("1")
		f.close()

	def gpioclearbits(self, gpioid):
		f = open("/sys/class/gpio/pioC" + str(gpioid) + "/value", "wb")
		f.write("0")
		f.close()

	def oled_write_command(self, command):
		#print "Writing command: {}".format(command)
		self.gpioclearbits(3)
		self.simplesend(command)

	def oled_write_data(self, data):
		self.gpiosetbits(3)
		self.simplesend(data)

	def spi_write_buffer(self):
		self.gpiosetbits(3)
		self.spi.writebytes(display_buffer)

	def oled_clear_display(self):
		for i in range(0, BUFFER_LENGTH):
			self.oled_write_data(0x00)

	def oled_buffer_update(self):
		print "Update"
		self.spi_write_buffer()
		#self.spi.writebytes(display_buffer)
		#for i in range(0, BUFFER_LENGTH):
		#	self.oled_write_data(display_buffer[i])

	def get_bit(self, byteval,idx):
		return ((byteval&(1<<idx))!=0);

	def write_string(self, X, Y, word):

		wLength = len(word)

		#Should handle overflow
		#if(X + (wLength * CHAR_WIDTH + 1) > DISPLAY_WIDTH):
		#	if OVERFLOW_X_MODE == 0:

		#	elif: OVERFLOW_X_MODE == 1:

		#	else:


		startPos = X

		for k in range(0, wLength):
			self.write_char(startPos, Y, word[k])
			startPos += CHAR_WIDTH + 1


	def write_char(self, X, Y, ch):
		if ((X >= DISPLAY_WIDTH - 5) or (Y >= DISPLAY_HEIGHT - 7)):
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


	#Working, it writes a pixel with the given scale
	def write_pixel(self, X, Y, scale):
		if X < 0 or X > DISPLAY_WIDTH or Y < 0 or Y > DISPLAY_HEIGHT:
			return

		x = float(X)
		y = float(Y)

		pos = int(math.floor((x/2 + (y*DISPLAY_WIDTH)/2)))

		#Bits [7:4] of the byte in pos must change to scale.
		#Scale is given as 0..F (0 to 15)
		bScale = '0x0' + scale
		if X%2 == 1:
			bScale = '0x' + scale + '0'

		chByte = int(bScale, 16)

		display_buffer[pos] |= chByte



	def oled_init(self):
		self.gpioexport(67, 3);	#EXPORT DATA COMAND OLED
		self.gpioexport(68, 4);	#EXPORT RESET OLED
		self.gpiosetdir(3,"out");  #DATA COMAND OUTPUTPIN
		self.gpiosetdir(4,"out");  #RESET	 OUTPUTPIN

		#LCD_DC = 0;
		self.gpioclearbits(3); #DATA COMAND LOW 
	  	#LCD_CS = 0;
		#gpioclearbits(SSD1325_CS);  #CHIP SELECT LOW
		#LCD_RS = 0;
		self.gpioclearbits(4); #RESET INIT
		self.delay100ms(); #delay for the reset
		#LCD_RS = 1;
		self.gpiosetbits(4);   #RESET FINISH
		self.oled_write_command(SSD1325_DISPLAYOFF); #ok
		self.oled_write_command(SSD1325_SETCLOCK); # set osc division */
		self.oled_write_command(0xF1); # 145 */
		self.oled_write_command(SSD1325_SETMULTIPLEX ); # multiplex ratio */
		self.oled_write_command(0x3f); # duty = 1/64 */
		self.oled_write_command( SSD1325_SETOFFSET); # set display offset --- */
		self.oled_write_command(0x4C); # 76 */
		self.oled_write_command(SSD1325_SETSTARTLINE); #set start line */
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
		self.delay100ms();

		self.oled_write_command(SSD1325_DISPLAYON);



#Open the SPI bus 0
spibus0 = spibus(32766,0)

#Send two characters
#spibus0.write_buffer[0]=chr(0x55)
#spibus0.write_buffer[1]=chr(0xAA)

#spibus0.send(2)

#Shows the 2 byte received in full duplex in hex format
#print hex(ord(spibus0.read_buffer[0]))
#print hex(ord(spibus0.read_buffer[1]))


#Now test the OLED screen
spibus0.oled_init()

spibus0.write_pixel(2,2,'F')
spibus0.write_pixel(2,3,'F')
spibus0.write_pixel(2,4,'F')
spibus0.write_pixel(2,5,'F')
spibus0.write_pixel(3,2,'F')
spibus0.write_pixel(3,3,'7')
spibus0.write_pixel(3,4,'7')
spibus0.write_pixel(3,5,'F')
spibus0.write_pixel(4,2,'F')
spibus0.write_pixel(4,3,'7')
spibus0.write_pixel(4,4,'7')
spibus0.write_pixel(4,5,'F')
spibus0.write_pixel(5,2,'F')
spibus0.write_pixel(5,3,'F')
spibus0.write_pixel(5,4,'F')
spibus0.write_pixel(5,5,'F')

if len(sys.argv) > 1:
	spibus0.write_string(2, 12, sys.argv[1])
spibus0.oled_buffer_update()