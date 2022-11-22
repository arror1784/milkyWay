#include <Util.h>
#include <sstream>

#include <WiFi.h>
#include <WebServer.h>

String Util::ipToString(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++) {
    s += i ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

IPAddress Util::stringToIp(const String &ip) {
  std::vector<String> splitString = Util::stringSplit(ip, '.');

  uint8_t ip1 = splitString[0].toInt();
  uint8_t ip2 = splitString[1].toInt();
  uint8_t ip3 = splitString[2].toInt();
  uint8_t ip4 = splitString[3].toInt();

  return {ip1, ip2, ip3, ip4};
}

std::vector <String> Util::stringSplit(const String &str, char Delimiter) {
  std::istringstream iss(str.c_str());
  std::string buffer;

  std::vector<String> result;

  while (std::getline(iss, buffer, Delimiter)) {
    result.emplace_back(buffer.c_str());
  }

  return result;
}

void Util::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}