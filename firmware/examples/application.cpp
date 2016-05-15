#include "application.h"
#include "ThinkParticle/ThinkParticle.h"

int led = D7; // Instead of writing D0 over and over again, we'll write led1
// You'll need to wire an LED to this one to see it blink.

int button = D6; 
bool buttonPressed = FALSE;

void postCallback(String name, String value)
{
    if (name == "LED") 
    {
        if (value == "on")
        {
            digitalWrite(led, HIGH);
        }
        if (value == "off")
        {
            digitalWrite(led, LOW);
        }
    }
}

// This sets the name of the device, type of device (by guid), and callback for processing
// commands from the platform. To browse existing device types or to create a new one
// go to https://app.thinkautomatic.io/devicetypes
ThinkDevice thinkDevice("Easy Button", "51c68fea-2d6f-46e9-b4d7-0cf772a10920", postCallback);

void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();

  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type != WebServer::HEAD)
  {
    /* this defines some HTML text in read-only memory aka PROGMEM.
     * This is needed to avoid having the string copied to our limited
     * amount of RAM. */
    P(message) = 
"<!DOCTYPE>"
"<html>"
  "<body>"
  "<center>"
  "<br>"
  "<br>"
  "<br>"
  "<form action=\"/think\" method=\"POST\">"
    "Tell your device what to do!<br>"
    "<br>"
    "<input type=\"radio\" name=\"LED\" value=\"on\">Turn the LED on."
    "<br>"
    "<input type=\"radio\" name=\"LED\" value=\"off\">Turn the LED off."
    "<br>"
    "<br>"
    "<input type=\"submit\" value=\"Do it!\">"
  "</form>"
  "</center>"
  "</body>"
"</html>";
    server.printP(message);
  }
}

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(button, INPUT_PULLUP);

   // For good measure, let's also make sure LED is off when we start:
  digitalWrite(led, LOW);

  thinkDevice.setDefaultCommand(&defaultCmd);

  thinkDevice.begin();
}

void loop()
{
  thinkDevice.process();
  
  if ((digitalRead(button) == LOW) && !buttonPressed) 
  {
      buttonPressed = TRUE;
      thinkDevice.patch("Button", "activate");
  }
  else if ((digitalRead(button) == HIGH) && buttonPressed)
  {
      buttonPressed = FALSE;
  }
}
