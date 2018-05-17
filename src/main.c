/* ------------------------------------------
 * Copyright (c) 2015, Synopsys, Inc. All rights reserved.

 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:

 * 1) Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.

 * 3) Neither the name of the Synopsys, Inc., nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * \version 2015.05
 * \date 2014-12-17
 * \author Wayne Ren(Wei.Ren@synopsys.com)
--------------------------------------------- */

/**
 * \defgroup	EMBARC_APP_GPIO		embARC GPIO Example
 * \ingroup	EMBARC_APPS_TOTAL
 * \ingroup	EMBARC_APPS_BOARD_EMSK
 * \ingroup	EMBARC_APPS_BAREMETAL
 * \brief	embARC Example for testing designware gpio module
 *
 * \details
 * ### Extra Required Tools
 *
 * ### Extra Required Peripherals
 *
 * ### Design Concept
 *     This example is designed to show how to use device [GPIO HAL API](dev_gpio.h) in embARC.
 *
 * ### Usage Manual
 *     Test cases for DW GPIO driver. In this example, the functions of the LEDs, buttons, and DIP switches are tested.
 *     ![ScreenShot of gpio under baremetal](pic/images/example/emsk/emsk_gpio.jpg)
 *
 * ### Extra Comments
 *
 */

/**
 * \file
 * \ingroup	EMBARC_APP_GPIO
 * \brief	example of timer0 and gpio
 */

/**
 * \addtogroup	EMBARC_APP_GPIO
 * @{
 */
//#define USE_FPU
#include "embARC.h"
#include "embARC_debug.h"
#include "ml_param.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static volatile int t0 = 0;
static volatile int blink ;
static DEV_GPIO_PTR led_port;
static DEV_GPIO_PTR port;

static DEV_GPIO_PTR port_output;
static DEV_GPIO_PTR port_input;


//extern unsigned short PATTERN_EXCEL[258];
extern unsigned short PATTERN_NEW[258];
// varialbe for ml operation 

uint32_t ticks_1s;                      /* Flag activated each second         */
int count;
int idx;
int s_thermal_in_sensing;
int s_thermal_in_ruler;
int s_thermal_in_ppg;
int s_thermal_out_sensing;
int s_thermal_out_ruler;
int s_thermal_out_ppg;
int s_in,s_ex;	
int min_in=10986;
int max_in=11840;
int key;
int print_count = 0;

#define PMOD_OFFSET0 8
#define MASK_GPIO0  0xFFFF00
#define PMOD_OFFSET1 8
#define MASK_GPIO1  0xFFFF00

// define by Vega
int est_temp_tmp =0;

int sum_in = 0;
int sum_out =0;

#ifdef USE_FPU 
float est_temp;
float mse_param[6]={
         0.46827281, // in**2
        -0.07968967, // in
       -26.49328155, // out**3
        56.32411059, // out**2
       -55.57685716, // out
        40.17161024};// const

float  in_offset =0,out_offset = 0;
float  train_min_in=11310.86;
float  train_min_out=3638.9;
float  train_max_in=13636.24;
float  train_max_out=17164.62;

float  in_raw_data, out_raw_data;
float in_sqr  ,in  ,out_cube , out_sqr  , out ; 
unsigned int dgr_fract,dgr_int,data_in100,data_out100;

#else
int est_temp;
signed int mse_param[6]={
         468, // in**2 e-3
        -8, // in   e-2
       -265, // out**3 e-1
        563, // out**2 e-1
       -55577, // out e-3
        4017};// const e-2
int in_offset = 756,out_offset = -1549;
int train_min_in=1131086;//e-2
int train_min_out=36389;//e-1
int train_max_in=1363624;//e-2
int train_max_out=1716462;//e-2
int in_raw_data, out_raw_data;
signed int in_sqr  ,in  ,out_cube , out_sqr  , out ; 
signed int dgr_fract,dgr_int,data_in100,data_out100;
#endif

int ave_cnt = 0;
int in_ave[100]={
	0
};
int ex_ave[100]={
	0
};

/**
 * \brief	timer0 interrupt service routine
 * \param[in]	*ptr	interrupt handler param
 */
static void timer0_isr(void *ptr)
{



	timer_int_clear(TIMER_0);
	uint32_t c_pattern;
	uint32_t bit0,bit1;
	int positive;			

	key=1;

	port_output-> gpio_write(PATTERN_NEW[count]<<PMOD_OFFSET0, MASK_GPIO0);	

	// read data bit 31 30  in J6[10:9]  
	port_input->gpio_read(&bit0,0x00010000) ; 
	port_input->gpio_read(&bit1,0x00020000) ; 
	bit0 = bit0 >> 16;
	bit1 = bit1 >> 17;

	//EMBARC_PRINTF("bit0: %d\n", bit0);
	//EMBARC_PRINTF("bit1: %d\n", bit1);
	positive=count%2;		//		positive=clk & 0x0001;
					
	if(count==0){
	     s_thermal_in_sensing=0;
	     s_thermal_in_ruler=0;
	     s_thermal_in_ppg=0;
	     s_thermal_out_sensing=0;
	     s_thermal_out_ruler=0;
	     s_thermal_out_ppg=0;
		 s_in=0;
		 s_ex=0;
	}																							
	if(count>=134 && count<=167 && positive==1){	
			if(count==135)
				idx=0;			
			s_thermal_in_sensing=s_thermal_in_sensing|(bit0<<(17-idx));
			s_thermal_out_sensing=s_thermal_out_sensing|(bit1<<(17-idx));	
			idx++;
	}

	if(count>=168 && count<=201 && positive==1){
			if(count==169)
				idx=0;	
			s_thermal_in_ruler=s_thermal_in_ruler|(bit0<<(17-idx));
			s_thermal_out_ruler=s_thermal_out_ruler|(bit1<<(17-idx));	
			idx++;	
	}

	if(count>=202 && count<=235 && positive==1){		
			if(count==203)
				idx=0;		
			idx++;								
		s_thermal_in_ppg=s_thermal_in_ppg | (bit0<<(idx-1));
		s_thermal_out_ppg=s_thermal_out_ppg | (bit1<<(idx-1));		
	}					
	//port_output -> gpio_read(&c_pattern,MASK_GPIO0);
	//EMBARC_PRINTF("count: %5d  pattern[15:0] HEX: %16x Bit1: %3d  Bit0:%3d \n", count,c_pattern>>PMOD_OFFSET0,bit1,bit0);
				
	if(count==237){				
		//Original
		//EMBARC_PRINTF("------------------------------\n");																			
		//EMBARC_PRINTF("s_thermal_in_sensing:  %5d\n",s_thermal_in_sensing);	
		//EMBARC_PRINTF("s_thermal_out_sensing: %5d\n",s_thermal_out_sensing);																			

	    //EMBARC_PRINTF("s_thermal_in_sensing :%d\n",s_thermal_in_sensing  );
	    //EMBARC_PRINTF("s_thermal_in_ruler   :%d\n",s_thermal_in_ruler    );
	    //EMBARC_PRINTF("s_thermal_in_ppg     :%d\n",s_thermal_in_ppg      );
	    //EMBARC_PRINTF("s_thermal_out_sensing:%d\n",s_thermal_out_sensing );
	    //EMBARC_PRINTF("s_thermal_out_ruler  :%d\n",s_thermal_out_ruler   );
	    //EMBARC_PRINTF("s_thermal_out_ppg    :%d\n",s_thermal_out_ppg     );

		ex_ave[ave_cnt] = s_thermal_out_sensing;
		in_ave[ave_cnt] = s_thermal_in_sensing;
		//EMBARC_PRINTF("ave_cnt: %5d\n",ave_cnt);																			
		ave_cnt = (ave_cnt+1)%100;
	}
	if(count==240){
		sum_in =0;
		sum_out=0;
		for(int i=0 ; i < 100 ; ++i){
			sum_in += in_ave[i];
			sum_out += ex_ave[i];
		}
#ifdef USE_FPU
		in_raw_data = sum_in / 100.0;
		out_raw_data = sum_out /100.0;
        int integer_digit_in = (unsigned int)(in_raw_data*100)/100;
        int fract_digit_in   = (unsigned int)(in_raw_data*100)%100;
        int integer_digit_out = (unsigned int)(out_raw_data*100)/100;
        int fract_digit_out   = (unsigned int)(out_raw_data*100)%100;
		EMBARC_PRINTF("in_ave: %d.%d\n",integer_digit_in,fract_digit_in);	
		EMBARC_PRINTF("out_ave: %d.%d\n",integer_digit_out,fract_digit_out);
#else
		in_raw_data = sum_in / 100;
		out_raw_data = sum_out /100;
		//EMBARC_PRINTF("in_ave: %5d\n",in_raw_data);
		//EMBARC_PRINTF("out_ave: %5d\n", out_raw_data);																			
#endif

		in_raw_data  += in_offset;
		out_raw_data += out_offset;
	}
	if(count==250){	
#ifdef USE_FPU
        out_raw_data= (out_raw_data - train_min_out)/(train_max_out-train_min_out);
        in_raw_data = (in_raw_data  - train_min_in)/(train_max_in-train_min_in);
        in_sqr      = in_raw_data*in_raw_data*mse_param[0];
        dgr_int     = (unsigned int) (in_sqr*10000)/10000;
        dgr_fract   = (unsigned int) (in_sqr*10000)%10000;
        EMBARC_PRINTF("in_sqr:  %d.%d\n",dgr_int,dgr_fract);
        in          = in_raw_data*mse_param[1];
		float in_tmp = -in;
        dgr_int     = (unsigned int) (in_tmp*10000)/10000;
        dgr_fract   = (unsigned int) (in_tmp*10000)%10000;
        EMBARC_PRINTF("in:  -%d.%d\n",dgr_int,dgr_fract);
        out_cube    = out_raw_data*out_raw_data*out_raw_data*mse_param[2];
		float out_cube_tmp = -out_cube;
        dgr_int     = (unsigned int) (out_cube_tmp*10000)/10000;
        dgr_fract   = (unsigned int) (out_cube_tmp*10000)%10000;
        EMBARC_PRINTF("out_cube:  -%d.%d\n",dgr_int,dgr_fract);
        out_sqr     = out_raw_data*out_raw_data*mse_param[3];
        dgr_int     = (unsigned int) (out_sqr*10000)/10000;
        dgr_fract   = (unsigned int) (out_sqr*10000)%10000;
        EMBARC_PRINTF("out_sqr:  %d.%d\n",dgr_int,dgr_fract);
        out         = out_raw_data*mse_param[4];
		float out_tmp = -out;
        dgr_int     = (unsigned int) (out_tmp*10000)/10000;
        dgr_fract   = (unsigned int) (out_tmp*10000)%10000;
        EMBARC_PRINTF("out:  -%d.%d\n",dgr_int,dgr_fract);
        dgr_int     = (unsigned int) (mse_param[5]*10000)/10000;
        dgr_fract   = (unsigned int) (mse_param[5]*10000)%10000;
        EMBARC_PRINTF("scalar:  %d.%d\n",dgr_int,dgr_fract);
		est_temp = in_sqr + in + out_cube + out_sqr + out + mse_param[5];
        dgr_int     = (unsigned int) (est_temp)*100/100;
        dgr_fract   = (unsigned int) (est_temp)*100%100;
        EMBARC_PRINTF("Estimated Temp:  %d.%d\n",dgr_int,dgr_fract);
#else
        // Output Degree
        data_out100 = (out_raw_data*100 - train_min_out*10)*100/1352572;
        data_in100  = (in_raw_data*100 - train_min_in)*100/232538;

        in_sqr      = data_in100*data_in100*mse_param[0]/100000;
        in          = data_in100*mse_param[1]/100;
        out_cube    = data_out100*data_out100*data_out100*mse_param[2]/100000;
        out_sqr     = data_out100*data_out100*mse_param[3]/1000;
        out         = data_out100*mse_param[4]/1000;
        est_temp    = in_sqr + in + out_cube + out_sqr + out + mse_param[5];
        //EMBARC_PRINTF("in_sqr: %d\nin:%d\nout_cube:%d\nout_sqr:%d\nout:%d\nscalar:%d\n",in_sqr,in,out_cube,out_sqr,out,mse_param[5]);
        dgr_int     = est_temp/100;
        dgr_fract   = est_temp%100;
		//EMBARC_PRINTF("\n\n                  Estimated Temp:  %d.%d\n",dgr_int,dgr_fract);
        //EMBARC_PRINTF("data_in100:  %d\n",data_in100);
#endif
	}
	if(count==255 && print_count == 0){
EMBARC_PRINTF("\n\n\n\n");
EMBARC_PRINTF("\n\n\n\n");
EMBARC_PRINTF("\n\n\n\n");
EMBARC_PRINTF("-------------------------------------------------------------------\n");																			
EMBARC_PRINTF("                      CHIP TEMPERATURE TESTING                     \n");
EMBARC_PRINTF("-------------------------------------------------------------------\n");																			
EMBARC_PRINTF("                       dP        .d888888   888888ba   a88888b.    \n"); 
EMBARC_PRINTF("                       88       d8'    88   88    `8b d8'   `88    \n"); 
EMBARC_PRINTF("   .d8888b. 88d8b.d8b. 88d888b. 88aaaaa88a a88aaaa8P' 88           \n"); 
EMBARC_PRINTF("   88ooood8 88'`88'`88 88'  `88 88     88   88   `8b. 88           \n"); 
EMBARC_PRINTF("   88.  ... 88  88  88 88.  .88 88     88   88     88 Y8.   .88    \n"); 
EMBARC_PRINTF("   `88888P' dP  dP  dP 88Y8888' 88     88   dP     dP  Y88888P'    \n"); 
EMBARC_PRINTF("                                                                   \n"); 
EMBARC_PRINTF("                           dP.  .dP                                \n"); 
EMBARC_PRINTF("                            `8bd8'                                 \n"); 
EMBARC_PRINTF("                            .d88b.                                 \n"); 
EMBARC_PRINTF("                           dP'  `dP                                \n"); 
EMBARC_PRINTF("                                                                   \n"); 
EMBARC_PRINTF(" .88888.   .d888888  .d88888b  dP .d88888b     d8888b. d88  d88888P\n"); 
EMBARC_PRINTF("d8a   a8b d8a    88  88a    'a 88 88a    ''        a88  88      d8'\n"); 
EMBARC_PRINTF("88     88 88aaaaa88a 'Y88888b. 88 `Y88888b.     aaad8'  88     d8' \n"); 
EMBARC_PRINTF("88     88 88     88        `8b 88       `8b        `88  88    d8'  \n"); 
EMBARC_PRINTF("Y8.   .8P 88     88  d8'   .8P 88 d8'   .8P        .88  88   d8'   \n"); 
EMBARC_PRINTF(" `8888P'  88     88   Y88888P  dP  Y88888P     d88888P d88P d8'    \n"); 
EMBARC_PRINTF("                                                                   \n");																			
EMBARC_PRINTF("-------------------------------------------------------------------\n");																			
EMBARC_PRINTF("                 Estimated Temp:  %d.%d\n",dgr_int,dgr_fract);
EMBARC_PRINTF("-------------------------------------------------------------------\n");																			
EMBARC_PRINTF("\n\n\n\n");
EMBARC_PRINTF("\n\n\n\n");
EMBARC_PRINTF("\n\n\n\n");
	}
		count++;
	if(count==258){
		count=0;
		print_count = (print_count + 1)%50;
	}

}

int main(void)
{
	

	int32_t ercd = 0;
	DEV_GPIO_BIT_ISR bit_isr;
	cpu_lock();
	board_init(); /* board init */

// customized gpio config
	port_output = gpio_get_dev(DW_GPIO_PORT_C);
	port_output -> gpio_open(0x00ffff00);
	port_output ->gpio_control(GPIO_CMD_SET_BIT_DIR_OUTPUT, (void *)0x00ffff00);
	port_output ->gpio_control(GPIO_CMD_DIS_BIT_INT, (void *)0x00ffff00);

	if (port_output == NULL){
		EMBARC_PRINTF("gpio_get_dev PORT C function error\r\n");
	} else {
		EMBARC_PRINTF("gpio_get_dev PORT C function OK\r\n");
		EMBARC_PRINTF("gpio_get_dev PORT C direction %x\r\n", port_output-> gpio_info.direction );
	}

	port_input =  gpio_get_dev(DW_GPIO_PORT_A);// J3_10_7 from [23:20]
	port_input -> gpio_open(0x000F0000);

	port_input -> gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x000F0000);

	port_input -> gpio_control(GPIO_CMD_DIS_BIT_INT, (void *)0x000F0000);
	if (port_input== NULL){
		EMBARC_PRINTF("gpio_get_dev PORT C function error\r\n");
	} else {
		EMBARC_PRINTF("gpio_get_dev PORT C  INPUT function OK\r\n");
		EMBARC_PRINTF("gpio_get_dev PORT C  INPUT direction %x\r\n",port_input-> gpio_info.direction );
	}


// timerinterrupt config	

	int_disable(INTNO_TIMER0);
	timer_stop(INTNO_TIMER0);
	int_handler_install(INTNO_TIMER0, timer0_isr);
	int_enable(INTNO_TIMER0);
	//timer_start(TIMER_0, TIMER_CTRL_IE, BOARD_CPU_CLOCK/1000);
	timer_start(TIMER_0, TIMER_CTRL_IE, BOARD_CPU_CLOCK/5000);


	cpu_unlock(); /* unlock system */

	while (1){
			
	}
		return E_SYS;
}

/** @} */
