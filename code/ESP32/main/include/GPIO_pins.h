#pragma once
#include <Arduino.h>

#ifdef EDGEBOX

void inline GPIO_Init() {}

// Programming and Debugging Port
#define U0_TXD GPIO_NUM_43
#define U0_RXD GPIO_NUM_44

// I2C
#define I2C_SDA GPIO_NUM_20
#define I2C_SCL GPIO_NUM_19

// I2C INT fro RTC PCF8563
#define I2C_INT GPIO_NUM_9

// SPI BUS for W5500 Ethernet Port Driver
#define ETH_SS GPIO_NUM_10
#define ETH_MOSI GPIO_NUM_12
#define ETH_MISO GPIO_NUM_11
#define ETH_SCK GPIO_NUM_13
#define ETH_INT GPIO_NUM_14
#define ETH_RST GPIO_NUM_15

// A7670G
#define LTE_AIRPLANE_MODE GPIO_NUM_16
#define LTE_PWR_EN GPIO_NUM_21
#define LTE_TXD GPIO_NUM_48
#define LTE_RXD GPIO_NUM_47

// RS485
#define RS485_TXD GPIO_NUM_17
#define RS485_RXD GPIO_NUM_18
#define RS485_RTS GPIO_NUM_8

// CAN BUS
#define CAN_TXD GPIO_NUM_1
#define CAN_RXD GPIO_NUM_2

// BUZZER
#define BUZZER GPIO_NUM_45

// Motor Controller
#define PWMa_H GPIO_NUM_13
#define ENABLE_H GPIO_NUM_12
#define PWMb_H GPIO_NUM_14
#define PWMa_V GPIO_NUM_27
#define ENABLE_V GPIO_NUM_26
#define PWMb_V GPIO_NUM_25

// Anemometer
#define AnemometerPin A0
#define ADC_Resolution 65536.0

#endif
#ifdef NORVI_GSM_AE02

#define BUTTONS GPIO_NUM_36 // Analog pin to read buttons

void inline GPIO_Init() { pinMode(GPIO_NUM_36, INPUT); }

// Programming and Debugging Port
#define U0_TXD GPIO_NUM_03
#define U0_RXD GPIO_NUM_01

// I2C
#define I2C_SDA GPIO_NUM_16
#define I2C_SCL GPIO_NUM_17

// GSM Modem
#define LTE_PWR_EN GPIO_NUM_21
#define LTE_TXD GPIO_NUM_32
#define LTE_RXD GPIO_NUM_33

// RS485
#define RS485_TXD GPIO_NUM_26
#define RS485_RXD GPIO_NUM_25
#define RS485_RTS GPIO_NUM_22

// OLED display definitions
#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

// Motor Controller
#define PWMa_H GPIO_NUM_13
#define ENABLE_H GPIO_NUM_12
#define PWMb_H GPIO_NUM_14
#define PWMa_V GPIO_NUM_27
#define ENABLE_V GPIO_NUM_26
#define PWMb_V GPIO_NUM_25

// Anemometer
#define AnemometerPin A0
#define ADC_Resolution 4095.0

#endif

#ifdef Waveshare_Relay_6CH

#define RGB_LED_PIN 38
#define GPIO_PIN_Buzzer 21   // Buzzer Control GPIO
#define PWM_Channel 1        // PWM Channel
#define Frequency 1000       // PWM frequencyconst
#define Resolution 8
#define Dutyfactor 200

void inline RGB_Light(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
   neopixelWrite(RGB_LED_PIN, green_val, red_val, blue_val); // RGB color adjustment
}

void inline Buzzer_PWM(uint16_t Time) // ledChannel：PWM Channe    dutyfactor:dutyfactor
{
   ledcWrite(PWM_Channel, Dutyfactor);
   delay(Time);
   ledcWrite(PWM_Channel, 0);
}

void inline GPIO_Init() {
   pinMode(RGB_LED_PIN, OUTPUT);     // Initialize the control GPIO of RGB
   pinMode(GPIO_PIN_Buzzer, OUTPUT); // Initialize the control GPIO of Buzzer

   ledcSetup(PWM_Channel, Frequency, Resolution); // Set PWM channel
   ledcAttachPin(GPIO_PIN_Buzzer, PWM_Channel);   // Connect the channel to the corresponding pin
}
// UARTS
#define U0_TXD GPIO_NUM_43
#define U0_RXD GPIO_NUM_44

// RS485
#define RS485_TXD GPIO_NUM_17
#define RS485_RXD GPIO_NUM_18
#define RS485_RTS -1

// I2C
#define I2C_SDA GPIO_NUM_47
#define I2C_SCL GPIO_NUM_48

// Motor Controller
#define PWMa_H GPIO_NUM_1
#define ENABLE_H GPIO_NUM_12
#define PWMb_H GPIO_NUM_2
#define PWMa_V GPIO_NUM_41
#define ENABLE_V GPIO_NUM_11
#define PWMb_V GPIO_NUM_42

// Anemometer
#define AnemometerPin A0
#define ADC_Resolution 4095.0

#endif
#ifdef Lilygo_Relay_4CH

// I2C
#define I2C_SDA GPIO_NUM_15
#define I2C_SCL GPIO_NUM_14

// OLED display definitions
#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

// Motor Controller
#define PWMa_H GPIO_NUM_21
#define ENABLE_H GPIO_NUM_22
#define PWMb_H GPIO_NUM_19
#define PWMa_V GPIO_NUM_18
#define ENABLE_V GPIO_NUM_23
#define PWMb_V GPIO_NUM_5

// Anemometer
#define AnemometerPin A0
#define ADC_Resolution 4095.0

#endif

#ifdef Lilygo_Relay_6CH

// HT74HC595
#define HT74HC595_CLOCK GPIO_NUM_5
#define HT74HC595_LATCH GPIO_NUM_6
#define HT74HC595_DATA GPIO_NUM_7
#define HT74HC595_OUT_EN GPIO_NUM_4

#define RTC_IRQ GPIO_NUM_18

// I2C
#define I2C_SDA GPIO_NUM_16
#define I2C_SCL GPIO_NUM_17

#ifdef Has_OLED
// OLED display definitions
#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#endif

// Anemometer
#define AnemometerPin A0
#define ADC_Resolution 4095.0

#endif

#ifdef ESP_32Dev

// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

// Motor Controller
#define PWMa_H GPIO_NUM_13
#define ENABLE_H GPIO_NUM_12
#define PWMb_H GPIO_NUM_14
#define PWMa_V GPIO_NUM_27
#define ENABLE_V GPIO_NUM_26
#define PWMb_V GPIO_NUM_25

// Anemometer
#define AnemometerPin A0
#define ADC_Resolution 4095.0


#endif

#ifdef ESP32_S3

// I2C
#define I2C_SDA GPIO_NUM_8
#define I2C_SCL GPIO_NUM_9

// Motor Controller
#define PWMa_H GPIO_NUM_4
#define ENABLE_H GPIO_NUM_5
#define PWMb_H GPIO_NUM_6
#define PWMa_V GPIO_NUM_7
#define ENABLE_V GPIO_NUM_15
#define PWMb_V GPIO_NUM_16

// Anemometer
#define AnemometerPin A0
#define ADC_Resolution 4095.0

#endif