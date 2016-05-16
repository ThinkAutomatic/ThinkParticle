# ThinkParticle
Particle library for integration with Think Automatic machine learning platform

## Getting Started

This library enables Particle.io devices such as the Photon to easily integrate with the Think Automatic machine learning platform. In order to keep this library small, simple and secure it communicates with the platform though a Node.js proxy called ThinkHub running on a machine on the same local network as the Particle device using this library. 

### Prerequisite

Install and run the ThinkHub proxy via npm and Node.js on any machine on the same LAN as your Particle device.
```
cd ~
mkdir ThinkHub
cd ThinkHub
npm install ThinkHub
```
Then to run it.
```
node .
```
The <a href="https://github.com/ThinkAutomatic/ThinkHub" target="_blank">ThinkHub</a> project uses the <a href="https://github.com/ThinkAutomatic/thinkdevice" target="_blank">thinkdevice</a> module which has instructions for associating devices such as the ThinkHub to a Think Automatic account which needs to be done before ThinkParticle devices can communicate through it.

### Using the ThinkParticle library

The ThinkParticle library is published through the Particle.io library publishing system. Simply search the community libraries for ThinkParticle. The provided example looks like the following and is designed to work with a modified Staples Easy Button although it will also work with any button input.
```
#include "application.h"
#include "ThinkParticle/ThinkParticle.h"

int led = D7; 

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
```
### Thanks

Special thanks to Matthew (m-mcgowan) and all the contributers to the Webduino webserver library which is included as part of this module. And thanks to Nils Mattisson (nmattisson) and all the contributers to the HttpClient library which served as a valuable reference.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
