#include <U8g2lib.h>
#include <U8x8lib.h>

#define ARDUINOJSON_ENABLE_PROGMEM 1
#include <ArduinoJson.h>

#include <ssl_client.h>
#include <WiFiClientSecure.h>

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
const char* tokenAddress    = "https://api.sbanken.no/identityserver/connect/token/";
const char* accountsAddress = "https://api.sbanken.no/identityserver/connect/token";

const int numberOfAccountsToDisplay = 2;
const String accountsToDisplay[] = {
  "Sparekonto",
  "HOVEDKONTO"
};

int tokenCount = 0;

const int displayPositions[] = {0, 82, 72, 62, 52, 42, 32, 22, 12, 0};

struct BankData {
  char token[1500];
};

struct BankAccount {
  String accountNumber;
  String name;
  String disposable;
};

int numberOfAccounts = 0;
BankAccount bankAccounts[10];

//U8G2_UC1701_DOGS102_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 18, 19);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  u8g2.begin();
}

void setup_wifi() {
  int connectionAttempts = 0;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi not connected");
    //listNetworks();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected");
      Serial.print("WiFi connected - IP: ");
      Serial.println(WiFi.localIP());
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

    if (connectionAttempts > 30) {
      Serial.println("Could not connect to WiFi");
      return;
    }
  }
}

void getToken(struct BankData* bankData) {
  char result[250];

  HTTPClient http;
  http.begin("https://api.sbanken.no/identityserver/connect/token");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", "Basic " + encodedCred);
  http.addHeader("Accept", "application/json");
  http.POST("grant_type=client_credentials");
  Serial.println("Got token: ");

  DynamicJsonBuffer jsonBuffer(2000);
  JsonObject& root = jsonBuffer.parseObject(http.getString());

  strcpy(bankData->token, root["access_token"]);
  Serial.println(bankData->token);
}

void getDisposable(struct BankData* bankData) {
  HTTPClient http;
  http.begin("https://api.sbanken.no/Bank/api/v1/Accounts/" + userId);
  http.addHeader("Authorization", String("Bearer ") + bankData->token);
  http.addHeader("Accept", "application/json");
  http.GET();

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(http.getString());
  numberOfAccounts = root["availableItems"];
  for (int i = 0; i < root["items"].size(); i++) {
    Serial.println(root["items"][i]["name"].as<const char*>());
    bankAccounts[i].name = String(root["items"][i]["name"].as<const char*>());
    bankAccounts[i].disposable = String(root["items"][i]["available"].as<const char*>());
    bankAccounts[i].accountNumber = String(root["items"][i]["accountNumber"].as<const char*>());
  }
}

void displayLogo() {
  int digits = 0;
  int yposition = 0;

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso24_tr);
    u8g2.drawStr(0, 45, "S banken");
    u8g2.drawDisc(23, 20, 5, U8G2_DRAW_ALL);
  } while ( u8g2.nextPage() );
}

bool shouldDisplay(struct BankAccount* bankAccount){
  for(int i = 0; i < numberOfAccountsToDisplay; i++) {
    if(bankAccount->name == accountsToDisplay[i]){
      return true;
    }
  }
  return false;
}

void displayAccount(struct BankAccount* bankAccount) {
  int digits = 0;
  int yposition = 0;

  int decimalSignAt = bankAccount->disposable.lastIndexOf('.');

  if (decimalSignAt > 0 && decimalSignAt < bankAccount->disposable.length() - 2){
    bankAccount->disposable = bankAccount->disposable.substring(0, bankAccount->disposable.lastIndexOf('.') + 3);
  }

  char accountNameCA[sizeof(bankAccount->name)];
  bankAccount->name.toCharArray(accountNameCA, sizeof(bankAccount->name));

  char disposableCA[sizeof(bankAccount->disposable)];
  bankAccount->disposable.toCharArray(disposableCA, sizeof(bankAccount->disposable));
 
  u8g2.firstPage();
  do {
    /* all graphics commands have to appear within the loop body. */
    //u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.drawStr(0, 20, accountNameCA);
    u8g2.setFont(u8g2_font_logisoso22_tr);
    
    if (bankAccount->disposable.length() < 10) {
      yposition = displayPositions[bankAccount->disposable.length()];
    } else {
      yposition = 0;
    }

    u8g2.drawStr(yposition, 56, disposableCA);
  } while ( u8g2.nextPage() );
}


void loop() {
  setup_wifi();
  BankData bankData;
  if (tokenCount > 40) {
    tokenCount = 0;
  }
  if (tokenCount == 0) {
    getToken(&bankData);
    tokenCount = tokenCount + 1;
  }

  displayLogo();
  getDisposable(&bankData);
  delay(5000);

  for(int i = 0; i < numberOfAccounts; i++) {
    if(shouldDisplay(&bankAccounts[i])){
      displayAccount(&bankAccounts[i]);
      delay(5000); 
    }
  }
}
