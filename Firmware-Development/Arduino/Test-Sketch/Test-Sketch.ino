#include"GeoLocation.hpp"

#define INTERVAL 5000   //5sec
int previousInterval = 0;
GeoLocation geoLocation;

void setup() {
    Serial.begin(115200);
  
    // Connect to Wi-Fi
    geoLocation.connectToWiFi();

    // Fetch geolocation data
    String geoLocationData = geoLocation.fetchData();
  
    // Parse geolocation data
    geoLocation.parseData(geoLocationData);
  
    // Post the data to the API
    
    geoLocation.login("tabish@esparkconsultants.com", "Tabish@eSpark");

    // You can use the token here, for example:
    Serial.print("Token after login: ");
    Serial.println(geoLocation.token);

    

}

void loop() {
    // No repeating code in the loop
    while ((millis() - previousInterval) > INTERVAL)
    {
      geoLocation.postData(geoLocation.getMacAddress().c_str());
      previousInterval = millis();
    }
}
