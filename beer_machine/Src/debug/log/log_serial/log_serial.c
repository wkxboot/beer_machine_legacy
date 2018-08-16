#include "stdarg.h"
#include "stdint.h"
#include "serial.h"
#include "log_serial.h"


extern serial_hal_driver_t log_serial_driver;
int  log_serial_handle;




#define putchar(c)    c

static void printchar(char **str, int c)
{
	//extern int putchar(int c);
	
	if (str) {
		**str = (char)c;
		++(*str);
	}
	else
	{ 
		(void)putchar(c);
	}
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = (unsigned int)i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = (unsigned int)-i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = (unsigned int)u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = (char)(t + '0');
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print( char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

int log_serial_sprintf(char *out, const char *format, ...)
{
   va_list args;      
   va_start( args, format );
   return print( &out, format, args );
}


int log_serial_snprintf( char *buf, unsigned int count, const char *format, ... )
{
  va_list args;       
  (void ) count;       
  va_start( args, format );
  return print( &buf, format, args );
}


int log_serial_printf(const char *format, ...)
{
   va_list args;   
   uint16_t cnt,free_size;
   
   static char buffer[LOG_SERIAL_PRINTF_BUFFER_SIZE];  
   char *p = buffer;
   va_start( args, format );
   
   cnt = print(&p, format, args );
   free_size = serial_complete(log_serial_handle,0);
   if(free_size >= cnt){
   serial_write(log_serial_handle,(uint8_t *)buffer,cnt);
   }else{
   cnt =0;
   }
   
  return cnt;  
}


int log_serial_init()
{
int result=0;
result = serial_create(&log_serial_handle,LOG_SERIAL_RX_BUFFER_SIZE,LOG_SERIAL_TX_BUFFER_SIZE);
serial_register_hal_driver(log_serial_handle,&log_serial_driver);
serial_open(log_serial_handle,LOG_SERIAL_PORT,LOG_SERIAL_BAUDRATE,LOG_SERIAL_DATA_BITS,LOG_SERIAL_STOP_BITS);
return result;
}

int log_serial_read(uint8_t *buffer,uint16_t cnt)
{
 uint16_t read_cnt;
 read_cnt = serial_read(log_serial_handle,buffer,cnt);
 return read_cnt;
}






