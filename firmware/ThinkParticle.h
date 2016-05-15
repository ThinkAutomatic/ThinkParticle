#ifndef __THINK_PARTICLE_H_
#define __THINK_PARTICLE_H_

#include "WebServer.h"

class ThinkDevice: public WebServer
{
public:
  typedef void ThinkCallback(String name, String value);

protected:
  TCPClient             m_client;
  static ThinkDevice    *s_thinkDevice;
  ThinkCallback         *m_thinkCallback;
  String                m_deviceId;
  String                m_deviceName;
  String                m_deviceTypeUuid;
  String                m_directUrl;
  String                m_hubIp;
  int                   m_hubPort;

  static String         httpEncode(String s);
  
  void                  handleParams(String name, String value);
  
  static void           sWebCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
  void                  webCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);

  static void           sWebLink(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
  void                  webLink(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
  
  void                  request(String hostName, int port, String path, String method);
  void                  get(String path, String name, String value);
  String                directUrl();
  String                deviceConf();

public:
  ThinkDevice(String deviceName, String deviceTypeUuid, ThinkCallback *thinkCallback);
  void                  patch(String name, String value);
  void                  process();
};


#endif
