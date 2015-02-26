
struct arduino_upload_config {
        char avrTool[16];
        char micro_controller_model[16];
        char configFilePath[256];
        int brate;
        char portName[128];
        char hexFile[256];
};
