#include <espOnOffGlobals.h>
#include <HttpServerHandles.h>

void httpServerHandlePwrReq(){
  if (httpServer.hasArg("RelayStReq")==true){
    if (httpServer.arg("RelayStReq")=="0xAA"){
      espOnOff.setRelaySt(besON,besRelReqHTTP);
    }else{
      espOnOff.setRelaySt(besOFF,besRelReqHTTP);
    }
    httpServer.sendHeader("Location","/"); // Add a header to respond with a new location for the browser to go to the home page again
    httpServer.send(303);
  }else{
    httpServer.sendHeader("Location","/");
    httpServer.send(404);
    //TODO: register incorrect request time, source ip
  }
  stdOut("req from " + httpServer.client().remoteIP().toString() + ", Pwr Req");
}

void httpServerHandleGetData(){
  if (httpServer.hasArg("getRelaySt")==true){
    if (espOnOff.getRelaySt()==besON)
      httpServer.send(200, "text/html", "RelayON");
    else
      httpServer.send(200, "text/html", "RelayOFF");
  }else if(httpServer.hasArg("fileList")==true){
    String jsonFileList = "{\"espData\":[{\"Field\":\"File Name\",\"Data\":\"Size\"},";
    Dir directoryEntry = SPIFFS.openDir("/");
    while (directoryEntry.next()) {
      File fileElement = directoryEntry.openFile("r");
      jsonFileList += "{\"Field\":\"" + String(fileElement.name()).substring(1) + "\",\"Data\":\"" + String(fileElement.size()) + "\"},";
      fileElement.close();
    }
    jsonFileList.remove(jsonFileList.length()-1,1); //remove last ","
    jsonFileList += "]}";
    httpServer.send(200, "text/html", jsonFileList);
  }else if(httpServer.hasArg("deviceData")==true){
    String jsonDeviceData="{\"espData\":[{\"Field\":\"Host Name\",\"Data\":\"" + espHost + "\"},"
    "{\"Field\":\"Device IP\",\"Data\":\"" + WiFi.localIP().toString() + "\"},"
    "{\"Field\":\"Device MAC Address\",\"Data\":\"" + WiFi.macAddress() + "\"},"
    "{\"Field\":\"WiFi RSSI\",\"Data\":\"" + String(WiFi.RSSI()) + "\"},"
    "{\"Field\":\"Free Heap\",\"Data\":\"" + String(ESP.getFreeHeap()) + "\"},"
    "{\"Field\":\"Core Version\",\"Data\":\"" + ESP.getCoreVersion() + "\"},"
    "{\"Field\":\"CPU Flash Frequency\",\"Data\":\"" + String(ESP.getCpuFreqMHz()) + "\"},"
    "{\"Field\":\"Flash Chip ID\",\"Data\":\"" + String(ESP.getFlashChipId()) + "\"},"
    "{\"Field\":\"Flash Chip Size\",\"Data\":\"" + String(ESP.getFlashChipSize()) + "\"},"
    "{\"Field\":\"Flash Chip Real Size\",\"Data\":\"" + String(ESP.getFlashChipRealSize()) + "\"},"
    "{\"Field\":\"Free Sketch Size\",\"Data\":\"" + String(ESP.getFreeSketchSpace()) + "\"},"
    "{\"Field\":\"Sketch Size\",\"Data\":\"" + String(ESP.getSketchSize()) + "\"},"
    "{\"Field\":\"Sketch MD5\",\"Data\":\"" + String(ESP.getSketchMD5()) + "\"},"
    "{\"Field\":\"Last Reset Reason\",\"Data\":\"" + String(ESP.getResetReason()) + "\"}]}";
    httpServer.send(200, "text/html", jsonDeviceData);
  }else if(httpServer.hasArg("deviceSSID")==true){
    String jsonDeviceData="{\"espData\":[{\"Field\":\"confSSID\",\"Data\":\"\"},"
    "{\"Field\":\"confPW\",\"Data\":\"\"},"
    "{\"Field\":\"confMACAddr\",\"Data\":\"" + WiFi.macAddress() + "\"}]}";
    httpServer.send(200, "text/html", jsonDeviceData);
  }
}

void httpServerHandleFileUpload(){
  logTelnetBuff.write("fileupload page requested\r\n");
  String mainPage;
  mainPage += "<!DOCTYPE html><html lang=\"en\">";
  mainPage += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /></head>";
  mainPage += "<title>espOnOff Livingroom 1 File Upload</title>";
  mainPage += "<form action=\"fileuploadstream\" method=\"post\" enctype=\"mulipart/form-data\">";
  /*mainPage += "<input type=\"file\" name=\"fileuploadstream\">";
  mainPage += "<input class=\"button\" type=\"submit\" value=\"upload\">";*/
  mainPage += "<input class='buttons' style='width:40%' type='file' name='fileuploadstream' id = 'fileuploadstream' value=''><br>";
  mainPage += "<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>";
  mainPage += "</form></body></html>";
  httpServer.sendHeader("Connection", "close");
  httpServer.send(200, "text/html", mainPage);
}

void httpServerHandleFileUploadStream(){
  // curl -X POST -F "file=@SomeFile.EXT" espIP/fileuploadstream
  logTelnetBuff.write("fileupload stream requested\r\n");
  HTTPUpload& upload = httpServer.upload();
  //File fsUploadFile;

  stdOut("handleFileUpload Entry: " + String(upload.status));
  logTelnetBuff.write("handleFileUpload Entry: " + String(upload.status) + "\r\n");
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    //Serial.println("FileUpload Name: " + filename);
    //logTelnetBuff.write("FileUpload Name: " + filename + "\r\n");
    fsUploadFile = SPIFFS.open(filename, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    //logTelnetBuff.write("fsUploadFile: " + String(fsUploadFile) + "\r\n");
    if(fsUploadFile){
      fsUploadFile.close();
      //Serial.print("FileUpload Size: " + String(upload.totalSize));
      //logTelnetBuff.write("FileUpload Size: " + String(upload.totalSize) + "\r\n");
      httpServer.sendHeader("Location","/");      // Redirect the client to the success page
      httpServer.send(303);
    }else{
      httpServer.send(500, "text/plain", "500: couldn't create file");
      logTelnetBuff.write("couldn't create file\r\n");
    }
  }
}

void httpServerHandleSaveSSID(){
  if (httpServer.hasArg("saveSSID")==true &&
  httpServer.hasArg("confSSID")==true &&
  httpServer.hasArg("confPW")==true &&
  httpServer.arg("saveSSID")=="Save" &&
  httpServer.arg("confSSID")!="" &&
  httpServer.arg("confPW")!=""){
    SPIFFS.remove("/config.dat");
    File configFile = SPIFFS.open("/config.dat", "w");
    String tmpMACAddr = httpServer.arg("confMACAddr");
    if (tmpMACAddr.length()==17)
      configFile.print(tmpMACAddr);
    else
      WiFi.macAddress();
    configFile.write(3);
    configFile.print(httpServer.arg("confSSID"));
    configFile.write(3);
    configFile.print(httpServer.arg("confPW"));
    configFile.write(3);
    configFile.close();
    httpServer.sendHeader("Location","/");
    httpServer.send(303);
    ESP.restart();
  }
  httpServer.sendHeader("Location","/");
  httpServer.send(303);
}

void httpServerHandleDeviceReset(){
  httpServer.sendHeader("Location","/"); // Add a header to respond with a new location for the browser to go to the home page again
  httpServer.send(303);
  ESP.restart();
}

void httpServerHandleNotFound(){
  if (loadFromSpiffs(httpServer.uri()))
    return;
  logTelnetBuff.write("uri error :" + httpServer.uri() + "\r\n");
  httpServer.sendHeader("Connection", "close");
  httpServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
  stdOut("req from " + httpServer.client().remoteIP().toString() + ", 404 page");
}

bool loadFromSpiffs(String path) {
    String dataType = "text/plain";
    bool fileTransferStatus = false;

    if (path.endsWith("/")){
        path += "index.html";
        dataType = "text/html";
    }
    else if (path.endsWith(".html")) dataType = "text/html";
    else if (path.endsWith(".htm")) dataType = "text/html";
    else if (path.endsWith(".css")) dataType = "text/css";
    else if (path.endsWith(".js")) dataType = "application/javascript";
    else if (path.endsWith(".png")) dataType = "image/png";
    else if (path.endsWith(".gif")) dataType = "image/gif";
    else if (path.endsWith(".jpg")) dataType = "image/jpeg";
    else if (path.endsWith(".ico")) dataType = "image/x-icon";
    else if (path.endsWith(".xml")) dataType = "text/xml";
    else if (path.endsWith(".pdf")) dataType = "application/pdf";
    else if (path.endsWith(".zip")) dataType = "application/zip";

    File dataFile = SPIFFS.open(path.c_str(), "r");

    if (!dataFile) {
        stdOut("Failed to open file:" + path);
        return fileTransferStatus;
    }
    else {
        if (httpServer.streamFile(dataFile, dataType) == dataFile.size()) {
            stdOut(String("Sent file: ") + path);
            fileTransferStatus = true;
        }
    }

    dataFile.close();
    return fileTransferStatus;
}
