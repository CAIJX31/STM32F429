#include "ft5206.h"
#include "ctiic.h"

//$)AOrFT5206P4HkR;4NJ}>]
//reg:$)AFpJ<<D4fFw5XV7
//buf:$)AJ}>];:;:4fGx
//len:$)AP4J}>]3$6H
//$)A75;XV5:0,3I9&;1,J'0\.
u8 FT5206_WR_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i;
	u8 ret=0;
	CT_IIC_Start();	 
	CT_IIC_Send_Byte(FT_CMD_WR);	//$)A7"KMP4C|An 	 
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//$)A7"KM5M8N;5XV7
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//$)A7"J}>]
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
  CT_IIC_Stop();					//$)A2zIzR;8vM#V9Lu<~	    
	return ret; 
}


//$)A4SFT52066A3vR;4NJ}>]
//reg:$)AFpJ<<D4fFw5XV7
//buf:$)AJ}>];:;:4fGx
//len:$)A6AJ}>]3$6H			  
void FT5206_RD_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(FT_CMD_WR);   	//$)A7"KMP4C|An 	 
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//$)A7"KM5M8N;5XV7
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(FT_CMD_RD);   	//$)A7"KM6AC|An		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //$)A7"J}>]	  
	} 
  CT_IIC_Stop();//$)A2zIzR;8vM#V9Lu<~     
} 


//$)A3uJ<;/FT52064%C~FA
//$)A75;XV5:0,3uJ<;/3I9&;1,3uJ<;/J'0\ 
u8 FT5206_Init(void)
{
	u8 temp[2]; 
	GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOH_CLK_ENABLE();			//$)A?*FtGPIOHJ1VS
	__HAL_RCC_GPIOI_CLK_ENABLE();			//$)A?*FtGPIOIJ1VS
							
	//PH7($)A51SPJV04OBJ1,R;V1Jd3v5M5gF=,7qTrR;V1Jd3v8_5gF=)
	GPIO_Initure.Pin=GPIO_PIN_7;            //PH7
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //$)AJdHk
	GPIO_Initure.Pull=GPIO_PULLUP;          //$)AIO@-
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //$)A8_KY
	HAL_GPIO_Init(GPIOH,&GPIO_Initure);     //$)A3uJ<;/
					
	//PI8
	GPIO_Initure.Pin=GPIO_PIN_8;            //PI8
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //$)AMFMlJd3v
	HAL_GPIO_Init(GPIOI,&GPIO_Initure);     //$)A3uJ<;/
			
	CT_IIC_Init();      	//$)A3uJ<;/5gH]FA5DI2CW\O_  
	FT_RST=1;
	delay_ms(120);			//$)AVAIYQSJ1100ms,6`<S20ms
	FT_RST=0;				//$)A84N;
	delay_ms(20);
	FT_RST=1;				//$)AJM7E84N;		    
	delay_ms(100);  //$)AUb@oVAIYPhR*QSJ1100ms,7qTr3uJ<;/;aJ'0\5D 	
	temp[0]=0;
	FT5206_WR_Reg(FT_DEVIDE_MODE,temp,1);	//$)A=xHkU}3#2YWwD#J= 
	FT5206_WR_Reg(FT_ID_G_MODE,temp,1);		//$)A2iQ/D#J= 
	temp[0]=22;								//$)A4%C~SPP'V5#,22#,T=P!T=AiCt	
	FT5206_WR_Reg(FT_ID_G_THGROUP,temp,1);	//$)AIhVC4%C~SPP'V5
	temp[0]=12;								//$)A<$;nV\FZ#,2;D\P!SZ12#,Wn4s14
	FT5206_WR_Reg(FT_ID_G_PERIODACTIVE,temp,1); 
	//$)A6AH!0f1>:E#,2N?<V5#:0x3003
	FT5206_RD_Reg(FT_ID_G_LIB_VERSION,&temp[0],2);  
	if((temp[0]==0X30&&temp[1]==0X03)||temp[1]==0X01||temp[1]==0X02)//$)A0f1>:0X3003/0X0001/0X0002
	{
		return 0;
	} 
	return 1;
}


