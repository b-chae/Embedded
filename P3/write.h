void fnd_write(int n);

/* 출력 초기화 */
void clear_device(void){
	fnd_write(0); //FND 0000 출력
}

/* fnd 출력 
 */
void fnd_write(int n){
	char value[4];
	unsigned short int value_short;
	
	value[0] = n/10;
	value[1] = n/100%10;
	value[2] = n%100/10;
	value[3] = n%10;
	
	value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
	outw(value_short,(unsigned int)iom_fpga_fnd_addr);
}
