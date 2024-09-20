#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <ESP32Servo.h>
#include <WiFiUdp.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define BUZZER 5
#define LED_1 16
#define LED_2 4
#define PB_CANCEL 34
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35
#define DHTPIN 15

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET 19800  // 5.5 hours * 3600 seconds/hour
#define UTC_OFFSET_DST 0  // No daylight saving time


int utc_offset = 0;
int utc_offset_dst = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

//Global variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

unsigned long timeNow = 0;
unsigned long timeLast = 0;

bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[] = {0, 1, 2};
int alarm_minutes[] = {1, 10, 20};
bool alarm_triggered[] = {false, false, false};

int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

int current_mode = 0;
int max_modes = 5;
String modes[] = {"1 - Set Time Zone", "2 - Set Alarm 1", "3 -  Set Alarm 2", "4 -  Set Alarm 3", "5 - Disable Alarms"};

/////Asignment_2_updated

WiFiClient espClient;
PubSubClient mqttClient(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

char temperature_Arr[6];
char ldr_Left_Arr[4];
char ldr_Right_Arr[4];

bool isScheduledON = false;
unsigned long scheduledOnTime;

Servo servo;

int theta_offset = 30;
float gamma_i = 0.75;
int theta=0;


#define LDR_LEFT 39
#define LDR_RIGHT 36

/////Asignment_2_updated

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22);

  Serial.begin(9600);
  // Initialize the OLED display using Wire library
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("Connecting to WIFI", 0, 0, 2);
  }

  display.clearDisplay();
  print_line("Connected to WIFI", 0, 0, 2);

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

  display.display();
  delay(2000); // Pause for 2 seconds

  /////Asignment_2_updated
  setupMqtt();

  timeClient.begin();
  timeClient.setTimeOffset(5.5 * 3600);

  servo.attach(19);

  /////Asignment_2_updated


  // Clear the buffer
  display.clearDisplay();
  print_line("Welcome to Medibox!", 0, 20, 2);
  display.clearDisplay();

}

void loop() {
  
  /////Asignment_2_updated
  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker");
    connectTOBroker();
  }

  mqttClient.loop();

  mqttClient.publish("Temperature_Input_210204R", temperature_Arr);
  delay(50);

  updateLight();

  mqttClient.publish("LDR_LEFT_210204R", ldr_Left_Arr);
  delay(50);
  mqttClient.publish("LDR_RIGHT_210204R", ldr_Right_Arr);
  delay(100);
  
  /////Asignment_2_updated


  // Main code here to run repeatedly:
  update_time_with_check_alarm();
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }
  check_temp();
  Serial.println("Minimum Angle- " + String(theta_offset)+" Controlling factor-"+String(gamma_i));
  Serial.println("Servo Angle-" + String(theta));
}

void print_line(String text, int column, int row, int text_size) {
  //display.clearDisplay();

  display.setTextSize(text_size);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(column, row);            // Start at top-left corner

  // Display the message
  display.println(text);

  display.display();

}

// function to display the current time DD:HH:MM:SS at a given position
void print_time_now(void) {
  display.clearDisplay();
  print_line(String(hours) + ":" + String(minutes) + ":" + String(seconds), 0, 10, 2);
}

// function to automatically update the current time
void update_time(void) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    char day_str[8];
    char hour_str[8];
    char min_str[8];
    char sec_str[8];

    strftime(day_str, 8, "%d", &timeinfo);
    strftime(sec_str, 8, "%S", &timeinfo);
    strftime(hour_str, 8, "%H", &timeinfo);
    strftime(min_str, 8, "%M", &timeinfo);

    hours = atoi(hour_str);
    minutes = atoi(min_str);
    days = atoi(day_str);
    seconds = atoi(sec_str);
}


// function to automatically update the current time while checking for alarms
void update_time_with_check_alarm() {
  update_time();
  print_time_now();

  // check for alarms
  if (alarm_enabled) {
    // iterating through all alarms
    for (int i = 0; i < n_alarms; i++) {
      if (alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]) {
        ring_alarm(); // call the ringing function
        alarm_triggered[i] = true;
      }
    }
  }
}

// ring an alarm
void ring_alarm() {
  // Show message on display
  display.clearDisplay();
  print_line("Medicine Time", 0, 0, 2);

  bool break_happened = false;

  // light up LED 1
  digitalWrite(LED_1, HIGH);
  while (break_happened == false && digitalRead(PB_CANCEL) == HIGH) {
    // ring the buzzer
    for (int i = 0; i < n_notes; i++) {
      if ((digitalRead(PB_CANCEL) == LOW)) {
        delay(200);
        break_happened = true;
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }



  digitalWrite(LED_1, LOW);
  display.clearDisplay();
}

// function to wait for button press in the menu
int wait_for_button_press() {
  while (true) {
    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    } else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    } else if (digitalRead(PB_CANCEL) == LOW) {
      delay(200);
      return PB_CANCEL;
    } else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    }
    update_time(); // Keep updating time while waiting for input
  }
}


// function to navigate through the menu
void go_to_menu(void) {
  while (digitalRead(PB_CANCEL) == HIGH) {
    display.clearDisplay();
    print_line(modes[current_mode], 0, 0, 2);
    int pressed = wait_for_button_press();

    if (pressed == PB_UP) {
      current_mode += 1;
      current_mode %= max_modes;
      delay(200);
    } else if (pressed == PB_DOWN) {
      current_mode -= 1;
      if (current_mode < 0) {
        current_mode = max_modes - 1;
      }
      delay(200);
    } else if (pressed == PB_OK) {
      delay(200);
      run_mode(current_mode);
    } else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }
}

void run_mode(int mode) {
  if (mode == 0) {
    set_time_zone();
  } else if (mode == 1 || mode == 2| mode == 3) {
    set_alarm(mode - 1);
  } else if (mode == 4) {
    alarm_enabled = false;
  }
}

void set_time_zone() {
  // Assuming utc_offset is now the total offset in minutes.
  int temp_utc_offset = utc_offset; // UTC offset in minutes
  display.clearDisplay();
  print_line("Set UTC offset (HH:MM):", 0, 0, 1);
  while (true) {
    int offset_hours = temp_utc_offset / 60;
    int offset_minutes = abs(temp_utc_offset % 60); // Use absolute value for minutes
    display.setCursor(0, 16);
    display.printf("%+03d:%02d", offset_hours, offset_minutes); // Format as +HH:MM or -HH:MM
    display.display();

    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      temp_utc_offset += 30; // increment the offset by 30 minutes
      delay(200);
    } else if (pressed == PB_DOWN) {
      temp_utc_offset -= 30; // decrement the offset by 30 minutes
      delay(200);
    } else if (pressed == PB_OK) {
      utc_offset = temp_utc_offset;
      configTime(utc_offset * 60, utc_offset_dst * 60, NTP_SERVER); // Convert minutes to seconds for configTime
      display.clearDisplay();
      print_line("Time zone set", 0, 16, 1);
      delay(1000);
      break;
    } else if (pressed == PB_CANCEL) {
      break;
    }
    display.clearDisplay();
    print_line("Set UTC offset (HH:MM):", 0, 0, 1);
  }
  display.clearDisplay();
}

void set_alarm(int alarm) {
  int temp_hour = alarm_hours[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    } else if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    } else if (pressed == PB_OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    } else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }
  int temp_minute = alarm_minutes[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);

    int pressed = wait_for_button_press();
    if (pressed == PB_UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    } else if (pressed == PB_DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    } else if (pressed == PB_OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    } else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  print_line("Alarm is set", 0, 0, 2);
  delay(1000);
}

void check_temp(void) {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  /////Asignment_2_updated
  String(data.temperature, 2).toCharArray(temperature_Arr, 6);
  /////Asignment_2_updated
  bool all_good = true;

  if (data.temperature > 32) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP HIGH", 0, 40, 1);
  } else if (data.temperature < 26) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP LOW", 0, 40, 1);
  }

  if (data.humidity > 80) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD HIGH", 0, 50, 1);
  } else if (data.humidity < 60) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD LOW", 0, 50, 1);
  }

  if (all_good) {
    digitalWrite(LED_2, LOW);
  }
}

/////Asignment_2_updated

void setupMqtt() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallback);
}

void connectTOBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client-sdfsjdfsdfsdf")) {
      Serial.println("MQTT Connected");
      mqttClient.subscribe("ENTC-ON-OFF_NI");
      mqttClient.subscribe("ENTC-ADMIN-SCH-ON_NI");
      mqttClient.subscribe("Servo_Angle_210204R");
      mqttClient.subscribe("Controlling_factor_210204R");
    } else {
      Serial.print("Failed To connect to MQTT Broker");
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}

void recieveCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  Serial.print("Message Recieved: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();
  if (strcmp(topic, "ENTC-ON-OFF_NI") == 0) {
    if (payloadCharAr[0] == '1') {
      digitalWrite(15, HIGH);
    } else {
      digitalWrite(15, LOW);
    }
  } else if (strcmp(topic, "ENTC-ADMIN-SCH-ON_NI") == 0) {
    if (payloadCharAr[0] == 'N') {
      isScheduledON = false;
    } else {
      isScheduledON = true;
      scheduledOnTime = atol(payloadCharAr);
    }
  } else if (strcmp(topic, "Servo_Angle_210204R") == 0) {
    theta_offset = String(payloadCharAr).toInt();

  } else if (strcmp(topic, "Controlling_factor_210204R") == 0) {
    gamma_i = String(payloadCharAr).toFloat();
  }
}

unsigned long getTime() {
  timeClient.update();
  return timeClient.getEpochTime();
}

void updateLight() {

  float left_ldr_value = analogRead(LDR_LEFT) * 1.00;
  float right_ldr_value = analogRead(LDR_RIGHT) * 1.00;

  float left_ldr_value_cha = (float)(left_ldr_value - 4063.00) / (32.00 - 4063.00);
  float right_ldr_value_cha = (float)(right_ldr_value - 4063.00) / (32.00 - 4063.00);
  //Serial.println("Left LDR-" + String(left_ldr_value_cha)+" Right LDR-"+String(right_ldr_value_cha));

  updateAngle(left_ldr_value_cha, right_ldr_value_cha);

  String(left_ldr_value_cha).toCharArray(ldr_Left_Arr, 4);
  String(right_ldr_value_cha).toCharArray(ldr_Right_Arr, 4);
}

void updateAngle(float left_ldr_value, float right_ldr_value) {
  float max_I = left_ldr_value;
  float D = 1.5;

  if (right_ldr_value > max_I) {
    max_I = right_ldr_value;
    D = 0.5;
  }

  theta = theta_offset * D + (180 - theta_offset) * max_I * gamma_i;
  theta = min(theta, 180);

  servo.write(theta);
}

/////Asignment_2_updated