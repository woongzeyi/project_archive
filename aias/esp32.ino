#include <HUSKYLENS.h>
#include <WiFi.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>

// Config
const char* ssid = "9626FIBER_5G";
const char* pwd = "Pooi#Wan.Liew633";
const char* server = "";
const int port = 5000;

const char* cl_id = "";

HUSKYLENS hl;
SoftwareSerial hl_serial(12, 13);
//green line >> Pin 12, blue line >> Pin 13

int ps_size = 0;
int* present_seats = nullptr;

void push_back(int*& arr, int& size, int value) {
  // Create a new temporary array with increased size
  int* temp = new int[size + 1];

  // Copy elements from the original array to the temporary array
  for (int i = 0; i < size; i++) {
    temp[i] = arr[i];
  }

  // Add the new value to the end of the temporary array
  temp[size] = value;

  // Delete the old array
  delete[] arr;

  // Update the array pointer and size
  arr = temp;
  size++;
}

bool contains(int* arr, int size, int value) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == value) {
      return true;  // Value found in the array
    }
  }
  return false;  // Value not found in the array
}

void sendHttpPostRequest(String seat, String class_id) {
  // Create an HTTPClient object
  HTTPClient http;

  // Specify the target server and port
  http.begin(server, port, "/");

  // Set content type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Define the data to be sent in the request body
  String data = "seat=" + seat + "&class_id=" + class_id;

  // Send the POST request and get the HTTP response
  int httpResponseCode = http.POST(data);

  // Check for a successful response
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    // Read and print the response body
    String response = http.getString();
    Serial.println("Response: " + response);
  } else {
    Serial.print("HTTP POST request failed, error code: ");
    Serial.println(httpResponseCode);
  }

  // Close the connection
  http.end();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  hl_serial.begin(9600);

  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }

  while (!hl.begin(hl_serial)) {
    Serial.println(F("Begin Failed"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }

  Serial.print("WiFi connected with IP:");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!hl.request() || !hl.isLearned() || !hl.available()) {
    return;
  }
  while (hl.available()) {
    HUSKYLENSResult result = hl.read();
    if (result.ID != 0 && contains(present_seats, ps_size, result.ID)) {
      sendHttpPostRequest(String(result.ID), String(cl_id));
      push_back(present_seats, ps_size, result.ID);
    }
  }
  delay(1000);
}
