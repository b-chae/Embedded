#include "driver_header.h"


void fnd_write(int n, int index){
	char value[4];
	int i;
	for(i=0; i<4; i++) value[i] = 0;
	value[index] = n;
	unsigned short int value_short;
	value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);
}

void dot_write(int n){
	int i;
	if(n == -1){
		for(i=0; i<10; i++){
			outw(fpga_set_blank[i], (unsigned int)iom_fpga_dot_addr + 2*i);
		}
		return;
	}
	for(i=0; i<10; i++){
		outw(fpga_number[n][i], (unsigned int)iom_fpga_dot_addr + 2*i);
	}
}

void led_write(unsigned char n){
	
	unsigned short _s_value = 0;
	
	switch(n){
		case 1 : _s_value = 128; break;
		case 2 : _s_value = 64; break;
		case 3 : _s_value = 32; break;
		case 4 : _s_value = 16; break;
		case 5 : _s_value = 8; break;
		case 6 : _s_value = 4; break;
		case 7 : _s_value = 2; break;
		case 8 : _s_value = 1; break;
	}

	outw(_s_value, (unsigned int)iom_fpga_led_addr);
}

void text_write(int l_index, int r_index) 
{
	int i;

	unsigned char value[32];
   	unsigned short int _s_value = 0;
	
	for(i=0; i<l_index; i++){
		value[i] = ' ';
	}
	for(i=l_index; i<l_index+STRLEN_STUDENT_NUMBER; i++){
		value[i] = student_number[i-l_index];
	}
	for(i=i; i<16; i++){
		value[i] = ' ';
	}
	
	for(i=16; i<r_index+16; i++){
		value[i] = ' ';
	}
	for(i=i; i<r_index+16+STRLEN_MY_NAME; i++){
		value[i] = my_name[i-r_index-16];
	}
	for(i=i; i<32; i++){
		value[i] = ' ';
	}

	for(i=0;i<32;i++)
    {
        _s_value = (value[i] & 0xFF) << 8 | value[i + 1] & 0xFF;
	outw(_s_value,(unsigned int)iom_fpga_text_lcd_addr+i);
        i++;
    }
}

void clear_device(void){
	int i;
	unsigned short _s_value;
	fnd_write(0, 0);
	dot_write(-1);
	led_write(0);
	for(i=0;i<32;i++)
    {
        _s_value = (' ' & 0xFF) << 8 | ' ' & 0xFF;
	outw(_s_value,(unsigned int)iom_fpga_text_lcd_addr+i);
        i++;
    }
}