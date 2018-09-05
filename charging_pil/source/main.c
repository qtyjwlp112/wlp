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


/***** 延时函数 ****/
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
*函数名:  void  Fosc_Init(void)
* 输入参数: 无
* 返回值:   无
*函数描述:  初始化时钟为8M
*************************************/
void  Fosc_Init(void)
{
    CLK_CKDIVR_HSIDIV = 1;    //2分频
}

/*do a test*/
/************************************* 
*函数名:  void  Adc_Init(void)
* 输入参数: 无
* 返回值:   无
*函数描述:  ADC初始化
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
    
    ADC_CR1_ADON = 1;      //打开adc
    ADC_CR2_ALIGN = 1;     //右对齐数据
    ADC_CR1_SPSEL = 3;     //4分频时钟
}

/************************************* 
*函数名:  void  Timer_Init(void)
* 输入参数: 无
* 返回值:   无
*函数描述:  初始化TIMER1定时器
*************************************/
void  Timer1_Init(void)
{
    TIM1_PSCRH = 0;
    TIM1_PSCRL = 7;          //(7+1)分频为1M
    TIM1_ARRH = 0x3;
    TIM1_ARRL = 0xe8;        //每1ms中断一次
    TIM1_IER = 0x01;         //允许更新中断
    TIM1_CR1 = 0x01;         //计数器使能，开始计数
}
/************************************* 
*函数名:  void  EEP_Init(void)
* 输入参数: 无
* 返回值:   无
*函数描述:  EEPROM存储初始化
*************************************/
void  EEP_Init(void)
{
    FLASH_CR1 = 0x00;
    FLASH_CR2 = 0x00;
    FLASH_NCR2 = 0xff;
    FLASH_DUKR = MASS_KEY1;
    FLASH_DUKR = MASS_KEY2;
    while(!FLASH_IAPSR_DUL);        //等待写保护解锁
}
/***** 8位数据写入函数 ****/
void  eep_write(uint addr,uchar dat) //不同芯片，EEPROM容量不同
{
    *((uchar*)(addr + EEP_BASE)) = dat;
    while(!FLASH_IAPSR_EOP);        //等待写完成
}
/***** 8位数据读取函数 ****/
uchar  eep_read(uint addr)
{
    return  *((uchar*)(EEP_BASE + addr));
}
/***** 16位数据写入函数 ****/
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
*函数名:  uint  adc_get(uchar channel)
* 输入参数: channel:adc通道
* 返回值:   Analog:采样adc值
*函数描述:  对相应通道的adc值进行采样
*************************************/
uint  adc_get(uchar channel)
{
    uint  dataH, dataL,Analog=0,value=0;
    uint m[5],max,min;
    uchar i;
    for(i=0;i<5;i++)
    {
        ADC_CSR_CH = channel;              //AIN*脚
        ADC_CR1_ADON = 1;      //启动转换
        asm("nop");
        asm("nop");
        while(ADC_CSR_EOC==0);
        dataL = ADC_DRL;            //右对齐数据
        dataH = ADC_DRH;            //读取寄存器数据
        ADC_CSR_EOC = 0;
        value = dataH<<8|dataL;
        m[i]=value;
        Analog=Analog+value;       
    }
    max=m[0];
    min=m[0];
    for(i=0;i<5;i++)    //去两头
    {
        if(m[i]>max)max=m[i];
        if(m[i]<min)min=m[i];
    }
    Analog=(Analog-max-min)/3; //取平均
    return Analog;
}

/***** ADC轮询处理函数 ****/
void ADC_FUNC(void)
{
    if(Flag_AdcEvt){
        Flag_AdcEvt = 0;

      }
}
/***** 蜂鸣器轮询处理函数 ****/
void BUZZER_LED_FUNC(void)
{
    if(Flag_BuzEvt){
        Flag_BuzEvt = 0;

      }
}
/***** 按键轮询处理函数 ****/
void KEY_FUNC(void)
{
    if(Flag_KeyEvt){
        Flag_KeyEvt = 0;

      }
}
/***** 译码管显示轮询处理函数 ****/
void DISPLAY_FUNC(void)
{
    if(Flag_DispEvt){
        Flag_DispEvt = 0;

      }
}
/************************************* 
*函数名:  void  main(void)
* 输入参数: 无
* 返回值:   无
*函数描述:  主函数 
*************************************/
void  main(void)
{
    asm("sim");    //关总中断
    Fosc_Init();
    
    Adc_Init();

    //_18b20_init();
    EEP_Init();
    asm("rim");    //开总中断
    while(1)
    {
        DISPLAY_FUNC();      
        KEY_FUNC();
        BUZZER_LED_FUNC();
        ADC_FUNC();
    }
}
/************************************* 
*函数名:  __interrupt void TIM1_OVR_UIF(void)
* 输入参数: 无
* 返回值:   无
*函数描述:  TIMER1中断函数
*************************************/
#pragma   vector = TIM1_OVR_UIF_vector
__interrupt void TIM1_OVR_UIF(void)
{
    TIM1_SR1_UIF = 0;
    static uchar AdcTimeCnt = 0,KeyTimeCnt = 0,BuzTimeCnt = 0;

     if(BuzTimeCnt++ >=Time_BuzImp){ // 轮询时间 宏定义中进行修改
        BuzTimeCnt = 0;
        Flag_BuzEvt = 1;
    }
    if(KeyTimeCnt++ >=Time_KeyImp){  //轮询时间 宏定义中进行修改
        KeyTimeCnt = 0;
        Flag_KeyEvt = 1;
    } 
    if(AdcTimeCnt++ >=Time_AdcImp){ //轮询时间 宏定义中进行修改
        AdcTimeCnt = 0;
        Flag_AdcEvt = 1;
    }  
    
    
}


