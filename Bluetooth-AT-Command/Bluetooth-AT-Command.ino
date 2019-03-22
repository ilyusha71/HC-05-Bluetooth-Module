/*

AUTHOR: Hazim Bitar (techbitar)
DATE: Aug 29, 2013
LICENSE: Public domain (use at your own risk)
CONTACT: techbitar at gmail dot com (techbitar.com)

*/
/***********************************************************************
 * Author: iLYuSha Wakaka (哇咔咔逗比遊戲實驗室)
 * Date: 2019-03-21
 * 各種大坑整理
 * 1. 序列阜監控窗記得要切換成 NL/CL 模式
 * 2. 本範例Pin9為HC-05提供高電位，進入AT命令模式
 *    若無法順利進入，請透過HC-05提供的按鍵直接進入AT命令模式
 * 限流電阻：
 *  HC-05 RX ==> 1KΩ ==> Arduino TX
 *  HC-05 RX ==> 220Ω ==> GND
***********************************************************************/
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11); // RX | TX
// Arduino RX = 10 <===> HC-05 TX
// Arduino TX = 11 <===> HC-05 RX

void setup()
{
  pinMode(9, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(9, HIGH);
  Serial.begin(9600);
  Serial.println("Enter AT commands:");
  BTSerial.begin(38400);  // HC-05 default speed in AT command more
}

void loop()
{

  // Keep reading from HC-05 and send to Arduino Serial Monitor
  if (BTSerial.available())
    Serial.write(BTSerial.read());

  // Keep reading from Arduino Serial Monitor and send to HC-05
  if (Serial.available())
    BTSerial.write(Serial.read());
}
