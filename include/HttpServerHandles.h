
// function prototypes for HTTP handlers
bool loadFromSpiffs(String path);

void httpServerHandlePwrReq();
void httpServerHandleGetData();
void httpServerHandleFileUpload();
void httpServerHandleFileUploadStream();
void httpServerHandleSaveSSID();
void httpServerHandleDeviceReset();
void httpServerHandleNotFound();
