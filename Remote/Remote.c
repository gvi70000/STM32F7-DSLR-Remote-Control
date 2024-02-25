#include "Remote.h"
#include "gpio.h"
#include "usart.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "adc.h"
#include "AD5235.h"
#include "CycleCounter.h"

// Structure of parameters received from Android
volatile Remote_Conf_t myRemote = {0};;
// Used in several functions to hold the delay in Sequence and Split mode
uint32_t delaySeq = 0;
uint32_t delaySplit = 0;

// Used to indicate USART1 transmission complete
volatile uint8_t txNotComplete = 0;
// Variables used in fast objects and HW interrupts
volatile photogate_t statusGates = UNDEF;
volatile uint8_t targetReached = 0;
// Receive buffer for USART3
volatile uint8_t ESP_RxBuff[ESP_RX_SIZE];
volatile uint32_t t1, t2, ccc;

// Function to generate checksum and place it in the last position of the array
void generateChecksum(uint8_t *array, uint8_t length) {
	uint8_t checkSum = 0;
	if (length < 3) return; // Ensure there's enough length for start, char, and checksum
	for (uint16_t i = 1; i < length - 1; i++) {
		checkSum += array[i];
	}
	array[length - 1] = checkSum; // Store the checksum in the last position
}

// Function to check if the checksum in the last position matches the calculated checksum
uint8_t checkChecksum(uint8_t length) {
	if (length < 4) return 0; // Not a valid structure
	uint8_t len = length - 2;
	uint8_t checksum = 0;
	for (uint8_t i = 1; i < len; i++) {
		checksum += ESP_RxBuff[i];
	}
	return (checksum == ESP_RxBuff[len]); // Compare calculated checksum with the stored one
}

// Init some of the remote parameters
void initRemote(){
	myRemote.hasNewdata = 0;
	myRemote.isSendingSensor = 0;
	myRemote.isProgramStopped = 1;
	myRemote.status_Params = 0;
}

// Process the message received
void processMsg(uint8_t msgLen) {
	if(checkChecksum(msgLen) && (ESP_RxBuff[START_POS] == CMD_START_PROGRAM) && (ESP_RxBuff[msgLen - 1] == END_MARKER)) {
		myRemote.hasNewdata = 1;
		switch(ESP_RxBuff[TYPE_POS]) {
			case CMD_STOP_SEN_VAL:
				myRemote.isSendingSensor = 0;
			break;	
			case CMD_START_SEN_VAL:
				myRemote.isSendingSensor = 1;
			break;
			case CMD_START_PROGRAM:
				myRemote.isProgramStopped = 0;
			break;
			case CMD_STOP_PROGRAM:
				myRemote.isProgramStopped = 1;
			break;
			case CMD_DATA:
				myRemote.Params = *(Remote_Param_t *)&ESP_RxBuff[DATA_POS];
				myRemote.status_Params = 1;
			break;
			case CMD_FOCUS1_ON:
				myRemote.status_Focus1 = 1;
			break;
			case CMD_SHUTTER1_ON:
				myRemote.status_Shutter1 = 1;
			break;
			case CMD_FOCUS2_ON:
				myRemote.status_Focus2 = 1;
			break;
			case CMD_SHUTTER2_ON:
				myRemote.status_Shutter2 = 1;
			break;
			case CMD_MENU:
				myRemote.crtMenu = ESP_RxBuff[DATA_POS];
			break;
			case CMD_FOCUS1_OFF:
				myRemote.status_Focus1 = 0;
			break;
			case CMD_SHUTTER1_OFF:
				myRemote.status_Shutter1 = 0;
			break;
			case CMD_FOCUS2_OFF:
				myRemote.status_Focus2 = 0;
			break;
			case CMD_SHUTTER2_OFF:
				myRemote.status_Shutter2 = 0;
			break;
		}
	}
}

// Enable the DMA receive
void startReceiving(){
	__HAL_UART_CLEAR_IDLEFLAG(&huart3);
	HAL_UART_Receive_DMA(&huart3, (uint8_t *)ESP_RxBuff, ESP_RX_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

// Set transmission flag trasfer complete
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) {
	if(UartHandle->Instance == USART3) {
		txNotComplete = 0;
	}
}

// DMA Rx callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
  if(UartHandle->Instance == USART3) {
		//Stop DMA	
		HAL_UART_DMAStop(&huart3);
		//Determine how many items of data have been received/
		uint8_t data_length = ESP_RX_SIZE - huart3.hdmarx->Instance->NDTR;
		//process the message
		if(data_length){
			processMsg(data_length);
		}
		startReceiving();
	}
}

// Send data to ESP/Android based on times
// Do not add these in the timers callback
void sendData() {
	// If new ADC is calculated
	if(myRemote.adcReady){
		// Is time to send battery?
		if(myRemote.batteryTime && (HAL_GetTick() > myRemote.batteryTime)){
			sendMessage(SEND_BATTERY, myRemote.batteryValue);
			ccc++;
		}
		// Is time to send sensor value?
		if(myRemote.sensorTime){
			sendMessage(SEND_SENSOR, myRemote.sensorValue);
			myRemote.sensorTime = 0;
		}
		myRemote.adcReady = 0;
	}
}

// Send messages to ESP/Android
void sendMessage(const uint8_t msgType, const uint16_t value) {
	uint8_t txBuffer[ESP_TX_SIZE];
	txBuffer[START_POS] = CMD_START_PROGRAM; // Add <
	txBuffer[TYPE_POS] = msgType;
	*(uint16_t *)&txBuffer[DATA_POS] = value;
	generateChecksum(txBuffer, END_POS);
	txBuffer[END_POS] = END_MARKER;
//	HAL_UART_Transmit(&huart3, (uint8_t *)txBuffer, ESP_TX_SIZE, TIMEOUT_SEND);
	txNotComplete = 1; // Assume transmission is not complete
	HAL_UART_Transmit_DMA(&huart3, (uint8_t *)txBuffer, ESP_TX_SIZE);

	// Wait for transmission completion or timeout
	uint32_t startTime = HAL_GetTick();
	while (txNotComplete) {
		// Check for timeout
		if ((HAL_GetTick() - startTime) > TIMEOUT_SEND) {
			// Timeout occurred, abort transmission
			HAL_UART_AbortTransmit(&huart3);
			// Re-initialize UART
			MX_USART3_UART_Init();
			txNotComplete = 0;
			break; // Exit the loop
		}
		// Add a short delay to prevent CPU overload
		HAL_Delay(1);
	}
}

// Calculate exposure time
uint32_t getExposureTime(void) {
	uint32_t referenceTime = HAL_GetTick() + msInSecond * myRemote.Params.Exposure_Time;
	if(myRemote.Params.Dev_Mode > DEV_NONE) {
		delaySeq = referenceTime + myRemote.Params.Seq_Delay;
	} else {
		delaySeq = 0;
	}		
	if(myRemote.Params.Dev_Mode == DEV_SPLIT) {		
		delaySplit = delaySeq + myRemote.Params.Split_Delay;
	} else {
		delaySplit = 0;
	}
	return referenceTime;
}

// Press Focus 1
void pressF1(void) {
	setFocus1(ON);
	HAL_Delay(myRemote.Params.BtnTime);
	setFocus1(OFF);
}

// Press Shutter 1
void pressS1(void) {
	setShutter1(ON);
	HAL_Delay(myRemote.Params.BtnTime);
	setShutter1(OFF);	
}

void pressF2(void) {
	setFocus2(ON);
	HAL_Delay(myRemote.Params.BtnTime);
	setFocus2(OFF);	
}

// Press Focus 2
void pressS2(void) {
	setShutter2(ON);
	HAL_Delay(myRemote.Params.BtnTime);
	setShutter2(OFF);		
}

// Press Shutter 1 if Mirror LockUp is used
void doMLU(void) {
	if(myRemote.Params.MLU_Delay) {
		pressS1();
		HAL_Delay(myRemote.Params.MLU_Delay);
	}	
}

// Press the focus on device 2 if the time is correct
void doSequence(void) {
	// Time to do the sequence?
	if(HAL_GetTick() >= delaySeq) {
		pressF2();	
		delaySeq = 0;
	}	
}
// Press the shutter on device 2 if the time is correct
void doSplit(void) {
	// Time to do the split?
	if(HAL_GetTick() >= delaySplit) {
		pressS2();
		delaySplit = 0;		
	}		
}

//
void checkDeviceModeTimes(void) {
	// We only run this if we have set a second device
	// If the split time > 0 and the device is set as split
	if(delaySplit)
		doSplit();
	// If the sequence time > 0 and the device is not none
	if(delaySeq)
		doSequence();
}

// This function is called when myRemote.Params.Dev_Mode] != 0, Device 2 is set to Camera, Flash or Split
void RunDeviceMode(void) {
	// The delay between the 2 devices
	HAL_Delay(myRemote.Params.Seq_Delay);
	switch(myRemote.Params.Dev_Mode) {
		case DEV_CAMERA:
		case DEV_FLASH:	
			setFocus2(ON);
			setShutter2(ON);
			HAL_Delay(myRemote.Params.BtnTime);
			setFocus2(OFF);
			setShutter2(OFF);	
		break;
		case DEV_SPLIT:
			setFocus2(ON);
			HAL_Delay(myRemote.Params.BtnTime);
			setFocus2(OFF);
			HAL_Delay(myRemote.Params.Split_Delay);
			setShutter2(ON);
			HAL_Delay(myRemote.Params.BtnTime);
			setShutter2(OFF);	
		break;
		default:
			//Nothing to do
		break;
	}
}

// Manual Mode Menu
void setManual(void) {
	// While we hare in manual mode
	while(myRemote.crtMenu == m_Manual) {
		// Send battery if available
		sendData();
		// If new data from Android is available
		if(myRemote.hasNewdata){
			if(myRemote.Params.Dev_Mode == DEV_NONE){
				setFocus1(myRemote.status_Focus1);
				setShutter1(myRemote.status_Shutter1);
				setFocus2(myRemote.status_Focus2);
				setShutter2(myRemote.status_Shutter2);
			} else {
				setFocus1(myRemote.status_Focus1);
				setShutter1(myRemote.status_Shutter1);
				if(myRemote.status_Shutter1) RunDeviceMode();
			}
			myRemote.hasNewdata = 0;	
		}
	}
}

// Bulb/TimeLapse Mode Menu extension
void doTheBulb(void) {
	// First we grab the number of images to tak in bulb mode
	// We decrese this number till we get to 0 and then stop the program
	uint16_t n_pics = myRemote.Params.Pictures_No;
	//time vatiable used for exposure and processing
	uint32_t timeExposure = 0;
	// While we have pictures to take and the program is not stopped
	while(n_pics && !myRemote.isProgramStopped) {
		// Send batter or pictures left to Android
		sendData();
		// Do i need to wake up the camera
		if(myRemote.Params.WakeUp)
			pressF1();
		// Do i need to activate the mirror?
		doMLU();	
		// Now i press the focus and shutter to take the picture
		setFocus1(ON);
		HAL_Delay(myRemote.Params.BtnTime);
		setShutter1(ON);
		// Get Exposure in ms, the function also sets the sequence and split times
		timeExposure = getExposureTime();
		while(HAL_GetTick() < timeExposure) {
				// If we press the Stop button during the wait period we stop the whole loop
				if(myRemote.isProgramStopped)
					break;
				// Do i need to do something for device 2?
				checkDeviceModeTimes();
		}
		setFocus1(OFF);
		setShutter1(OFF);
		n_pics--;
		sendMessage(SEND_SENSOR, n_pics);
		// If we have time for processing the image on the camera we will have to wait till we get next image
		if(myRemote.Params.Process_Time) {
			// Get the waiting in ms
			timeExposure = HAL_GetTick() + msInSecond*myRemote.Params.Process_Time;
			while(HAL_GetTick() < timeExposure) {
				// If we press the Stop button during the wait period we stop the whole loop
				if(myRemote.isProgramStopped)
					break;
			}	
		}	
		//send num of pictures left to ESP
	}//end if(!isProgramStopped)
	myRemote.isProgramStopped = 1;	
}

// Bulb/TimeLapse Mode Menu 
void setBulb(void) {
	// Calculate the time to wait
	uint32_t timeToWait = 0;
	// Reset the other time variable
	uint32_t timeToWakeUp = 0;
	// While we hare in bulb mode
	while(myRemote.crtMenu == m_Bulb) {
		sendData();
		// If the status is not set to stop
		if(!myRemote.isProgramStopped) {
			// Do i need to wait?
			timeToWait = HAL_GetTick() + msInHour * myRemote.Params.Delay_H + msInMinute * myRemote.Params.Delay_M + msInSecond * myRemote.Params.Delay_S;
			// Time to wake up camera? only if we have a value > 0 in settings
			if(myRemote.Params.WakeUp) {
				timeToWakeUp = timeToWait - msInSecond * myRemote.Params.WakeUp;
			} else {
				timeToWakeUp = 0;
			}
			// Wait for time to pass
			while(HAL_GetTick() < timeToWait) {
				// If we press the Stop button during the wait period we stop the whole loop
				if(myRemote.isProgramStopped)
					break;
				// Is time to wake up the camera?
				if(timeToWakeUp && (HAL_GetTick() > timeToWakeUp))
					pressF1();
			}
			// Finally, if the status is not set to stop then doTheBulb
			if(!myRemote.isProgramStopped) 
				doTheBulb();
		}//end if(!isProgramStopped) 
	}	//end while(myRemote.crtMenu == 2)	
}

// Triggered Mode Menu
void setTriggered(void) {
	uint32_t cycleTime = 0;
	//Send sensor values to Android every 250ms
	MX_TIM6_Init();
	// PowerUp the sensor
	setAdPower(ON);
	// Deselect the SPI CS
	setSpiSelect(ON);
	// Init SPI
	MX_SPI1_Init();
	// Wait a bit
	HAL_Delay(500);
	// Set the sensor sensitivity
	setWiper(0, myRemote.Params.pAmp_Value);
	setWiper(1, myRemote.Params.Amp_Value);
	// While we are in trigger mode
	while(myRemote.crtMenu == m_Triggered) {
		// Send battery/sensor value to Android
		sendData();
		// If a parameter is changed update the potentiometer
		if(myRemote.status_Params) {
			setWiper(0, myRemote.Params.pAmp_Value);
			setWiper(1, myRemote.Params.Amp_Value);
			myRemote.status_Params = 0;
		}
		if(!myRemote.isProgramStopped) {
			delaySeq = 0;	
			delaySplit = 0;
			cycleTime = 0;
			// Check is sensor value > trigger value
			// If sensor value > trigger value and we trigger at high
			// or the sensor value  < trigger value and we trigger at low
			if (((myRemote.sensorValue > myRemote.Params.TriggerVal) && (myRemote.Params.TriggerAt == TRIG_H)) ||
          ((myRemote.sensorValue < myRemote.Params.TriggerVal) && (myRemote.Params.TriggerAt == TRIG_L)) ||
          ((myRemote.sensorValue == myRemote.Params.TriggerVal) && (myRemote.Params.TriggerAt == TRIG_E))) {
				// Calculate the moment when the shutter 1 will be ON
				uint32_t S1_Time = HAL_GetTick() + myRemote.Params.Dev_Delay;
				cycleTime = S1_Time;
				// Press focus 1
				setFocus1(ON);
				// Calculate the moment when the focus 2 will be ON - we have a sequence or split
				if(myRemote.Params.Dev_Mode > DEV_CAMERA) {
					delaySeq = S1_Time + myRemote.Params.Seq_Delay;
					cycleTime = delaySeq;
				}
				// Calculate the moment when the shutter 2 will be ON - we have a split
				if(myRemote.Params.Dev_Mode > DEV_FLASH) {
					delaySplit = delaySeq + myRemote.Params.Split_Delay;	
					cycleTime = delaySplit;
				}
				while(HAL_GetTick() < cycleTime) {
					if(HAL_GetTick() >= S1_Time)
						setShutter1(ON);
					if(delaySeq && (HAL_GetTick() >= delaySeq))
						doSequence();
					if(delaySplit &&(HAL_GetTick() >= delaySplit))
						doSplit();	
				}				
				setShutter1(OFF);	
				setFocus1(OFF);
				cycleTime += myRemote.Params.Dev_Cycle;
				while(HAL_GetTick() < cycleTime) {
					//Do nothing
				}
			}
		}
	}
	HAL_TIM_Base_Stop(&htim6);
	HAL_TIM_Base_MspDeInit(&htim6);
	// Power Down the sensor
	setAdPower(OFF);
	// Select the SPI CS
	setSpiSelect(OFF);
	// DeInit SPI
	HAL_SPI_MspDeInit(&hspi1);
}

//Fast Objects Mode Menu
void setFast(void) {
	HAL_NVIC_DisableIRQ(EXTI3_IRQn);
	HAL_TIM_Base_MspDeInit(&htim1); // Disable buzzer timer, just in case
	HAL_TIM_Base_MspDeInit(&htim6); // Data send timer, just in case
	HAL_TIM_Base_MspDeInit(&htim6); // Disable battery timer
	HAL_SPI_MspDeInit(&hspi1); // Disable SPI, just in case
	HAL_UART_MspDeInit(&huart3); // Disable UART 3
	HAL_ADC_MspDeInit(&hadc1); //Disable ADC
	
	const double DIST_GATES				= 50000000.0; //Distace between photogates (5cm) in nm
	const double TICK_PERIOD			= 9.2592592592592592592592592592593; //Time of one tick for TIM2 frequency 108MHz
	// Distance to target is in cm, 1cm = 10mm = 10000um = 10000000nm
	const uint32_t PERIOD_IN_CM		= 1080000; //10000000.0/9.2592592592592592592592592592593;
	const uint32_t PERIOD_IN_5CM	= 5400000; //50000000.0/9.2592592592592592592592592592593;
	const uint16_t CYCLE_COMPENSATION	= 1132; //1133 - 1
	uint32_t cyclesCounter = 0;
	uint32_t cyclesToTarget = 0;
	uint32_t cntStart = 0;
	uint32_t cntStop = 0;
	double speed = 0.0;
	while(myRemote.crtMenu == m_FastObjects) {
		if(!myRemote.isProgramStopped) {
			// Set up timer
			TIMER2_DMA_Init();
			// TIM5 is set up to trigger an interrupt after 15 s if no photogate is triggered
			MX_TIM5_Init();
			MX_TIM2_Init();
			statusGates = UNDEF;
	
			// Start the timer
			// TIM2 is working at 108MHz, this mean that we have a period of 9.2592592592592592592592592592593 ns for each cycle
			// The distance between the photogates is 5cm or 50mm or 50000um or 50000000nm
			// The time it takes to travel between photogates is counted by TIM2 * 9.2592592592592592592592592592593 ns
			// The speed v=d/t = 50000000nm/(TIM2 Value*9.2592592592592592592592592592593)
			// TIM2 is started By EXTI on pin PB9 and is stoped by EXTI on PB8
			// HAL_TIM_Base_Start_IT(&htim2); takes 278 MCU cycles @ 216MHz 
			// HAL_TIM_Base_Stop_IT(&htim2); takes 250 MCU cycles @ 216MHz  
			// HAL_TIM_PeriodElapsedCallback takes 78 MCU cycles @ 216MHz for each call or 20 TIM2 cycles @ 54MHz
			// Seq A takes 1538 MCU cycles @ 216MHz or 385 cycles @ 54MHz including Turning On Shutter and Focus
			myRemote.Params.TargetDistance = 1000;
			// Set to 10 seconds
			// PhotogateOK = 1 if we trigger photogate 1
			// PhotogateOK = 2 if we trigger photogate 2
			// PhotogateOK = 3 if period gets to 1079999999(10s)
			// htim5.Instance->ARR = 1619999999;//15seconds
			// Avoid trigger of the interrup at start - bug in HAL?
			htim5.Instance->SR = 0;
			HAL_TIM_Base_Start_IT(&htim5);
			HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, &cntStart,1);
			HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, &cntStop,1);
			// 2706 Clock cycles for instructions above
			while(statusGates < GATE2) {
				;
			}	
			// ***Seq A Start***
			if(statusGates == GATE2) {
				HAL_TIM_Base_Stop_IT(&htim5);
				// 2266 cock cycles for seq A /2=1133 TIM5 cycles
				cyclesCounter = cntStop - cntStart;
				speed = (double)PERIOD_IN_5CM/(double)cyclesCounter;
				// Knowing the speed we can calculate the number of cycles to reach the target
				// t=d/v
				// t = (myRemote.Params.TargetDistance]*10000000)/speed
				// period*cyclesToTarget = (myRemote.Params.TargetDistance]*10000000)/speed
				cyclesToTarget = (uint32_t)((double)PERIOD_IN_CM*((double)myRemote.Params.TargetDistance/speed)) - CYCLE_COMPENSATION;
				htim5.Instance->CNT = 0;
				htim5.Instance->ARR = cyclesToTarget;
				HAL_TIM_Base_Start_IT(&htim5);
				setFocus2(ON);
				setShutter2(ON);
				// ***Seq A End***

				while(!targetReached) {
					;
				}
				setFocus1(ON);
				setShutter1(ON);
				HAL_TIM_Base_Stop_IT(&htim5);
				HAL_TIM_Base_MspDeInit(&htim2);
				HAL_TIM_Base_MspDeInit(&htim5);
				// Start GPIO to initial state
				MX_GPIO_Init();
				// Restart DMA
				MX_DMA_Init();
				// Restart UART
				MX_USART3_UART_Init();
				// Restart ADC
				MX_ADC1_Init();
				// Init Battery Timer
				MX_TIM7_Init();
				
				sendMessage(SEND_SENSOR, (uint16_t)(speed  *1000.0));
				myRemote.isProgramStopped = 1;
			}
		}
	}
}

//Droplets Mode Menu
void setValve(void) {
	// Set Pins as Output
	HAL_TIM_Base_MspDeInit(&htim2);
	setPinsValveMode();
	// Stop Tim6
	HAL_TIM_Base_Stop(&htim6);
  // Arrays to store the timings
	uint16_t Drop_1_Start[2] = {0, 0};
	uint16_t Drop_1_End[2] = {0, 0};
	uint16_t Drop_2_Start[2] = {0, 0};
	uint16_t Drop_2_End[2] = {0, 0};
	uint16_t Drop_3_Start[2] = {0, 0};
	uint16_t Drop_3_End[2] = {0, 0};
	uint16_t FlashDelay[2] = {0, 0};
	uint32_t Refresh[2] = {0, 0};
	uint8_t nValves = 0;
	uint16_t MaxTime = 0;
	
	while(myRemote.crtMenu == m_Droplets) {
		sendData();
		// If a parameter is changed recalculate all values
		if(myRemote.status_Params) {
			// If the fisrt drop size for each valve is 0
			if((!myRemote.Params.V1D1_Size) && (!myRemote.Params.V1D1_Size)) {
				nValves = 0; // 
				} else if((myRemote.Params.V1D1_Size) && (myRemote.Params.V1D1_Size)) {
					nValves = 2;
			} else {
				nValves = 1;
			}
			//recalculate timings arrays
			Drop_1_Start[0] = 0;
			Drop_1_Start[1] = myRemote.Params.V1V2_Delay;
			
			Drop_1_End[0] = Drop_1_Start[0] + myRemote.Params.V1D1_Size;
			Drop_2_Start[0] = Drop_1_End[0] + myRemote.Params.V1D1_Delay;
			Drop_2_End[0] = Drop_2_Start[0] + myRemote.Params.V1D2_Size;
			Drop_3_Start[0] = Drop_2_End[0] + myRemote.Params.V1D2_Delay;
			Drop_3_End[0] = Drop_3_Start[0] + myRemote.Params.V1D3_Size;
			FlashDelay[0] = Drop_3_End[0] + myRemote.Params.V1D3_Delay;
			
			Drop_1_End[1] = Drop_1_Start[1] + myRemote.Params.V2D1_Size;
			Drop_2_Start[1] = Drop_1_End[1] + myRemote.Params.V2D1_Delay;
			Drop_2_End[1] = Drop_2_Start[1] + myRemote.Params.V2D2_Size;
			Drop_3_Start[1] = Drop_2_End[1] + myRemote.Params.V2D2_Delay;
			Drop_3_End[1] = Drop_3_Start[1] + myRemote.Params.V2D3_Size;
			FlashDelay[1] = Drop_3_End[1] + myRemote.Params.V2D3_Delay;
			if(FlashDelay[0] > FlashDelay[1]) {
				MaxTime = FlashDelay[0];
			} else {
				MaxTime = FlashDelay[1];
			}
			myRemote.status_Params = 0;
		}
		
		if(!myRemote.isProgramStopped) {
			// ToDo doMLU with timer
			doMLU();
			Refresh[0] = HAL_GetTick();
			Refresh[1] = 0;
			// Prepare camera
			setFocus1(ON);
			setShutter1(ON);
			// Wait to start from the begining of milisecond
			while (Refresh[0] == HAL_GetTick()) {;}
				
			Refresh[0] = HAL_GetTick();//start now
			while (Refresh[1] <= MaxTime) {
				Refresh[1] = HAL_GetTick() - Refresh[0];
				//i = 0 Sensor 1/ i = 1 Sensor 2
				for(uint8_t ii = 0; ii < nValves; ii++) {
					if (Refresh[1] >= Drop_3_End[ii])
						Valve_Off(ii);
					else if (Refresh[1] >= Drop_3_Start[ii])
						Valve_On(ii);
					else if (Refresh[1] >= Drop_2_End[ii])
						Valve_Off(ii);
					else if (Refresh[1] >= Drop_2_Start[ii])
						Valve_On(ii);
					else if (Refresh[1] >= Drop_1_End[ii])
						Valve_Off(ii);
					else if (Refresh[1] >= Drop_1_Start[ii])
						Valve_On(ii);
				}
			}
			//wait to trigger the flash
			HAL_Delay(MaxTime);
			//trigger the flash
			setShutter2(ON);
			HAL_Delay(10);
			//turn off camera shutter and focus
			setShutter1(OFF);
			setFocus1(OFF);
			//turn off flash
			setShutter2(OFF);
		}
	}
	//Stop Tim6
	HAL_TIM_Base_Stop(&htim6);
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	// If the Interrupt is triggered by 1st Channel
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		statusGates = GATE1;
	}
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
		statusGates = GATE2;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// TIM5 used to measure the time till gast object get to target
  if (htim->Instance == htim5.Instance) {
		//To avoid false trigger at the start
		//if(htim5.Instance->CNT) {
				if(statusGates == GATE2) {
					targetReached = 1;
				} else {
					statusGates = ERR;
				}
		//}
	}
	// TIM6 used to send sensor values to Android every 250ms
  if (htim->Instance == htim6.Instance) {
		if(myRemote.isSendingSensor) {
			myRemote.sensorTime = 1;
		}
  }
	
	// TIM7 used for battery
	// Send battery data to Android every 20s
	// At 19sec we switch on the battery voltage divider
	// and after one more second we read the value and send it to Android
	// battMode indicates that the battery voltage divider switch is on
	if (htim->Instance == htim7.Instance) {
		t2 = HAL_GetTick() - t1;
		t1 = HAL_GetTick();
		// If batteryTime = 1,  Can be replaced with HAL_GPIO_ReadPin(CTRL_BATT_GPIO_Port, CTRL_BATT_Pin)
		if(myRemote.batteryTime) {
			myRemote.batteryTime = 0;
			// Disable voltage divider
			setBatteryCtrl(OFF);
			setShutter1(OFF);
			// Reset timer
			// Set new timer interrupt to 19seconds
			htim->Instance->ARR = sec19;
		} else { //
			// At 19 seconds, turn on the battery voltage divider
			setBatteryCtrl(ON);
			setShutter1(ON);
			myRemote.batteryTime = HAL_GetTick() + BATERY_DELAY;
			// Set the new timer period to 1 second
			htim->Instance->ARR = sec1;
			ccc = 0;
		}
	}
}