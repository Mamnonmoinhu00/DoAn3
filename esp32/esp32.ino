//KHAI BAO THU VIEN VA CAC CHAN
#include <DHT.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "key.h"
#include "FirebaseESP32.h"
//**************************
#define WIFI_SSID String(SSID)
#define WIFI_PASSWORD String(PASSWORD)
//**************************
#define FIREBASE_HOST String(FB_HOST)
#define FIREBASE_AUTH String(FB_AUTH)
//
int SOIL_PIN = 35;	
int DHT11_PIN = 4;
int CBAS_PIN = 34;
DHT DHT(DHT11_PIN,DHT11);/*CAU HINH DHT11*/

/*CAC BIEN*/
//**************************
FirebaseData firebaseData;
String path= "/";

//Cam bien
float AmD;
float Am, Nhiet;
int As;
//Gioi han cai dat
float AmDGH;
float AmGH, NhietGH;
int AsGH;

//Thoi gian millis()
unsigned long MillisHienTai,MillisTruoc;
const unsigned long DoTre = 2000;//delay 1s

//JSON
StaticJsonDocument<200> doc;
StaticJsonDocument<200> doctt;


//SETUP
void setup() {
  Serial.begin(115200);
  //
  DHT.begin();	/*KHOI TAO DHT*/
  delay(1000);
  pinMode(CBAS_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  //KẾT NỐI VỚI WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //**************************
  //KẾT NỐI VỚI FIREBASE
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  if(!Firebase.beginStream(firebaseData, path)){
    Serial.println("Reason: "+firebaseData.errorReason());
    Serial.println();
  }
  Serial.println("Ket noi thanh cong");
  //**************************
  
}
//TEST**************
void loop() {
  MillisHienTai = millis();  // Lấy thời gian hiện tại
  if (MillisHienTai - MillisTruoc >= DoTre) {  // Kiểm tra nếu đã đủ thời gian chờ
    //LẤY THÔNG TIN CÀI ĐẶT TỪ FIREBASE
    if (Firebase.getJSON(firebaseData, "/setting")) {//từ setting
    // Kiểm tra nếu dữ liệu lấy về thành công
      if (firebaseData.dataType() == "json") {
        // Phân tích dữ liệu JSON
        FirebaseJson& json = firebaseData.jsonObject();
        String jsonString;
        json.toString(jsonString, true);
        deserializeJson(doc, jsonString);// chuyển sang chuỗi Json doc
        
      }
    }
    //Đọc giá trị cảm biến
    DocCamBien();
    //JSON doc các thông số
    doc["amd"] = AmD;
    doc["am"] = Am;
    doc["nhiet"] = Nhiet;
    doc["as"] = As;
    //
    String doc1;
    serializeJson(doc, doc1);
    Serial.println(doc1);//Gửi qua UART
    //Gửi lên firebase**************************
    FirebaseJson JsonGui;
    JsonGui.set("/amd",AmD);
    JsonGui.set("/am",Am);
    JsonGui.set("/nhiet",Nhiet);
    JsonGui.set("/as",As);
    // Gửi JSON lên Firebase
    Firebase.setJSON(firebaseData, "/data", JsonGui);
    if (Firebase.setJSON(firebaseData, "/data", JsonGui)) {
        Serial.println("json1 sent successfully");
    } else {
        Serial.print("Failed to send json1, reason: ");
        Serial.println(firebaseData.errorReason());
    }
    //Cập nhật thời gian trước đó
    MillisTruoc = MillisHienTai;  
  }
  //------------------------------------------------------------
  //đọc dữ liệu từ Arduino
  if (Serial.available()) {// Nếu có dữ liệu từ Serial
    // Đọc dữ liệu JSON từ UART
    String jsonStringtt = Serial.readStringUntil('\n');
    // Phân tích chuỗi JSON thành đối tượng JSON
    DeserializationError error = deserializeJson(doctt, jsonStringtt);
    if (error) {
      return;
    }
    JsonObject obj = doctt.as<JsonObject>();
    int tt1 = obj["tt1"];
    int tt2 = obj["tt2"];
    int tt3 = obj["tt3"];
    int tt4 = obj["tt4"];
    FirebaseJson JsonGuitt;
    JsonGuitt.set("/tt1",tt1);
    JsonGuitt.set("/tt2",tt2);
    JsonGuitt.set("/tt3",tt3);
    JsonGuitt.set("/tt4",tt4);
    // Gửi JSON trạng thái lên Firebase
    Firebase.setJSON(firebaseData, "/tt", JsonGuitt);
    
    // gửi lên firebase
  }
}
void DocCamBien(){
  //DOC DO AM DAT
  int AnalogAmD;
  AnalogAmD = analogRead(SOIL_PIN);
  AmD = ( (4095 - AnalogAmD)*100/4095.00);
  
  //DOC NHIET DO, DO AM
  Am = DHT.readHumidity();
  Nhiet = DHT.readTemperature();

  //DOC CUONG DO ANH SANG
  As = ((4000-analogRead(CBAS_PIN))*100)/4000;
}