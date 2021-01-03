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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);     //��ʼ����ʱ����
	uart_init(115200);	//��ʼ�����ڲ�����Ϊ115200
	LED_Init();					//��ʼ��LED 
	KEY_Init(); 				//������ʼ�� 
  my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ��	

	while(SD_Init())//��ⲻ��SD��
	{
		delay_ms(500);					
		LED0=!LED0;//DS0��˸
	}
  
	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 				
	while(FR_OK != f_mount(fs[0],"0:",1)){//����SD�� 
		delay_ms(200);
	}
 	/*res=f_mount(fs[1],"1:",1); 				//����FLASH.	
	if(res==0X0D){//FLASH����,FAT�ļ�ϵͳ����,���¸�ʽ��FLASH
		res=f_mkfs("1:",1,4096);//��ʽ��FLASH,1,�̷�;1,����Ҫ������,8������Ϊ1����
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:ALIENTEK");	//����Flash���̵�����Ϊ��ALIENTEK
		}else{
			printf("format SD failed !");
		}
		delay_ms(1000);
	}*/												    	  
	while(exf_getfree("0",&total,&free)){	//�õ�SD������������ʣ������
		delay_ms(200);
		LED0=!LED0;//DS0��˸
	}	
	res=f_open(file,"2021-01-04.txt",FA_OPEN_ALWAYS|FA_WRITE);
	if(FR_OK != res){
			printf("\r\n�����ļ�ʧ�� !\r\n");
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

