#ifndef __remote_H
#define __remote_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#define msInHour					3600000
#define msInMinute				60000
#define msInSecond				1000

#define ESP_RX_SIZE	65

#define START_POS			0
#define TYPE_POS			START_POS + 1
#define DATA_POS			TYPE_POS + 1
#define CHECK_SUM_POS	DATA_POS + 2
#define END_POS				CHECK_SUM_POS + 1
#define ESP_TX_SIZE		END_POS + 1
#define TIMEOUT_SEND	100
#define BATERY_DELAY	997
typedef enum {
	CMD_START_SEN_VAL = 58, // :
	CMD_STOP_SEN_VAL = 59, // ;
	CMD_START_PROGRAM = 60, // <
	CMD_STOP_PROGRAM = 61, // =
	END_MARKER = 62, // >
	CMD_FOCUS1_ON = 65, // A
	CMD_FOCUS1_OFF = 97, // a
	CMD_SHUTTER1_ON = 66, // B
	CMD_SHUTTER1_OFF = 98, // b
	CMD_FOCUS2_ON = 67, // C
	CMD_FOCUS2_OFF = 99, // c
	CMD_SHUTTER2_ON = 68, // D
	CMD_SHUTTER2_OFF = 100, // d
	CMD_MENU = 77, // M
	CMD_DATA = 78, // N
	SEND_BATTERY = 76, // L
	SEND_SENSOR = 83, // S
} CommandValues_t;

//Used in Fast Mode
typedef enum {
	UNDEF,
	GATE1,
	GATE2,
	ERR
} photogate_t ;

// Type of device connected
typedef enum {
	DEV_NONE = 0,
	DEV_CAMERA,
	DEV_FLASH,
	DEV_SPLIT
} DevMode_t;

// When to trigger
typedef enum {
	TRIG_L = 0,
	TRIG_E = 1,
	TRIG_H = 2
} Dev_Trig_Mode_t;

// Menu of the remote
typedef enum {
    m_None = 0,
		m_Manual,
		m_Bulb,
		m_Triggered,
		m_FastObjects,
		m_Droplets,
		m_Test
	
} Menu_t;

// Structure for remote parameters
typedef struct __attribute__((packed))  {
    DevMode_t Dev_Mode; //
    uint16_t Seq_Delay; //
    uint16_t Split_Delay;
    uint8_t Delay_H;
    uint8_t Delay_M;
    uint8_t Delay_S;
    uint8_t WakeUp;
    uint16_t MLU_Delay;
    uint16_t Pictures_No;
    uint16_t Exposure_Time;
    uint16_t Process_Time;
    uint8_t Interval_H;
    uint8_t Interval_M;
    uint8_t Interval_S;
    uint16_t pAmp_Value;
    uint16_t Amp_Value;
    uint16_t TriggerVal;
    Dev_Trig_Mode_t TriggerAt;
    uint16_t Dev_Delay;
    uint16_t Dev_Cycle;
    uint16_t TargetDistance;
		uint8_t FreeFall;
    uint16_t V1V2_Delay;
    uint16_t V1D1_Size;
    uint16_t V1D1_Delay;
    uint16_t V1D2_Size;
    uint16_t V1D2_Delay;
    uint16_t V1D3_Size;
    uint16_t V1D3_Delay;
    uint16_t V2D1_Size;
    uint16_t V2D1_Delay;
    uint16_t V2D2_Size;
    uint16_t V2D2_Delay;
    uint16_t V2D3_Size;
    uint16_t V2D3_Delay;
    uint8_t BtnTime;
} Remote_Param_t;

// Remote configuration
typedef struct {
	uint8_t adcReady; // We have a complete ADC
	uint32_t batteryTime;
	uint16_t batteryValue;// Battery ADC reading
	uint8_t sensorTime; // When to send the sensor data
	uint16_t sensorValue; // Sensor ADC reading
	uint8_t hasNewdata; // New hata has arrived on serial
	Menu_t crtMenu; // Selected menu mode
	uint8_t isSendingSensor; // Asked to send sensor reading
	uint8_t isProgramStopped; // Asked to run a "program" Start/Stop on Android
	uint8_t status_Focus1; // Is Focus 1 is pressed
	uint8_t status_Shutter1; // Is Shutter 1 is pressed
	uint8_t status_Focus2; // Is Focus 2 is pressed
	uint8_t status_Shutter2; // Is Shutter 1 is pressed
	uint8_t status_Params; // New parameters received on serial
	Remote_Param_t Params; // Parameters array
} Remote_Conf_t;

extern volatile Remote_Conf_t myRemote;
extern volatile uint8_t battMode;
extern volatile uint8_t battTime;
extern volatile uint8_t photogateOK;
void initRemote();
void sendData();
void startReceiving();
void sendMessage(const uint8_t msgType, const uint16_t value);
void menuManual(void);
void setManual(void);
void setBulb(void);
void setTriggered(void);
void setFast(void);
void setValve(void);

#ifdef __cplusplus
}
#endif

#endif /* __remote_H */

