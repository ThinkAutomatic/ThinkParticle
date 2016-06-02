// This #include statement was automatically added by the Particle IDE.
#include "ThinkParticle.h"

#define PREFIX ""
#define PORT 3205

bool g_keepAliveFlag = TRUE;

void setKeepAliveFlag()
{
  g_keepAliveFlag = TRUE;
}

ThinkDevice *ThinkDevice::s_thinkDevice;

String ThinkDevice::httpEncode(String s)
{
  uint i;
  String result = "";
    
  for (i = 0; i < s.length(); i++)
  {
    if (s[i] == ' ')
      result += "%20";
    else
      result += s[i];
  }
  return result;
}

void ThinkDevice::handleParams(String name, String value)
{
  m_connected = true;
  
  if (name == "name")
    m_deviceName = value;
  else if (name == "deviceId")
    m_deviceId = value;
  else if (name == "hubIp")
    m_hubIp = value;  
  else if (name == "hubPort")
    m_hubPort = atoi(value);
  else
    m_thinkCallback(name, value);
}

String ThinkDevice::deviceConf()
{
    String result;
    String myIDStr = Particle.deviceID();

    result = "{ \"deviceTypeUuid\": \"";
    result += m_deviceTypeUuid;
    result += "\", \"uid\": \"";
    result += myIDStr;
    if (m_deviceId != "") 
    {
      result += "\", \"deviceId\": \"";
      result += m_deviceId;
    }
    if (m_deviceName != "") 
    {
      result += "\", \"name\": \"";
      result += m_deviceName;
    }
    if (m_hubIp != "")
    {
      result += "\", \"hubIp\": \"";
      result += m_hubIp;
      result += "\", \"hubPort\": \"";
      result += m_hubPort;
    }
      
    result += "\" }";
    
    return result;
}


void ThinkDevice::sWebCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
    s_thinkDevice->webCmd(server, type, url_tail, tail_complete);
}

void ThinkDevice::webCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  bool repeat;
  bool connected = (m_hubIp != "");
  char name[32], value[32];

  if (type == WebServer::POST)
  {
    if (m_thinkCallback) 
    {
      do
      {
        repeat = server.readPOSTparam(name, sizeof(name), value, sizeof(value));
        handleParams(name, value);
      } while (repeat);
    }
    
    server.httpSeeOther("/");
  }
  else
  {
    server.httpSuccess();
    
    if (type == WebServer::GET)
    {
      if (m_thinkCallback)
      {
        while (URLPARAM_EOS != server.nextURLparam(&url_tail, name, sizeof(name), value, sizeof(value)))
        {
          handleParams(name, value);
        }
      }
        
      server.printP(deviceConf());
    }    
  }
  
  if (!connected && (m_hubIp != ""))
    setKeepAliveFlag();
}


void ThinkDevice::sWebLink(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
    s_thinkDevice->webLink(server, type, url_tail, tail_complete);
}

void ThinkDevice::webLink(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  bool success = false;
  char name[128], value[128];
  char successRedirect[128];
  
  memset(successRedirect, 0, sizeof(successRedirect));

  if (type == WebServer::GET)
  {
    String myIDStr = Particle.deviceID();

    if (m_thinkCallback && m_hubIp != "" && m_deviceId != "")
    {
      while (URLPARAM_EOS != server.nextURLparam(&url_tail, name, sizeof(name), value, sizeof(value)))
      {
        if (String(name) == "linkToken")
        {
          patch_helper("link", name, value);
          // patch(name, value);
          success = true;
        }
        else if (String(name) == "successRedirect")
        {
          memcpy(successRedirect, value, sizeof(value));
        }
      }
      
      if (success)
      {
        if (successRedirect[0])
        {
          server.httpSeeOther(successRedirect);
        }
        else
        {
          server.httpSuccess();
          server.printP("<!DOCTYPE><html><body><center><br><br><p>linkToken passed on to ThinkHub but no successRedirect supplied</p></center></body></html>");
        }
      }
      else
      {
        server.httpSuccess();
        server.printP("<!DOCTYPE><html><body><center><br><br><p>Error: no linkToken supplied</p></center></body></html>");
      }
    }    
  }
}


ThinkDevice::ThinkDevice(String deviceName, String deviceTypeUuid, ThinkCallback *thinkCallback):
  WebServer(PREFIX, PORT),
  m_deviceId(""),
  m_deviceTypeUuid(deviceTypeUuid),
  m_hubIp(""),
  m_hubPort(0),
  m_timer(15 * 60 * 1000, setKeepAliveFlag)
{
  m_deviceName = deviceName;
  m_thinkCallback = thinkCallback;
  addCommand("think", &sWebCmd);
  addCommand("link", &sWebLink);
  m_timer.start();
  s_thinkDevice = this;
  m_started = false;
  m_connected = false;
}

String ThinkDevice::directUrl()
{
    IPAddress myIp;

    myIp = spark::WiFi.localIP();
    return String("http://") + myIp[0] + '.' + myIp[1] + '.' + myIp[2] + '.' + myIp[3] + ':' + PORT;
}

void ThinkDevice::process()
{
  char buff[1024];
  int len = 1024;

  if (!m_started) 
  {
    if (spark::WiFi.ready())
    {
      begin();
      m_started = true;
    }
  }
  else
  {
    processConnection(buff, &len);

    if (g_keepAliveFlag) 
    {
      g_keepAliveFlag = FALSE;
    
      if (m_hubIp == "") 
      {
        request("api.thinkautomatic.io", 80, "/discover?directUrl=" + directUrl(), "POST");
      }
      else 
      {
        // Sends keepAlive
        patch("", "");
      }
    }
  }
}


void ThinkDevice::request(String hostName, int port, String path, String method)
{
  if (hostName != "") 
  {
    if (!m_client.connect(hostName.c_str(), port)) 
    {
      m_client.stop();
      m_connected = false;
      return;
    }

    // Send HTTP Headers
    m_client.println(method + " " + path + " HTTP/1.0");
    m_client.println("Connection: close");
    m_client.println("HOST: " + hostName);
    m_client.println("Content-Type: application/json");
    m_client.println("Accept: application/json");

    m_client.println();
    m_client.flush();

    unsigned long lastRead = millis();

    while (m_client.connected() && (millis() - lastRead < 5000)) 
    {
      while (m_client.available()) 
      {
        if (-1 == m_client.read())
        {
          m_client.stop();
          return;
        }

        lastRead = millis();
      }
      
      delay(200);
    }

    m_client.stop();
  }
}


void ThinkDevice::patch_helper(String path, String name, String value)
{
  String fullPath;
  
  if (m_hubIp != "") 
  {
    fullPath = "/" + path + "?deviceTypeUuid=" + m_deviceTypeUuid + "&uid=" + Particle.deviceID() + "&directUrl=" + directUrl(); 
    if (name != "")
        fullPath += "&" + httpEncode(name) + "=" + httpEncode(value);
    if (m_deviceName != "")
        fullPath += "&name=" + httpEncode(m_deviceName);
    if (m_deviceId != "")
        fullPath += "&deviceId=" + httpEncode(m_deviceId);
    
    // patch does not work on ThinkHub so treat as GET instead  
    request(m_hubIp, m_hubPort, fullPath, "GET");
  }
}


void ThinkDevice::patch(String name, String value)
{
  patch_helper("", name, value);
}

bool ThinkDevice::connected()
{
  return m_connected;
}
