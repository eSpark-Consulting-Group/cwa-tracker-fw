#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class GeoLocation {
private:
    // Replace with your network credentials
    const char* ssid = "eSpark Wifi";
    const char* password = "47356880";
    // Replace with your IPGeolocation API key
    const char* apiKey = "fced2abbf6be43928734bb3aa9389f77";
    // Replace with your API endpoint URL
    const char* baseUrl = "https://api.chipworksarabia.com";  // Base URL for the API

    String createJsonPayload(const char* deviceId, double latitude, double longitude);
    String createSignupPayload(const String& email, const String& password, const String& name);
    String createLoginPayload(const String& email, const String& password);

public:
    volatile long geoname_id;
    volatile double latitude;
    volatile double longitude;
    char coordinate[50];
    char* token;  // Variable to store the token

    void connectToWiFi();
    String getMacAddress();
    String fetchData();
    void parseData(const String& payload);
    void postData(const char* deviceId);

    void signup(const String& email, const String& password, const String& name);
    void login(const String& email, const String& password);
    void parseLoginResponse(const String& response);  
};