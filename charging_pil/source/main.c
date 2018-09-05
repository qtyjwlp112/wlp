#include <iostm8s903k3.h>
#include "api.h"

#define        MASS_KEY1        0xAE
#define        MASS_KEY2        0x56
#define        EEP_BASE         0x4000

volatile    uchar Flag_AdcEvt;
volatile    uchar Flag_DispEvt;
volatile    uchar Flag_KeyEvt; 
volatile    uchar Flag_BuzEvt; 

const uint tmp_tab[100] = { 751,742,733,726,716,706,697,687,678,668, //0-9'c
                            658,649,639,629,619,609,600,590,580,571, //10-19'c    
                            561,551,541,531,521,512,502,492,482,473, //20-29'c
                            464,455,445,436,427,418,410,401,393,384, //30-39'c
                            375,367,359,350,344,336,328,321,314,306, //40-49'c
                            298,286,279,273,266,260,254,248,242,236, //50-59'c
                            230,225,220,215,210,204,198,195,190,186, //60-69'c
                            181,177,172,169,164,160,156,153,149,145, //70-79'c
                            142,138,135,132,129,126,123,121,117,115, //80-89'c 
                            112,109,107,104,102,100,97,95,93,90 //90-99'c 
                            };


/***** ��ʱ���� ****/
void  delay_ms(uchar  ms)
{
    uchar  i,j;
    while(ms--)
    {
        for(i=4;i!=0;i--)
          for(j=100;j!=0;j--);
    }
}
/************************************* 
*������:  void  Fosc_Init(void)
* �������: ��
* ����ֵ:   ��
*��������:  ��ʼ��ʱ��Ϊ8M
*************************************/
void  Fosc_Init(void)
{
    CLK_CKDIVR_HSIDIV = 1;    //2��Ƶ
}


/************************************* 
*������:  void  Adc_Init(void)
* �������: ��
* ����ֵ:   ��
*��������:  ADC��ʼ��
*************************************/
void  Adc_Init(void)
{
    PD_DDR &= 0x9f;
    PD_CR1 &= 0x9f;
    PD_CR2 &= 0x9f;
    PD_ODR |= 0x60;
    PB_DDR_DDR0 = 0;
    PB_CR1_C10 = 0;
    PB_CR2_C20 = 0;
    PB_ODR_ODR0 = 1;
    
    ADC_CR1_ADON = 1;      //��adc
    ADC_CR2_ALIGN = 1;     //�Ҷ�������
    ADC_CR1_SPSEL = 3;     //4��Ƶʱ��
}

/************************************* 
*������:  void  Timer_Init(void)
* �������: ��
* ����ֵ:   ��
*��������:  ��ʼ��TIMER1��ʱ��
*************************************/
void  Timer1_Init(void)
{
    TIM1_PSCRH = 0;
    TIM1_PSCRL = 7;          //(7+1)��ƵΪ1M
    TIM1_ARRH = 0x3;
    TIM1_ARRL = 0xe8;        //ÿ1ms�ж�һ��
    TIM1_IER = 0x01;         //��������ж�
    TIM1_CR1 = 0x01;         //������ʹ�ܣ���ʼ����
}
/************************************* 
*������:  void  EEP_Init(void)
* �������: ��
* ����ֵ:   ��
*��������:  EEPROM�洢��ʼ��
*************************************/
void  EEP_Init(void)
{
    FLASH_CR1 = 0x00;
    FLASH_CR2 = 0x00;
    FLASH_NCR2 = 0xff;
    FLASH_DUKR = MASS_KEY1;
    FLASH_DUKR = MASS_KEY2;
    while(!FLASH_IAPSR_DUL);        //�ȴ�д��������
}
/***** 8λ����д�뺯�� ****/
void  eep_write(uint addr,uchar dat) //��ͬоƬ��EEPROM������ͬ
{
    *((uchar*)(addr + EEP_BASE)) = dat;
    while(!FLASH_IAPSR_EOP);        //�ȴ�д���
}
/***** 8λ���ݶ�ȡ���� ****/
uchar  eep_read(uint addr)
{
    return  *((uchar*)(EEP_BASE + addr));
}
/***** 16λ����д�뺯�� ****/
void  eep_input(uint addr,uint data)
{
    uchar  dataH, dataL;
    dataL = data;
    dataH = data>>8;
    eep_write(addr*2, dataH);
    delay_ms(2);
    eep_write(addr*2+1, dataL);
    delay_ms(2);
}

/************************************* 
*������:  uint  adc_get(uchar channel)
* �������: channel:adcͨ��
* ����ֵ:   Analog:����adcֵ
*��������:  ����Ӧͨ����adcֵ���в���
*************************************/
uint  adc_get(uchar channel)
{
    uint  dataH, dataL,Analog=0,value=0;
    uint m[5],max,min;
    uchar i;
    for(i=0;i<5;i++)
    {
        ADC_CSR_CH = channel;              //AIN*��
        ADC_CR1_ADON = 1;      //����ת��
        asm("nop");
        asm("nop");
        while(ADC_CSR_EOC==0);
        dataL = ADC_DRL;            //�Ҷ�������
        dataH = ADC_DRH;            //��ȡ�Ĵ�������
        ADC_CSR_EOC = 0;
        value = dataH<<8|dataL;
        m[i]=value;
        Analog=Analog+value;       
    }
    max=m[0];
    min=m[0];
    for(i=0;i<5;i++)    //ȥ��ͷ
    {
        if(m[i]>max)max=m[i];
        if(m[i]<min)min=m[i];
    }
    Analog=(Analog-max-min)/3; //ȡƽ��
    return Analog;
}

/***** ADC��ѯ������ ****/
void ADC_FUNC(void)
{
    if(Flag_AdcEvt){
        Flag_AdcEvt = 0;

      }
}
/***** ��������ѯ������ ****/
void BUZZER_LED_FUNC(void)
{
    if(Flag_BuzEvt){
        Flag_BuzEvt = 0;

      }
}
/***** ������ѯ������ ****/
void KEY_FUNC(void)
{
    if(Flag_KeyEvt){
        Flag_KeyEvt = 0;

      }
}
/***** �������ʾ��ѯ������ ****/
void DISPLAY_FUNC(void)
{
    if(Flag_DispEvt){
        Flag_DispEvt = 0;

      }
}
/************************************* 
*������:  void  main(void)
* �������: ��
* ����ֵ:   ��
*��������:  ������ 
*************************************/
void  main(void)
{
    asm("sim");    //�����ж�
    Fosc_Init();
    
    Adc_Init();

    //_18b20_init();
    EEP_Init();
    asm("rim");    //�����ж�
    while(1)
    {
        DISPLAY_FUNC();      
        KEY_FUNC();
        BUZZER_LED_FUNC();
        ADC_FUNC();
    }
}
/************************************* 
*������:  __interrupt void TIM1_OVR_UIF(void)
* �������: ��
* ����ֵ:   ��
*��������:  TIMER1�жϺ���
*************************************/
#pragma   vector = TIM1_OVR_UIF_vector
__interrupt void TIM1_OVR_UIF(void)
{
    TIM1_SR1_UIF = 0;
    static uchar AdcTimeCnt = 0,KeyTimeCnt = 0,BuzTimeCnt = 0;

     if(BuzTimeCnt++ >=Time_BuzImp){ // ��ѯʱ�� �궨���н����޸�
        BuzTimeCnt = 0;
        Flag_BuzEvt = 1;
    }
    if(KeyTimeCnt++ >=Time_KeyImp){  //��ѯʱ�� �궨���н����޸�
        KeyTimeCnt = 0;
        Flag_KeyEvt = 1;
    } 
    if(AdcTimeCnt++ >=Time_AdcImp){ //��ѯʱ�� �궨���н����޸�
        AdcTimeCnt = 0;
        Flag_AdcEvt = 1;
    }  
    
    
}


