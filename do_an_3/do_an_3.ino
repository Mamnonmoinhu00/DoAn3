//KHAI BAO THU VIEN VA CAC CHAN
#include <ArduinoJson.h>
#define MAYBOM 8
#define QUAT 9
#define DEN 10
#define IN4 11
#define ChoPhepBom 13 
/*CAC BIEN*/
//Cam bien
float AmD;
float Am, Nhiet;
int As;
//Gioi han cai dat
float AmDGH;
float AmGH, NhietGH;
int AsGH;
//JSON
StaticJsonDocument<200> doc;
StaticJsonDocument<200> doctt;
//Trạng thái
int tt1,tt2,tt3,tt4;
//Lưu lượng
volatile int xung_cbll; // Đo xung cảm biến lưu lượng
unsigned int l_min; // Tính toán số lít/giờ
unsigned char port_cbll = 2; // Cảm biến nối với chân 2
unsigned long currentTime;//thời điểm hiện tại
unsigned long cloopTime;//thời điểm trước
unsigned long onTime;//Thời điểm bật nước
//biến cho phép bật máy bơm
int MayBat=1;
void dem_xung () // Hàm ngắt
{
   xung_cbll++;
}

void setup() {
  Serial.begin(115200); // Khởi tạo UART
  //Các thiết bị
  pinMode(MAYBOM, OUTPUT);
  pinMode(QUAT, OUTPUT);
  pinMode(DEN, OUTPUT);
  pinMode(IN4, OUTPUT);
  digitalWrite(MAYBOM,1);
  digitalWrite(QUAT,1);
  digitalWrite(DEN,1);
  digitalWrite(IN4,1);
  //
  pinMode(ChoPhepBom, INPUT_PULLUP);
  //lưu lượng
  pinMode(port_cbll, INPUT);
  digitalWrite(port_cbll, HIGH); // Tùy chỉnh Pull-Up
  attachInterrupt(0, dem_xung, RISING); // Cài đặt ngắt
  sei(); // cho phép ngắt
  currentTime = millis();
  cloopTime = currentTime;
}

void loop() {
  /*Đọc dữ liệu thông qua UART*/
  if (Serial.available()) {// Nếu có dữ liệu từ Serial
    // Đọc dữ liệu JSON từ UART
    String jsonString = Serial.readStringUntil('\n');
    // Phân tích chuỗi JSON thành đối tượng JSON
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
      return;
    }
    
    // Xử lý dữ liệu JSON
    JsonObject obj = doc.as<JsonObject>();
    // Đọc dữ liệu
    AmD = obj["amd"];
    Am = obj["am"];
    Nhiet = obj["nhiet"];
    As = obj["as"];
    AmDGH = obj["amdgh"];
    AmGH = obj["amgh"];
    NhietGH = obj["nhietgh"];
    AsGH = obj["asgh"];
    //Kiểm tra trạng thái sau khi cập nhập dữ liệu
    DieuChinhTT();
    //JSON doc trạng thái
    doctt["tt1"] = (!MayBat)?2:tt1;
    doctt["tt2"] = tt2;
    doctt["tt3"] = tt3;
    doctt["tt4"] = tt4;
    String doc1;
    serializeJson(doctt, doc1);
    Serial.println(doc1);//Gửi qua UART
  }
  LuuLuongNuoc();
  if(!digitalRead(ChoPhepBom)){MayBat=1;}
  //Kiểm tra trạng thái sau khi cập nhập dữ liệu
  DieuChinhTT();
  /*Điều khiển relay*/
  DieuKhienRelay();
}
/*--------------------------Các hàm-------------------------------*/


void DieuChinhTT(){
  if(MayBat){//cho phép máy bật
    if(!tt1){
      if(AmDGH>AmD){
        tt1 = 1;
        onTime=currentTime;
      }
    }else{
      if(currentTime - onTime >= 10000){
        if(l_min==0){
          tt1=0;
          MayBat=0;//Không cho phép bật nữa
        }
        onTime=currentTime;
      }
      if(AmDGH<=AmD){
          tt1=0;
      }
    }
  }else{
    tt1=0;
  }

  tt2=((NhietGH<Nhiet)||(AmGH>Am))?1:0;
  tt3=(As<AsGH)?1:0;
}
void DieuKhienRelay(){
  //Điều khiển máy bơm
  digitalWrite(MAYBOM,!tt1);
  //Điều khiển quạt
  digitalWrite(QUAT,!tt2);
  //Điều khiển đèn
  digitalWrite(DEN,!tt3);
  //IN4
  digitalWrite(IN4,!tt4);
}
void LuuLuongNuoc(){
  currentTime = millis();
  // Every min, calculate and print litres/hour
  if(currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime; // Updates cloopTime
    // Tần số xung (Hz) = 7.5Q, Q là lưu lượng dòng L/min.
    l_min = (xung_cbll/ 7.5); // (Tần số xung) / 7.5 = Q (lưu lượng dòng L/min)
    //
    xung_cbll = 0; // Reset số xung
  }
}