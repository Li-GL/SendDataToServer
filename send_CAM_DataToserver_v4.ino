/*2016-06-17 modified by Lane 
 1. Remove the system reboot function.
 2. Add one line to connect the server continuously
 3. Remove the client.stop within "else" in connectAgain function
 2016-08-18 modified by Lane 
 1. 加一句client.print("<data>"), 并且client.println替换为client.print
 
*/

#include <SPI.h>
#include <WiFi.h>
#include <SD.h>
const int chipSelect = 4;
boolean lastConnected = false;  
const unsigned long postingInterval = 20*1000;  // delay between updates, in milliseconds
unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
String incomingByte ="";
String filename = "";

//服务器的端口和IP
int port = 4203;
IPAddress server(23,99,101,82); 

WiFiClient client;

void setup() {
   
// Initialize serial and wait for port to open:注意接收数据的是Serial1，Serial用于Arduino串口调试
   Serial.begin(115200);
   Serial1.begin(115200);

//初始化SD卡
   pinMode(10, OUTPUT);
   if (!SD.begin(chipSelect)) {
    
	Serial.println("Card failed, or not present");
    
	// don't do anything more:
    return;
  }
  
   Serial.println("card initialized.");
  
// 准备连服务器
   delay(5000);
   Serial.println("\nStarting connection to server...");
   client.connect(server, port);
    
}


void loop() {
  
// Serial1 is related to Edison Arduino expansion 0RX and 0TX
  
  if (Serial1.available() > 0)  
  {
     //读取串口1（注意接CAM板的是Serial1）发来的数据
     incomingByte = Serial1.readStringUntil('\n');
     
     // Arduino 串口调试时用到
     Serial.println(incomingByte);
        
     // 往服务器发数据
     client.print("<data>");
     client.print(incomingByte);

//////////////////////////写入Edison 内存///////////////////////////////////
     
	 //命名文件
     filename = incomingByte.substring(26,36)+ "-" + incomingByte.substring(6,16)+".txt";
     char FileName[filename.length()+1];
     filename.toCharArray(FileName,sizeof(FileName));

     //抓开头为<data>的数据
     if(incomingByte.startsWith("<data>")){

       //创建文件并存储数据
       File dataFile = SD.open(FileName, FILE_WRITE);
       dataFile.println(incomingByte.substring(6));
       dataFile.close();
      }     
    
  }
  
   

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    
	Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    
	}
   
  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected()&&(millis() - lastConnectionTime > postingInterval)) {
       
    //重新连接wifi，然后再连服务器 
    connectAgain();  
 
  }

  //store the state of the connection for next time through the loop:
  
  lastConnected = client.connected();
    
  client.connect(server, port);

  }

///////////////////////////////////////////连不上服务器，先重启wifi//////////////////////////
  void connectAgain(){
   
   //重启wifi,等待10秒
   system("systemctl restart wpa_supplicant");
   delay(10000);
   
  // if there's a successful connection:
   if (client.connect(server, port)){
 
      Serial.println("now connected again");
   
      // note the time that the connection was made:
      lastConnectionTime = millis();
   }   

}
