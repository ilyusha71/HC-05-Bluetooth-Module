/***********************************************************************
 * Author: iLYuSha Wakaka (哇咔咔逗比遊戲實驗室)
 * Date: 2019-03-22
***********************************************************************/
/***********************************************************************
 * HC-05 Bluetooth Module
 * Current limit resistor：
 *    HC-05 RX ==> 1KΩ ==> Arduino TX
 *    HC-05 RX ==> 220Ω ==> GND
***********************************************************************/
#include <SoftwareSerial.h>

#define RX_PIN     2      // Transmit pin
#define TX_PIN     3      // Receive pin

SoftwareSerial hc05(RX_PIN, TX_PIN); // RX | TX
// RX_PIN <===> HC-05 TX
// TX_PIN <===> HC-05 RX

/***********************************************************************
 * MFRC 522 RFID Reader
***********************************************************************/
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN    9      // Reset pin
#define SS_PIN     10     // Slave select pin

MFRC522 mfrc522(SS_PIN, RST_PIN);    // 建立MFRC522物件
// 12 pin <===> MFRC522 MISO
// 11 pin <===> MFRC522 MISO
// 13 pin <===> MFRC522 SCK
// 10 pin <===> MFRC522 NSS
// 9  pin <===> MFRC522 RST

MFRC522::MIFARE_Key key;  // 儲存金鑰

byte sector = 15;   // 指定讀寫的「區段」，可能值:0~15
byte block = 1;     // 指定讀寫的「區塊」，可能值:0~3
byte blockData[16] = "abcd1234";   // 最多可存入16個字元
// 若要清除區塊內容，請寫入16個 0
//byte blockData[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// 暫存讀取區塊內容的陣列，MIFARE_Read()方法要求至少要18位元組空間，來存放16位元組。
byte buffer[18];

MFRC522::StatusCode status;

void setup() 
{
    Serial.begin(9600);         // Initialize Arduino SP
    hc05.begin(9600);          // Initialize Bluetooth SP
    SPI.begin();               // Initialize SPI
    mfrc522.PCD_Init();        // Initialize MFRC522
    Serial.println(F("Please scan MIFARE Classic card..."));
    // 準備金鑰（用於key A和key B），出廠預設為6組 0xFF。
    for (byte i = 0; i < 6; i++) 
    {
        key.keyByte[i] = 0xFF;
    }    
}
char val;
String msg, command;
void loop() 
{
    // Serial monitor -> Unity
    // 若收到「序列埠監控視窗」的資料，則送到藍牙模組
    if (Serial.available()>0) 
    {             
        val = Serial.read();
        if(val != '\n')
            msg += val;
        else
        {
            hc05.println(msg); //必須使用println才能與Unity對接，因為設定換行結尾char(10)
            msg = "";
        }
    }
       
    // 若收到藍牙模組的資料，則送到「序列埠監控視窗」
    if (hc05.available()) 
    {
        val = hc05.read();
        if(val !=  '\n')
            msg += val;
        else
        {
            command = msg;
            Serial.println(msg);
            msg = "";
        }
    }
    
    if(command == "Pass")
    {
        digitalWrite(12, LOW);
        Serial.println("Yes!");
        hc05.println("OK! Feedback");
        command = "";
    }
    // RFID -> Unity
    // Unity -> RFID  
}






/***********************************************************************
 * RFID
***********************************************************************/


void writeBlock(byte _sector, byte _block, byte _blockData[]) {
  if (_sector < 0 || _sector > 15 || _block < 0 || _block > 3) {
    // 顯示「區段或區塊碼錯誤」，然後結束函式。
    Serial.println(F("Wrong sector or block number."));
    return;
  }

  if (_sector == 0 && _block == 0) {
    // 顯示「第一個區塊只能讀取」，然後結束函式。
    Serial.println(F("First block is read-only."));
    return;
  }

  byte blockNum = _sector * 4 + _block;  // 計算區塊的實際編號（0~63）
  byte trailerBlock = _sector * 4 + 3;   // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK) {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // 在指定區塊寫入16位元組資料
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockNum, _blockData, 16);
  // 若寫入不成功…
  if (status != MFRC522::STATUS_OK) {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // 顯示「寫入成功！」
  Serial.println(F("Data was written."));
}

void readBlock(byte _sector, byte _block, byte _blockData[])  {
  if (_sector < 0 || _sector > 15 || _block < 0 || _block > 3) {
    // 顯示「區段或區塊碼錯誤」，然後結束函式。
    Serial.println(F("Wrong sector or block number."));
    return;
  }

  byte blockNum = _sector * 4 + _block;  // 計算區塊的實際編號（0~63）
  byte trailerBlock = _sector * 4 + 3;   // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK) {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  byte buffersize = 18;
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockNum, _blockData, &buffersize);

  // 若讀取不成功…
  if (status != MFRC522::STATUS_OK) {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // 顯示「讀取成功！」
  Serial.println(F("Data was read."));
}

void SetupRFID() {
  Serial.begin(9600);
  SPI.begin();               // 初始化SPI介面
  mfrc522.PCD_Init();        // 初始化MFRC522卡片

  Serial.println(F("Please scan MIFARE Classic card..."));

  // 準備金鑰（用於key A和key B），出廠預設為6組 0xFF。
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

}

void LoopRFID() {
  // 查看是否感應到卡片
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;  // 退回loop迴圈的開頭
  }

  // 選取一張卡片
  if ( ! mfrc522.PICC_ReadCardSerial()) {  // 若傳回1，代表已讀取到卡片的ID
    return;
  }

//  writeBlock(sector, block, blockData);  // 區段編號、區塊編號、包含寫入資料的陣列

  readBlock(sector, block, buffer);      // 區段編號、區塊編號、存放讀取資料的陣列
  Serial.print(F("Read block: "));        // 顯示儲存讀取資料的陣列元素值
  for (byte i = 0 ; i < 16 ; i++) {

  //Serial.write ("x");
    Serial.write (buffer[i]);
  }
  Serial.println();

  String myString = String((char *)buffer);
  
  hc05.println(myString);
  if (memcmp(buffer, blockData, 16) == 0) {
          Serial.println("Correct");  // 顯示標籤的名稱
  }
  // 令卡片進入停止狀態
  mfrc522.PICC_HaltA();

  mfrc522.PCD_StopCrypto1();
}
