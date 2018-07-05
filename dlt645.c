#include <stdlib.h>
#include <stdio.h> /* Standard input/output definitions */
#include <string.h> /* String function definitions */
#include <time.h>
#include "gateway.h"
#include "comm.h"
#include "dlt645.h"

/*       DLT645 ֡�ṹ         */

#define DLT645_START  0    /*֡��ʼλ*/
#define DLT645_ADDR 1     /*��ַ��*/
#define DLT645_DSTART 7   /*������ʼλ*/
#define DLT645_CTRL 8      /*������λ*/
#define DLT645_DLEN 9	       /*���ݳ���λ*/
#define DLT645_DATA 10         /*����λ*/
#define DLT645_CS            /*У��λ*/
#define DLT645_END           /*֡����λ*/

#define DLT645_TRIP_STATE_BYTE (DLT645_DATA + 4)         /*����λ*/

#define TRIP_CMD_MAX 6

//�����룺N2
#define TRIP_N2 0x33

/*       DLT645 ֡����         */
#define DLT645_FRAME_START  0x68
#define DLT645_FRAME_END  0x16
#define DLT645_FRAME_READ_DATA 0x11
#define DLT645_C_OK	0x91
#define DLT645_C_ERRO 0xd1
#define DLT645_DATA_OFFSET 0x33

const char PAP1P2P3[4] = {0x35,0x33,0x33,0x33}; //����
const char C0C1C2C3[4] = {0x34,0x33,0x33,0x33}; //�����ߴ���
const char TRIP_N1[TRIP_CMD_MAX+1] = {0x4d,0x4e,0x4f,0x5d,0x5e,0x6d,0x6e};//�����룺N1  {�����룺����բ������������}



/*       DLT645 ���ݱ�ʶ��         */
const char DLT645_Data_Mark[ELECT_DATA_FRAME_TYPE_MAX][4] =
{
	{0x33,0x33,0x34,0x33}, /*0 ENERGY_FORWARDA */
	{0x33,0x33,0x35,0x33}, /*1 ENERGY_BACKWARDA*/
	{0x33,0x34,0x34,0x35}, /*2 VOLTAGE;        */
	{0x33,0x34,0x35,0x35}, /*3 CURRENT;	      */
	{0x33,0x33,0x36,0x35}, /*4 POWER_ACT;      */
	{0x33,0x33,0x37,0x35}, /*5 POWER_REACT;	  */
	{0x33,0x33,0x38,0x35}, /*6 FACTOR;         */
	{0x35,0x33,0xb3,0x35}, /*7 FREQUENCY;     */
	{0x35,0x33,0x33,0x39}, /*8 LOADRECORD;     */
	{0x36,0x38,0x33,0x37} /*9 READ_TRIP       */
};

const char DLT645_BroadCast_Addr[6] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};/*�㲥��ַ����*/

const char DLT645_CTRL_CODE_ENERGY = 0x11;/*�����룺�����ܱ�����*/

const char DLT645_CTRL_CODE_TRIP = 0x1C;/*�����룺��բ����բ*/

void get_load_record(char buff[],int len, elect_meter_t3 *meter)
{
	int num = 0;
	meter->type = ElectricEnergyMeter;
	meter->DateRealTime[0] = dlt645_data_to_int32(&buff[5],1); /*��*/
	meter->DateRealTime[1] = dlt645_data_to_int32(&buff[4],1); /*��*/
	meter->DateRealTime[2] = dlt645_data_to_int32(&buff[3],1); /*��*/
	meter->DateRealTime[3] = dlt645_data_to_int32(&buff[2],1); /*ʱ*/
	meter->DateRealTime[4] = dlt645_data_to_int32(&buff[1],1); /*��*/
	meter->DateRealTime[5] = dlt645_data_to_int32(&buff[0],1); /*��*/
	meter->DateRealTime[5] = '\0';
	num = 6;
	if(buff[num] != 0xAA)
		{
			meter->VoltageA = dlt645_data_to_int32(&buff[num],2); /*��ѹ*/
			num += 2;
			meter->VoltageB = dlt645_data_to_int32(&buff[num],2); /*��ѹ*/
			num += 2;
			meter->VoltageC = dlt645_data_to_int32(&buff[num],2); /*��ѹ*/
			num += 2;
			
			meter->CurrentA = dlt645_data_to_int32(&buff[num],3); /*��ѹ*/
			num += 3;
			meter->CurrentB = dlt645_data_to_int32(&buff[num],3); /*��ѹ*/
			num += 3;
			meter->CurrentC = dlt645_data_to_int32(&buff[num],3); /*��ѹ*/
			num += 3;
			
			meter->Frequency = dlt645_data_to_int32(&buff[num],2); /*Ƶ��*/
			num += 2;
		}
	else
		num++;	
	if(buff[num] != 0xAA)
		{
			meter->ActPower = dlt645_data_to_int32(&buff[num],3); /*    ���й�����*/
			num += 3;
			meter->ActPowerA = dlt645_data_to_int32(&buff[num],3); /*A�� ���й�����*/
			num += 3;
			meter->ActPowerB = dlt645_data_to_int32(&buff[num],3); /*B�� ���й�����*/
			num += 3;
			meter->ActPowerC = dlt645_data_to_int32(&buff[num],3); /*C�� ���й�����*/
			num += 3;
			
			meter->ReactPower = dlt645_data_to_int32(&buff[num],3); /*    ���޹�����*/
			num += 3;
			meter->ReactPowerA = dlt645_data_to_int32(&buff[num],3); /*A�� ���޹�����*/
			num += 3;
			meter->ReactPowerB = dlt645_data_to_int32(&buff[num],3); /*B�� ���޹�����*/
			num += 3;
			meter->ReactPowerC = dlt645_data_to_int32(&buff[num],3); /*C�� ���޹�����*/
			num += 3;
		}
	else
		num++;
			
	if(buff[num] != 0xAA)
		{
			meter->Factor = dlt645_data_to_int32(&buff[num],2); /*   ��������*/
			num += 2;                                          
			meter->FactorA = dlt645_data_to_int32(&buff[num],2); /*A�� ��������*/
			num += 2;                                          
			meter->FactorB = dlt645_data_to_int32(&buff[num],2); /*B�� ��������*/
			num += 2;                                          
			meter->FactorC = dlt645_data_to_int32(&buff[num],2); /*C�� ��������*/
			num += 2;
		}	
	else
		num++;		
		
	if(buff[num] != 0xAA)
		{
			meter->EnergyForward = dlt645_data_to_int32(&buff[num],4); /*�����й��ܵ���*/
			num += 4;
			meter->EnergyBackward = dlt645_data_to_int32(&buff[num],4); /*�����й��ܵ���*/
			num += 4;
			meter->EnergyReactCombination1 = dlt645_data_to_int32(&buff[num],4); /*����޹�1�ܵ���*/
			num += 4;
			meter->EnergyReactCombination2 = dlt645_data_to_int32(&buff[num],4); /*����޹�2�ܵ���*/
			num += 4;
		}	
	else
		num++;	
				
	if(buff[num] != 0xAA)
		{
			meter->QuadrantReactCombination1 = dlt645_data_to_int32(&buff[num],4); /*��1�����޹��ܵ���*/
			num += 4;
			meter->QuadrantReactCombination2 = dlt645_data_to_int32(&buff[num],4); /*��2�����޹��ܵ���*/
			num += 4;
			meter->QuadrantReactCombination3 = dlt645_data_to_int32(&buff[num],4); /*��3�����޹��ܵ���*/
			num += 4;
			meter->QuadrantReactCombination4 = dlt645_data_to_int32(&buff[num],4); /*��4�����޹��ܵ���*/
			num += 4;
		}	
	else
		num++;		
		
	if(buff[num] != 0xAA)
		{
			meter->ActiveForwardDemand = dlt645_data_to_int32(&buff[num],3); /*��ǰ�й�����*/
			num += 3;
			meter->ReactForwardDemand = dlt645_data_to_int32(&buff[num],3); /*��ǰ�й�����*/
			num += 3;
		}	
	else
		num++;			
}

/****
  * Function: int dlt645_decode(unsigned const char *buff,int len,meter_db *meter)
  *@Describe: dlt645 ����
  *@Param_1 : buff :���������ݴ�
  *@Param_2 : len:���ݴ�����
  *@Param_3 : meter :�����ṹ��
  *@Return  : ����-1 ����ʧ��
  */
int dlt645_decode(unsigned char *buff,int len,elect_meter_t1 *meter)
{
	int i,bytes,type;
	long res;
	unsigned char Cchar[20] = {'\0'};
	unsigned char cs = 0;
	unsigned char *data = buff;
	time_t now;
	struct tm *p;
	unsigned char ptime[30];
	for(i = 0; i < (data[DLT645_DLEN] + DLT645_DATA); i++)
	{
	//	printf("%x,",data[i]);
				cs += data[i];
	}
	//printf("cs : %x,data[cs] = %x",cs,data[i]);
	if(cs == data[i])
				{
				//	printf("cs ok!\n");
				
				  if(data[DLT645_CTRL] == 0xd1)						
						{
							printf("\nCann't suppor this type:0xd1");
							return -2;	
						}
					if((data[DLT645_CTRL] & 0xf0) == 0x10)						
						{
							printf("\nrecive other host cmd");
							return -3;	
						}		
					
          //type					
          meter->type = ElectricEnergyMeter;
				  
				  //ID   
					//bytes = sprintf(meter->id,"%02x%02x%02x%02x%02x%02x%02x",data[DLT645_ADDR+5],data[DLT645_ADDR+4],data[DLT645_ADDR+3],data[DLT645_ADDR+2],data[DLT645_ADDR+1],data[DLT645_ADDR]);
					//meter->id[bytes] = '\0';
					
					ReverseByArray(&data[DLT645_ADDR],6);
					BcdToString(meter->id,&data[DLT645_ADDR],6);
					


          //DateRealTime
          time(&now);
					p = localtime(&now);		
					bytes = sprintf(meter->DateRealTime,"%04d%02d%02d%02d%02d%02d",1900 + p->tm_year,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
					meter->DateRealTime[bytes] = '\0';
					
					for(i=0;i<ELECT_DATA_FRAME_TYPE_MAX;i++)          /*���ݱ�ʶ����*/
						{
							if(memcmp(&DLT645_Data_Mark[i],&data[DLT645_DATA],4))
								continue;
							type = i;
							break;
						}

					switch(type)
					{							
							case ENERGY_FORWARDA:     /*�����й��ܵ���*/									
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],4);
							meter->EnergyForward = res;
							break;
						case ENERGY_BACKWARDA:    /*�����й��ܵ���*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],4);
							meter->EnergyBackward =  res;
							break;
						case VOLTAGE:            /*��ѹ/(A)��ѹ*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],2);
							meter->Voltage =  res;
							break;
						case CURRENT:	            /*����/(A)����*/									
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],3);
							meter->Current =  res;
							break;
						case POWER_ACT:            /*˲ʱ���й�����*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],3);
							meter->ActPower =  res;
							break;
						case POWER_REACT:         /*˲ʱ���޹�����*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],3);
							meter->ReactPower =  res;
							break;
						case FACTOR:              /*�ܹ�������*/							
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],2);
							meter->Factor =  res;
							break;
						case FREQUENCY:            /*����Ƶ��*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],2);
							meter->Frequency =  res;
							break;	

						case READ_TRIP:            /*����բ״̬*/
							
							if(((data[DLT645_TRIP_STATE_BYTE] - 0x33) && 0x10))
								meter->trip = 1;       /* �̵��� �ϣ���բ״̬*/
							else
								meter->trip = 0;       /* �̵��� ͨ����բ״̬*/	
							break;						
						default:
							printf("\r\nDLT645 format Erro!");
							type = -1;
							break;
					}
					return type;
				}
	return -1;			
}


int dlt645_decode1(unsigned char *buff,int len,elect_meter_t3 *meter)
{
	int i,bytes,type;
	long res;
	time_t now;
	struct tm *p;
	unsigned char ptime[50];
	unsigned char Cchar[20] = {'\0'};
	unsigned char cs = 0;
	unsigned char *data = buff;
	for(i = 0; i < (data[DLT645_DLEN] + DLT645_DATA); i++)
	{
	//	printf("%x,",data[i]);
				cs += data[i];
	}
	//printf("cs : %x,data[cs] = %x",cs,data[i]);
	if(cs == data[i])
				{
			//		printf("cs ok!\n");
				
					if(data[DLT645_CTRL] == 0xd1)						
						{
							printf("\nCann't suppor this type:0xd1");
							return -2;	
						}	
					
					if((data[DLT645_CTRL] & 0xf0) == 0x10)						
						{
							printf("\nrecive other host cmd");
							return -1;	
						}		

           //type					
          meter->type = ElectricEnergyMeter3;
          				  
				  //ID   
					//bytes = sprintf(meter->id,"%02x%02x%02x%02x%02x%02x%02x",(char)data[DLT645_ADDR+5],(char)data[DLT645_ADDR+4],(char)data[DLT645_ADDR+3],(char)data[DLT645_ADDR+2],(char)data[DLT645_ADDR+1],(char)data[DLT645_ADDR]);
					//meter->id[bytes] = '\0';
					
					ReverseByArray(&data[DLT645_ADDR],6);
					BcdToString(meter->id,&data[DLT645_ADDR],6);
					
          //DateRealTime
          time(&now);
					p = localtime(&now);		
					bytes = sprintf(meter->DateRealTime,"%04d%02d%02d%02d%02d%02d",1900 + p->tm_year,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
					meter->DateRealTime[bytes] = '\0';
					for(i=0;i<8;i++)          /*���ݱ�ʶ����*/
						{
							if(memcmp(&DLT645_Data_Mark[i],&data[DLT645_DATA],4))
								continue;
							type = i;
							break;
						}
						

					switch(type)
					{							
							case ENERGY_FORWARDA:     /*�����й��ܵ���*/									
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],4);
							meter->EnergyForward = res;
							break;
						case ENERGY_BACKWARDA:    /*�����й��ܵ���*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],4);
							meter->EnergyBackward =  res;
							break;
						case VOLTAGE:            /*��ѹ/(A)��ѹ*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],2);
							meter->VoltageA =  res;
							break;
						case CURRENT:	            /*����/(A)����*/									
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],3);
							meter->CurrentA =  res;
							break;
						case POWER_ACT:            /*˲ʱ���й�����*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],3);
							meter->ActPower =  res;
							break;
						case POWER_REACT:         /*˲ʱ���޹�����*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],3);
							meter->ReactPower =  res;
							break;
						case FACTOR:              /*�ܹ�������*/							
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],2);
							meter->Factor =  res;
							break;
						case FREQUENCY:            /*����Ƶ��*/								
							res = dlt645_data_to_int32(&data[DLT645_DATA + 4],2);
							meter->Frequency =  res;
							break;	
						case LOADRECORD:            /*��������*/
							if(data[DLT645_DATA] == 0xa0 && data[DLT645_DATA + 1] == 0xa0)
								get_load_record(&data[DLT645_DATA+3],data[DLT645_DATA + 2],meter);					
							else
								return -1;
							break;		
						default:
							printf("\r\nDLT645 format Erro!");
							type = -1;
							break;
					}
					return meter->type;
				}
	return -1;			
}

/****
  * Function: int dlt645_encode(char dlt645[],char *id,int type
  *@Describe: dlt645 ����
  *@Param_1 : dlt645[] :����������֡��ַ
  *@Param_2 : id:���ID
  *@Param_3 : type :��������
  *@Return  : ���ر��������ݳ���
  */
int dlt645_encode(unsigned char dlt645[],char *id,int type)
{
	int i,len,types;
	unsigned char cs;
	cs = 0;
	len = 0;
	types  = type;
	if(types > ELECT_DATA_FRAME_TYPE_MAX)             //��ֹ����DLT645_Data_MarkԽ��
		return -1;
	dlt645[DLT645_START]= DLT645_FRAME_START;                         /*֡��ʼλ   */  
	StringToBcd(id,&dlt645[DLT645_ADDR],1);                           /*��ַ��     */ 
	dlt645[DLT645_DSTART]= DLT645_FRAME_START;                        /*������ʼλ */ 
	dlt645[DLT645_CTRL] = DLT645_FRAME_READ_DATA;                     /*������λ   */   
	dlt645[DLT645_DLEN] = 4;                                          /*���ݳ���λ */ 
	len = DLT645_DATA;
	for(i=0;i<dlt645[DLT645_DLEN];i++)
		dlt645[len++] = DLT645_Data_Mark[type][i];                      /*����λ     */     
  for(i=0;i<len;i++)                              
		{
			cs += dlt645[i];
		}
	dlt645[len++]= cs;                                              /*У��λ     */ 
	dlt645[len++]= DLT645_FRAME_END;                                /*֡����λ   */
	return len;
}

/****
  * Function: int dlt645_ctl_decode(unsigned const char *buff,int len,meter_db *meter)
  *@Describe: dlt645 ����
  *@Param_1 : buff :���������ݴ�
  *@Param_2 : len:���ݴ�����
  *@Param_3 : meter :�����ṹ��
  *@Return  : ����-1 ����ʧ��
  */
int dlt645_ctl_decode(unsigned char *buff,int len,elect_meter_t1 *meter)
{
	int i,bytes,type;
	long res;
	unsigned char Cchar[20] = {'\0'};
	unsigned char cs = 0;
	unsigned char *data = buff;
	time_t now;
	struct tm *p;
	unsigned char ptime[30];
	for(i = 0; i < (data[DLT645_DLEN] + DLT645_DATA); i++)
		cs += data[i];
	if(cs == data[i])
	{
	
	  if(data[DLT645_CTRL] == 0x9c)		
	  	{
	  		printf("\n meter command successfull!");
	  		return 0;
	  	}
	  else if(data[DLT645_CTRL] == 0xdc)	 					
			{
				printf("\n meter command Fail!");
	  		return 1;	
			}
		else						
			{
				printf("\nCann't suppor this type:%d",data[DLT645_CTRL]);
				return 2;	
			}
  }
	return -1;			
}

/****
  * Function: int dlt645_ctl_encode(char dlt645[],char *id,int cmd)
  *@Describe: dlt645 ����բ����
  *@Param_1 : dlt645[] :����������֡��ַ
  *@Param_2 : id:���ID
  *@Param_3 : cmd :������������
                     0 ��բ��1 ��բ����2 ��բ��3 ������4 �����Ӵ���5 ���硢6 ������
  *@Return  : ���ر��������ݳ���
  */
int dlt645_ctl_encode(unsigned char dlt645[],char *id,int cmd)
{
	time_t now;
	struct tm *p;
	int i,len,types;
	unsigned char cs,tmp;
	cs = 0;
	len = 0;
	if(cmd > TRIP_CMD_MAX)
		return -1;
	dlt645[DLT645_START]= DLT645_FRAME_START;                         /*֡��ʼλ   */  
	StringToBcd(id,&dlt645[DLT645_ADDR],1);                           /*��ַ��     */ 
	dlt645[DLT645_DSTART]= DLT645_FRAME_START;                        /*������ʼλ */ 
	dlt645[DLT645_CTRL] = DLT645_CTRL_CODE_TRIP;                     /*������λ   */   
	dlt645[DLT645_DLEN] = 0x10;                                      /*���ݳ���λ */ 
	len = DLT645_DATA;
	
	for(i=0;i<4;i++) //����	PAP1P2P3
	 dlt645[len++] = PAP1P2P3[i];

  for(i=0;i<4;i++)
	 dlt645[len++] = C0C1C2C3[i];//�����ߴ���	:C0C1C2C3
  

  
  dlt645[len++] = TRIP_N1[cmd];           //�����룺N1 ��0 ��բ��1 ��բ����2 ��բ��3 ������4 �����Ӵ���5 ���硢6 ������
 
  dlt645[len++] = TRIP_N2;                //����λ��N2
  

  now = time(NULL);
	p = localtime(&now);	
	

	                                   //ʱ�䣺ssmmhhDDMMYY
                             
	dlt645[len++] = int8Tohex(p->tm_sec) + 0x33;
	dlt645[len++] = int8Tohex(p->tm_min) + 0x33;
	dlt645[len++] = int8Tohex(p->tm_hour) + 0x33;
	dlt645[len++] = int8Tohex(p->tm_mday) + 0x33;
	dlt645[len++] = int8Tohex(p->tm_mon + 1) + 0x33;
	dlt645[len++] = int8Tohex(p->tm_year + 1900 - 2000) + 0x33;
 
		
//  printf("\n");
  for(i=0;i<len;i++)                              
		{
			cs += dlt645[i];
	//		printf("%02x ",dlt645[i]);
		}
//	printf("\n");	
	dlt645[len++]= cs;                                                /*У��λ     */ 
	dlt645[len++]= DLT645_FRAME_END;	                                /*֡����λ   */
	return len;
}

