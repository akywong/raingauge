#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "spi.h"
#include "w25qxx.h"
#include "key.h"  
#include "sram.h"   
#include "malloc.h" 
#include "usmart.h"
#include "sdio_sdcard.h"    
#include "malloc.h"   
#include "ff.h"  
#include "exfuns.h" 
#include "ad7767.h"

#define SYNC_COUNT  1000
#define READ_COUNT   100000
 
/*struct fs_status{
	FATFS fs;
	FIL   fsrc;
	char  fpath[32];
	FILINFO finfo;
	uint8_t file_flag;
	uint32_t sd_cap;
	uint32_t file_id;
	uint32_t line_num;
};
struct fs_status cur;*/
	
int main(void)
{ 
	u32 total,free;
	u8 res = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);     //初始化延时函数
	uart_init(115200);	//初始化串口波特率为115200
	LED_Init();					//初始化LED 
	KEY_Init(); 				//按键初始化 
  my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池	

	while(SD_Init())//检测不到SD卡
	{
		delay_ms(500);					
		LED0=!LED0;//DS0闪烁
	}
  
	exfuns_init();							//为fatfs相关变量申请内存				 				
	while(FR_OK != f_mount(fs[0],"0:",1)){//挂载SD卡 
		delay_ms(200);
	}
 	/*res=f_mount(fs[1],"1:",1); 				//挂载FLASH.	
	if(res==0X0D){//FLASH磁盘,FAT文件系统错误,重新格式化FLASH
		res=f_mkfs("1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:ALIENTEK");	//设置Flash磁盘的名字为：ALIENTEK
		}else{
			printf("format SD failed !");
		}
		delay_ms(1000);
	}*/												    	  
	while(exf_getfree("0",&total,&free)){	//得到SD卡的总容量和剩余容量
		delay_ms(200);
		LED0=!LED0;//DS0闪烁
	}	
	res=f_open(file,"2021-01-04.txt",FA_OPEN_ALWAYS|FA_WRITE);
	if(FR_OK != res){
			printf("\r\n创建文件失败 !\r\n");
		  return -1;
	}
	f_lseek(file,file->fsize);
	if(1){
		int ret;
		int value;
		int len;
		char buf[32];
		UINT count;
		int write_count=0;
		int remain_len = READ_COUNT;
		
		ad7767_init() ;
		ret = ad7767_powerup();
		if(ret==0){
			printf("ad7767 powerup failed !\n");
		}
		delay_ms(500);
		while(remain_len--){
			ret = ad7767_read_data(&value);
			//write data to sd
			len = sprintf(buf,"%d,%d\n",value,ret);
			ret = f_write(file,buf,len,&count);
			if(FR_OK != ret){
				f_close(file);//FR_OK
				printf("write file failed \n");
			}else{
				write_count++;
				if(write_count==SYNC_COUNT){
					f_sync(file);
					write_count=0;
				}
			}
		}
		f_close(file);
		printf("read finished \n");
		return 0;
	}
}

