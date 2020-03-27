
/*
PLL M>16
0.5MHz < internal oscillator/PLL N < 2MHz ··················································································· (6)
40MHz < (internal oscillator/PLL N)× PLL M < 200MHz ········································································ (7)
PLL frequency = ((internal oscillator × PLL M)/(PLL N × PLL O)) ······································································· (8)
GCLK frequency = PLL frequency / GCLK frequency divider

internal oscillator	5.85 MHz
PLL M	58
PLL N	5
PLL O	1
PLL frequency	67.86 MHz
GCLK frequency divider	[000011] (divided by 4 )
GCLK frequency	16.96 MHz

PLL M	48	PLL N	6	PLL O	3	GCLK_Frequency_Divider(5)	// PLL=15.6MHz 	GCLK = 3.12MHz (0.3205 us)
PLL M	58	PLL N	5	PLL O	1	GCLK_Frequency_Divider(5)	// 67.86MHz		13.572MHz(0.074 us)
PLL M	75	PLL N	5	PLL O	1	GCLK_Frequency_Divider(5)	// 87.75MHz 	17.55 MHz(0.056898us ~ 57ns)


Dummy time = Dummy time width x GCLK cycle > 1us

Dead time = Dead time width x GCLK cycle > 3us

Scramble time= The total number of GCLKs in Scramble × GCLK cycle time
(Dummy time + Scramble time + Dead time) × the number of scrambles	// one line
	x the number of scans < 16.6ms									// one frame 

*/
static uint16_t reg_value_override(uint16_t addr_, uint16_t value_) {
#define PWM_mode 1
	//[0]: continue mode
	//[1]: one shot mode
#define PWM_data_bit (0x04) // 0x00
	//				The total number of GCLKs
	//[000]: 14-bit		16384
	//[001]: 13-bit		8192
	//[010]: 12-bit		4096
	//[011]: 11-bit		2048
	//[100]: 10-bit		1024
	//[101]~[111]: same as code [000] 16384
#define PWM_counting_mode 0x01 // 0x00
	//[00]: odd channel forward counting mode/even channel backward counting mode
	//[01]: all channel forward counting mode
	//[10]: all channel backward
	//[11]: same as code [00]
#define Chip_sleep_mode_enable 0
	//[0]: Disable
	//[1]: Enable
#define Chip_sleep_select 0
	//[0]: wakeup for brightness have value
	//[1]: wakeup for vsync
#define SYNC_pin_enable 0
	//[0]: vsync command valid, SYNC pin disable
	//[1]: vsync command invalid, SYNC pin enable
#define Scramble_number 0x00 // 0x01	// 0x03
	//[00]: 1 scramble
	//[01]: 4 scramble
	//[10]: 8 scramble
	//[11]: 16 scramble
#define Scan_number 0x07 //0x7
	//[0000]: 1 lines; [1000]: 9 lines;
	//[0001]: 2 lines; [1001]: 10 lines;
	//[0010]: 3 lines; [1010]: 11 lines;
	//[0011]: 4 lines; [1011]: 12 lines;
	//[0100]: 5 lines; [1100]: 13 lines;
	//[0101]: 6 lines; [1101]: 14 lines;
	//[0110]: 7 lines; [1110]: 15 lines;
	//[0111]: 8 lines; [1111]: 16 lines;
	if (addr_ == 0X0000) { return 
(PWM_mode<<0xf | PWM_data_bit <<0xc | PWM_counting_mode<<0xa | Chip_sleep_mode_enable <<0x9 | Chip_sleep_select <<0x8 | SYNC_pin_enable<<0x6 | Scramble_number<<0x4 | Scan_number<<0x0); } // 0x8037
//-----------------------------------------------------------------------------------------------------------------------------
	// Dummy time width
	else if(addr_ == 0X0001) { return 0x0018; }	// 0x0018 // Reserve
//-----------------------------------------------------------------------------------------------------------------------------
	// Dead time width
	else if(addr_ == 0X0002) { return 0x0048; }	// 0x0048 // Reserve
//-----------------------------------------------------------------------------------------------------------------------------
	// Scan change period
	else if(addr_ == 0X0003) { return 0x0018; }	// 0x0018 // scan change period. SC[15:0] x GCLK period
//----------------------------------------------------------------------------------------------------------------------------
	// Scan separate period
	else if(addr_ == 0X0004) { return 0x0018; }	// 0x0018 // MOS separate period, MS[15:0] x GCLK period
//-----------------------------------------------------------------------------------------------------------------------------
#define Channel_parallel_defined 0x00
	//[00]: 32 channel define
	//[01]: 16 channel define
	//[10]: 8 channel define
	//[11]: same as code [00]
#define PLL_O 0x03 // 0x3
	// [00]~[11]=Div is 8/4/2/1(PLL)
	//[00]: 8
	//[01]: 4
	//[10]: 2
	//[11]: 1
#define PLL_N 6 // 5 // 0x6
	// b~7(5bits) // [00000]~[11111]: 0~31
#define PLL_M 48 // 75 // 0x30
	// 6~0(7bits) // [0000000]~[1111111]: 0~127
	//Design options (analog_sel[2:0], PLL)
	//PLL M >= 64,analog selection = [001]
	//PLL M < 64,analog selection = [000]
	else if(addr_ == 0X0005) { return
((Channel_parallel_defined)<<0xE | (PLL_O)<<0xC | (PLL_N)<<0x7 | (PLL_M)<<0x0); } // 0x3330
#define SW_Discharge_enable 1
	//[0]: Disable
	//[1]: Enable
#define SW_Discharge_voltage_level 0x0
	//[000]~[111]:
	//[000]=> VLED-VSW=4.64V
	//[100]=> VLED-VSW=2.77V
	//[111]=> VLED-VSW=1.17V
#define SW_Discharge_time 0
	//[0]: 1us
	//[1]: 2us
#define GCLK_Frequency_Divider (0x02)
	//[000000]: 1
	//[000001]: 2
	//[000010]: 3
	//[000011]: 4
	//...
	//[111101]: 62
	//[111110]: 63
	//[111111]: 64
	else if(addr_ == 0X0006) { return 
((SW_Discharge_enable)<<0xf | (SW_Discharge_voltage_level)<<0xc | (SW_Discharge_time)<<0xb | (GCLK_Frequency_Divider)<<0x0); }	// 0x8005
//-----------------------------------------------------------------------------------------------------------------------------
#define Precharge_voltage 0 // (0xf)
	//[0000]: 1.95V [1000]: 3.15V
	//[0001]: 2.1V  [1001]: 3.3V
	//[0010]: 2.25V [1010]: 3.45V
	//[0011]: 2.4V  [1011]: 3.6V
	//[0100]: 2.55V [1100]: 3.75V
	//[0101]: 2.7V  [1101]: 3.9V
	//[0110]: 2.85V [1110]: 4.05V
	//[0111]: 3V  [1111]: 4.2V
#define Precharge_location (0)
	//[0]: pre-charge in deadtime
	//[1]: pre-charge during channel off
#define Precharge_global_enable (0)
	//[0]: Disable
	//[1]: Enable
#define Current_gain (0x07) // (0)
	//current gain value, [000]~[111]
	//Delta-ratio rang :100%~200% step by 14.29%
	else if(addr_ == 0X0007) { return 
((Precharge_voltage)<<0xc | (Precharge_location)<<0xb | (Precharge_global_enable)<<0xa | (Current_gain)<<0x0); }	// 0x0000
//-----------------------------------------------------------------------------------------------------------------------------
#define Thermal_test_voltage 0
	//[0]: 0.5V about 120CTSD=0
	//[1]: 0.425V about 168CTSD=1 (thermal shut down)
#define Analog_selection 0x0 // 0x01 // 0x0
	// Design options (analog_sel[2:0], PLL)
#define Decrease_overshoot 0
	//[0]: Internal VD=0.1V (Def)
	//[1]: Internal VD=0.3V
#define Feedback_function_enable 0
	//[0]: Disable
	//[1]: Enable
#define Open_error_detection_voltage 0
	//*Open Error detection voltage threshold,
	//[00] ~ [11]: High ~ Low
#define Short_error_detection_voltage 0
	//*Short Error detection voltage threshold,
	//[00] ~ [11]: Low ~ High
#define Tr_2_0 0
	//Speed Control Tr, [000] ~ [111]: Low ~ High
	//[000]:50nS	[100]:155nS
	//[001]:75nS	[101]:195nS
	//[010]:105nS	[110]:240nS
	//[011]:130nS	[111]: 300nS
#define Tf_2_0 0
	//Speed Control Tf, [000] ~ [111]: Low ~ High
	//[000]:55nS	[100]:155nS
	//[001]:85nS	[101]:205nS
	//[010]:105nS	[110]:285nS
	//[011]:125nS	[111]: 400nS
	else if(addr_ == 0X0008) { return 
(Thermal_test_voltage<<0xf | Analog_selection<<0xc | Decrease_overshoot<<0xb | Feedback_function_enable<<0xa | Open_error_detection_voltage<<0x8 | Short_error_detection_voltage<<0x6 | Tr_2_0<<0x3 | Tf_2_0<<0x0); }		// 0x0000
//-----------------------------------------------------------------------------------------------------------------------------
#define MOS_switch_parallel_defined (0x01)
	//MOS switch parallel defined
	//[00]: 16 switch define
	//[01]: 8 switch define
	//[10]: 4 switch define
	//[11]: 2 switch define
#define Global_brightness_control (0x3fff)
	// Global Brightness control (14-bit)
	else if(addr_ == 0X0009) { return 
(MOS_switch_parallel_defined<<0xe | Global_brightness_control<<0x0) ; }		// 0x7FFF
//-----------------------------------------------------------------------------------------------------------------------------
	else if(addr_ == 0X000a) { return 0x0000; }		// 0x0000
	else if(addr_ == 0X000b) { return 0x0008; }		// 0x0008
	else if(addr_ == 0X000c) { return 0x00FF; }		// 0x00FF
	else if(addr_ == 0X000d) { return 0x0000; }		// 0x0000
			//0x3fff
	else if(addr_ >= 0x0010 && addr_ <= 0x010f) { return 0xfff; }	// SCAN0~SCAN7 Brightness code of Channel31~Channel0
	else if(addr_ >= 0x0110 && addr_ <= 0x020f) { return 0; }	// SCAN8~SCAN15 Brightness code of Channel31~Channel0
			// 0xffff
	else if(addr_ >= 0x0210 && addr_ <= 0x023f) { return 0; }	// SCAN0~SCAN7 dot correction
	else if(addr_ >= 0x0240 && addr_ <= 0x026f) { return 0; }		// SCAN8~SCAN15 dot correction
	else if(addr_ == 0x0009) { return 0x3fff; }
	return value_;
}
