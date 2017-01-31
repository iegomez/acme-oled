# acme-oled

## Summary
This library is a port of other libraries written in C and Python for this type of screen. It adds some funcionalities not present in Python simple libraries, and also ports some present on C libraries. More important, almost all libraries target the Raspberry Pi, Arduino or even BeagleBone Black, while there seems to be a lack of support for Acme Systems boards. This library targets the Aria G25 specifically, but can be easily adapted to any other (not only Acme ones) by changing some settings.

## Example usage:

Create a new bus: 
```python
spibus0 = spibus(32766,0)
```

Initialize the screen:
```python
spibus0.oled_init()
```

Set overflow modes to control how to present long lines.
```python
spibus0.setOverflowMode(1, 1)
```

Write a pixel:
```python
spibus0.write_pixel(X, Y, grayLevel)
```

Write a char:
```python
spibus0.write_char(X, Y, grayLevel)
```

Write a string:
```python
spibus0.write_string(X, Y, string)
```

Write a paragraph separated by "\n":
```python
text = "This is the first line of the paragraph.\nThis is the second one."
spibus0.write_paragraph(text)
```

Update display:
```python
spibus0.spi_write_80()
```

Scroll if necessary:
```python
if spibus0.yCursor > 64:
	spibus0.setVerticalScroll(0x00, 3, 2, 200)
```
