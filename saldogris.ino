#include <U8g2lib.h>
#include <U8x8lib.h>

#define ARDUINOJSON_ENABLE_PROGMEM 1
#include <ArduinoJson.h>

#include <ssl_client.h>
#include <WiFiClientSecure.h>

#include <Base64.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ********* WiFi Setup ****************

char* ssid        = "";
char* password    = "";

// ********* Bank Setup ****************

// base64-encoded clientid:secret
const char* encodedCred   = "Basic [base64-encoded clientid:secret]";
// 11 digit ssn
const char* userId        = "";

const char* host          = "api.sbanken.no";
const int   httpsPort     = 443;
const char* tokenAddress  = "https://api.sbanken.no/identityserver/connect/token/";
const char* accountsAddress = "https://api.sbanken.no/identityserver/connect/token";

const int[] displayPositions = {0, 80, 70, 60, 50, 40, 30, 16, 10, 0}

int tokenCount = 0;

struct BankData {
  char token[1500];
  char disposable[32];
  char accountname[32];
  char cdisposable[32];
  char caccountname[32];
};

U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 18, 19);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  u8g2.begin();
}

void setup_wifi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected");
    } else if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("No shield");
    } else if (WiFi.status() == WL_IDLE_STATUS) {
      Serial.println("Idle");
    } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial.println("Network not found");
    } else if (WiFi.status() == WL_SCAN_COMPLETED) {
      Serial.println("Scan completed");
    } else if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed connecting");
    } else if (WiFi.status() == WL_CONNECTION_LOST) {
      Serial.println("Connection lost");
    } else if (WiFi.status() == WL_DISCONNECTED) {
      Serial.println("Disconnected");
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void printDataData(const struct BankData* bankData) {
  Serial.print("Token = ");
  Serial.println(bankData->token);
}

void getToken(struct BankData* bankData) {
  char result[250];

  HTTPClient http;
  http.begin("https://api.sbanken.no/identityserver/connect/token");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", encodedCred);
  http.addHeader("Accept", "application/json");
  http.POST("grant_type=client_credentials");
  //Serial.println("Result:");

  DynamicJsonBuffer jsonBuffer(2000);
  JsonObject& root = jsonBuffer.parseObject(http.getString());

  strcpy(bankData->token, root["access_token"]);
  //Serial.println(bankData->token);
}

void getDisposable(struct BankData* bankData) {
  HTTPClient http;
  http.begin("https://api.sbanken.no/Bank/api/v1/Accounts/" + userId);
  http.addHeader("Authorization", String("Bearer ") + bankData->token);
  http.addHeader("Accept", "application/json");
  http.GET();
  
  DynamicJsonBuffer jsonBuffer; //(2000);
  JsonArray& accounts = jsonBuffer.parseArray(http.getString()); //root;
  Serial.println("parsed"); 
  strcpy(bankData->disposable, accounts[0]["available"]);
  strcpy(bankData->accountname, accounts[0]["name"]);
  strcpy(bankData->cdisposable, accounts[1]["creditLimit"]);
  strcpy(bankData->caccountname, accounts[1]["name"]);
  Serial.print("Disposable: ");
  Serial.println(bankData->disposable);
}


void displayLogo(){
  int digits = 0;
  int yposition = 0;
  
  u8g2.firstPage();
  do {  
    u8g2.setFont(u8g2_font_logisoso24_tr);
    u8g2.drawStr(0,45,"S banken");
    u8g2.drawDisc(23, 20, 5, U8G2_DRAW_ALL);
  } while ( u8g2.nextPage() );
}


void displayDisposable(struct BankData* bankData){
  int digits = 0;
  int yposition = 0;
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.drawStr(0,20,bankData->accountname);
    u8g2.setFont(u8g2_font_logisoso22_tr);
    
    for (int i = 0; i < sizeof(bankData->disposable); i++) {
      if (bankData->disposable[i] == '.') {
        if (bankData->disposable[i+1] == '\0') {
          bankData->disposable[i+1] = 0;
        } else if (!(bankData->disposable[i+1] >= 0)){
          bankData->disposable[i+1] = 0;
        }

        if (bankData->disposable[i+2] == '\0') {
          bankData->disposable[i+2] = 0;
        } else if (!(bankData->disposable[i+2] >= 0)){
          bankData->disposable[i+2] = 0;
        }

        bankData->disposable[i+3] = '\0';
        digits = i + 3;
        break;
      }
    }
    
    if (digits < 10){
        yposition = displayPositions[digits + 1];
    } else {
        yposition = 0;
    }

    u8g2.drawStr(yposition,56, bankData->disposable);
  } while ( u8g2.nextPage() );
}


void displayCredit(struct BankData* bankData){
  int digits = 0;
  int yposition = 0;
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.drawStr(0,20,bankData->caccountname);
    u8g2.setFont(u8g2_font_logisoso22_tr);
    
    for (int i = 0; i < sizeof(bankData->cdisposable); i++) {
      if (bankData->cdisposable[i] == '.') {
        if (bankData->cdisposable[i+1] == '\0') {
          bankData->cdisposable[i+1] = 0;
        } else if (!(bankData->cdisposable[i+1] >= 0)){
          bankData->cdisposable[i+1] = 0;
        }

        if (bankData->cdisposable[i+2] == '\0') {
          bankData->cdisposable[i+2] = 0;
        } else if (!(bankData->cdisposable[i+2] >= 0)){
          bankData->cdisposable[i+2] = 0;
        }

        bankData->cdisposable[i+3] = '\0';
        digits = i + 3;
        break;
      }
    }
    
    if (digits < 10){
        yposition = displayPositions[digits + 1];
    } else {
        yposition = 0;
    }

    u8g2.drawStr(yposition,56, bankData->cdisposable);
  } while ( u8g2.nextPage() );
}


void loop() {
  BankData bankData;
  if (tokenCount > 40) {
    tokenCount = 0;
  }
  if (tokenCount == 0) {
    getToken(&bankData);
    tokenCount = tokenCount + 1;
  }
  displayLogo();
  delay(5000);
  getDisposable(&bankData);
  displayDisposable(&bankData);
  delay(5000);
  displayCredit(&bankData);
  delay(5000);
}