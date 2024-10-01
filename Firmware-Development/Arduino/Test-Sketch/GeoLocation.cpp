#include"GeoLocation.hpp"

void GeoLocation::connectToWiFi() {
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
  
    Serial.println("\nConnected to Wi-Fi");
}

String GeoLocation::getMacAddress() 
{
    String macAddress = WiFi.macAddress();
    Serial.print("MAC Address: ");
    Serial.println(macAddress);
    return macAddress;
}

String GeoLocation::fetchData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = "https://api.ipgeolocation.io/ipgeo?apiKey=" + String(apiKey);
      
        http.begin(url); // Specify the URL
        int httpResponseCode = http.GET(); // Send the request

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Geolocation Data:");
            Serial.println(payload);
            http.end(); // Free resources
            return payload;
        } 
        else 
        {
            Serial.print("Error on HTTP request: ");
            Serial.println(httpResponseCode);
            http.end(); // Free resources
            return "No payload";
        }
    } 
    else 
    {
        Serial.println("Wi-Fi not connected");
        return "Wi-Fi not connected";
    }
}

void GeoLocation::postData(const char* deviceId) {
    if (WiFi.status() == WL_CONNECTED) 
    {
        HTTPClient http;
        String url = String(baseUrl) + "/tracker"; // Tracker URL
        http.begin(url);  // The URL to your server

        // Set content type to JSON
        http.addHeader("Content-Type", "application/json");

        // Add Authorization header with the token
        if (token != nullptr) 
        {
            String authHeader = "Bearer ";
            authHeader += token;  // Append the token
            http.addHeader("Authorization", authHeader);
        } 
        else 
        {
            Serial.println("Token is not available!");
            return;  // Exit the method if the token is not available
        }

        // Create the JSON payload
        String requestBody = createJsonPayload(deviceId, latitude, longitude);  

        // Print the request body for debugging
        Serial.println("POST request body:");
        Serial.println(requestBody);

        // Send the POST request
        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            Serial.println("Response: ");
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();  // Free the resources
    } else {
        Serial.println("Wi-Fi not connected. Can't send data.");
    }
}

void GeoLocation::parseData(const String& payload) {
    const size_t capacity = 2 * JSON_OBJECT_SIZE(40) + 1024;  // Adjusted size based on the larger JSON structure
    DynamicJsonDocument doc(capacity);

    // Parse the JSON string
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Extract values
    geoname_id = doc["geoname_id"];          
    latitude = doc["latitude"].as<double>();  
    longitude = doc["longitude"].as<double>();

    // Print the coordinates as a formatted JSON string
    snprintf(coordinate, sizeof(coordinate), "[%.6f, %.6f]", longitude, latitude);
    Serial.println(coordinate); 
}

void GeoLocation::signup(const String& email, const String& password, const String& name) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(baseUrl) + "/signup"; // Signup URL
        http.begin(url);

        http.addHeader("Content-Type", "application/json");

        // Create the JSON payload
        String requestBody = createSignupPayload(email, password, name);

        Serial.println("Signup request body:");
        Serial.println(requestBody);

        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.print("Signup HTTP Response code: ");
            Serial.println(httpResponseCode);
            Serial.println("Response: ");
            Serial.println(response);
        } else {
            Serial.print("Error on signup POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("Wi-Fi not connected. Can't send data.");
    }
}

void GeoLocation::login(const String& email, const String& password) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(baseUrl) + "/login"; // Login URL
        http.begin(url);

        http.addHeader("Content-Type", "application/json");

        // Create the JSON payload
        String requestBody = createLoginPayload(email, password);

        Serial.println("Login request body:");
        Serial.println(requestBody);

        int httpResponseCode = http.POST(requestBody);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.print("Login HTTP Response code: ");
            Serial.println(httpResponseCode);
            Serial.println("Response: ");
            Serial.println(response);
            GeoLocation::parseLoginResponse(response);
        } else {
            Serial.print("Error on login POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("Wi-Fi not connected. Can't send data.");
    }
}

String GeoLocation::createSignupPayload(const String& email, const String& password, const String& name) {
    DynamicJsonDocument doc(256);
    doc["email"] = email;
    doc["password"] = password;
    doc["name"] = name;

    String requestBody;
    serializeJson(doc, requestBody);
    
    return requestBody;
}

String GeoLocation::createLoginPayload(const String& email, const String& password) {
    DynamicJsonDocument doc(256);
    doc["email"] = email;
    doc["password"] = password;

    String requestBody;
    serializeJson(doc, requestBody);
    
    return requestBody;
}

String GeoLocation::createJsonPayload(const char* deviceId, double latitude, double longitude) {
    // Create a JSON document for the payload
    DynamicJsonDocument doc(256);

    // Create the main JSON object
    doc["deviceId"] = deviceId;
    doc["deviceName"] = "ESP32"; // Hardcoded device name

    // Create a nested object for location
    JsonObject location = doc.createNestedObject("location");

    // Create a nested array for coordinates
    JsonArray coordinates = location.createNestedArray("coordinates");
    coordinates.add(latitude);  
    coordinates.add(longitude);   

    // Serialize JSON document to string
    String requestBody;
    serializeJson(doc, requestBody);
    
    return requestBody;
}

void GeoLocation::parseLoginResponse(const String& response) {
    // Allocate memory for token
    if (token != nullptr) {
        free(token); // Free previous token if exists
    }
    token = (char*)malloc(256);  // Allocate enough memory for the token

    const size_t capacity = JSON_OBJECT_SIZE(10) + 512;  
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, response);
    if (error) {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Extract the token from the response
    const char* extractedToken = doc["data"]["token"];
    strncpy(token, extractedToken, 255);  // Copy token to the member variable
    token[255] = '\0';  // Ensure null termination

    Serial.print("Extracted token: ");
    Serial.println(token);
}