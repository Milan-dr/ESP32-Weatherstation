#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

const char ssid[] = "";    // Network SSID (name)
const char pass[] = "";    // Network password (use for WPA, or use as key for WEP)
const String discord_webhook = "";
const String discord_tts = "false";
//openssl s_client -showcerts -connect discordapp.com:443 (get last certificate)
const char* discordappCertificate = \
                                    "-----BEGIN CERTIFICATE-----\n"
                                    "MIIDzTCCArWgAwIBAgIQCjeHZF5ftIwiTv0b7RQMPDANBgkqhkiG9w0BAQsFADBa\n"
                                    "MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl\n"
                                    "clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw\n"
                                    "MDEyNzEyNDgwOFoXDTI0MTIzMTIzNTk1OVowSjELMAkGA1UEBhMCVVMxGTAXBgNV\n"
                                    "BAoTEENsb3VkZmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkZmxhcmUgSW5jIEVD\n"
                                    "QyBDQS0zMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEua1NZpkUC0bsH4HRKlAe\n"
                                    "nQMVLzQSfS2WuIg4m4Vfj7+7Te9hRsTJc9QkT+DuHM5ss1FxL2ruTAUJd9NyYqSb\n"
                                    "16OCAWgwggFkMB0GA1UdDgQWBBSlzjfq67B1DpRniLRF+tkkEIeWHzAfBgNVHSME\n"
                                    "GDAWgBTlnVkwgkdYzKz6CFQ2hns6tQRN8DAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0l\n"
                                    "BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwNAYI\n"
                                    "KwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j\n"
                                    "b20wOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL09t\n"
                                    "bmlyb290MjAyNS5jcmwwbQYDVR0gBGYwZDA3BglghkgBhv1sAQEwKjAoBggrBgEF\n"
                                    "BQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzALBglghkgBhv1sAQIw\n"
                                    "CAYGZ4EMAQIBMAgGBmeBDAECAjAIBgZngQwBAgMwDQYJKoZIhvcNAQELBQADggEB\n"
                                    "AAUkHd0bsCrrmNaF4zlNXmtXnYJX/OvoMaJXkGUFvhZEOFp3ArnPEELG4ZKk40Un\n"
                                    "+ABHLGioVplTVI+tnkDB0A+21w0LOEhsUCxJkAZbZB2LzEgwLt4I4ptJIsCSDBFe\n"
                                    "lpKU1fwg3FZs5ZKTv3ocwDfjhUkV+ivhdDkYD7fa86JXWGBPzI6UAPxGezQxPk1H\n"
                                    "goE6y/SJXQ7vTQ1unBuCJN0yJV0ReFEQPaA1IwQvZW+cwdFD19Ae8zFnWSfda9J1\n"
                                    "CZMRJCQUzym+5iPDuI9yP+kHyCREU3qzuWFloUwOxkgAyXVjBYdwRVKD05WdRerw\n"
                                    "6DEdfgkfCv4+3ao8XnTSrLE=\n"
                                    "-----END CERTIFICATE-----\n";



WiFiMulti WiFiMulti;

void connectWIFI() {
Serial.print( "Connecting to Wi-Fi..." );
  // Loop until WiFi connection is successful
    while ( WiFi.status() != WL_CONNECTED ) {
      WiFi.begin( ssid, pass );
      delay( 1000 );
      Serial.print( WiFi.status() ); 
  }
  Serial.println( "Connected to Wi-Fi." );
}

void sendDiscord(String content) {
  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client -> setCACert(discordappCertificate);
    {
      HTTPClient https;
      //Serial.println("[HTTP] Connecting to Discord...");
      //Serial.println("[HTTP] Message: " + content);
      //Serial.println("[HTTP] TTS: " + discord_tts);
      if (https.begin(*client, discord_webhook)) {  // HTTPS
        // start connection and send HTTP header
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST("{\"content\":\"" + content + "\",\"tts\":" + discord_tts +"}");

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
          }
        } else {
          Serial.print("[HTTP] Post... failed, error: ");
          Serial.println(https.errorToString(httpCode).c_str());
        }

        https.end();
      } else {
        Serial.printf("[HTTP] Unable to connect\n");
      }

      // End extra scoping block
    }

    delete client;
  } else {
    Serial.println("[HTTP] Unable to create client");
  }
}