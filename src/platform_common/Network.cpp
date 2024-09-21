#include "Network.h"
#include "MIDI.h"
#define SHADER_FILENAME(mode) (std::string(mode)+ "_" + RoomName + "_" + NickName + ".glsl")
#define LOG(header,message) printf("[" header "] " message " \n")
namespace Network {

    Network::NetworkConfig config;
    Network::ShaderMessage shaderMessage;

    struct mg_mgr mgr;
    struct mg_connection* c;
    bool done = false;
    std::thread* tNetwork;
    bool IsNewShader = false;
    char szShader[65535];
    bool connected = false;


  char* GetUrl() {
      return config.Url;
  }
  void SetUrl( char* url) {
    config.Url =url;
  }
  Network::NetworkMode GetNetworkMode() {
    return config.Mode;
  }
  void SetNetworkMode(NetworkMode mode) {
    config.Mode = mode;
  }
  void PrintConfig() {
    std::cout << "******************* Network Config ********************" << std::endl;
    std::cout << config.Url << std::endl;
    if (config.Mode == NetworkMode::OFFLINE) {
      std::cout << "OFFLINE" << std::endl;
    }
    else if (config.Mode == NetworkMode::SENDER) {
      std::cout << "SENDER" << std::endl;
    }
    else if (config.Mode == NetworkMode::GRABBER) {
      std::cout << "GRABBER" << std::endl;
    }

  }
  bool HasNewShader() {
    if (IsNewShader) {
      IsNewShader = false;
      return true;
    }
    return false;

  }
  void RecieveShader(size_t size, char* data) {
    // TODO: very very bad, we should:
    // - use json
    // - verify size
    // - non-ascii symbols ?
    // - asynchronous update ?
    //data[size - 1] = '\0';
    std::string TextJson(data);
    jsonxx::Object NewShader;
    jsonxx::Object Data;
    bool ErrorFound = false;

    if (NewShader.parse(TextJson)) {
      if (NewShader.has<jsonxx::Object>("Data")) {
        Data = NewShader.get<jsonxx::Object>("Data");
        if (!Data.has<jsonxx::String>("Code")) ErrorFound = true;
        if (!Data.has<jsonxx::Number>("Caret")) ErrorFound = true;
        if (!Data.has<jsonxx::Number>("Anchor")) ErrorFound = true;
        if (!Data.has<jsonxx::Number>("FirstVisibleLine")) ErrorFound = true;
        if (!Data.has<jsonxx::Boolean>("Compile")) ErrorFound = true;
      }
      else {
        ErrorFound = true;
      }
    }
    else {
      ErrorFound = true;
    }
    if (ErrorFound) {
      fprintf(stderr, "Invalid json formatting\n");
      return;
    }
    if (Data.has<jsonxx::Number>("ShaderTime")) {

      float t = Data.get<jsonxx::Number>("ShaderTime");
      shaderMessage.Code = Data.get < jsonxx::String>("Code");
      shaderMessage.AnchorPosition = Data.get<jsonxx::Number>("Anchor");
      shaderMessage.CaretPosition = Data.get<jsonxx::Number>("Caret");
      shaderMessage.NeedRecompile = Data.get<jsonxx::Boolean>("Compile");
      IsNewShader = true;

    }

  }
  static void fn(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_OPEN) {
      c->is_hexdumping = 0;
    }
    else if (ev == MG_EV_ERROR) {
      // On error, log error message
      MG_ERROR(("%p %s", c->fd, (char*)ev_data));
    }
    else if (ev == MG_EV_WS_OPEN) {
      fprintf(stdout, "[Network]: Connected\n");
      connected = true;
      // When websocket handshake is successful, send message
      // mg_ws_send(c, "hello", 5, WEBSOCKET_OP_TEXT);
    }
    else if (ev == MG_EV_WS_MSG && config.Mode == GRABBER) {
      // When we get echo response, print it

      struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;

      RecieveShader((int)wm->data.len, wm->data.buf);

      // printf("GOT ECHO REPLY: [%.*s]\n", (int)wm->data.len, wm->data.buf);
    }

    /*/if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_MSG) {
      *(bool*)c->fn_data = true;  // Signal that we're done
    }*/
  }

  void Create() {
    fprintf(stdout, "[Network]: Try to connect to %s\n", config.Url);
    mg_mgr_init(&mgr);
    c = mg_ws_connect(&mgr, config.Url, fn, &done, NULL);
    if (c == NULL) {
      fprintf(stderr, "Invalid address\n");
      return;
    }
    while (true) {
      mg_mgr_poll(&mgr, 100);
    }
    mg_mgr_free(&mgr);
  }

  void Init() {
    if(config.Mode != OFFLINE){
      std::thread network(Create);
      tNetwork = &network;
      tNetwork->detach();
    }
    else {
      fprintf(stdout, "[Network]: OFFLINE Mode, not starting Network loop\n");
    }
  }
  bool ReloadShader() {
    if (config.Mode == GRABBER && shaderMessage.NeedRecompile) {
      shaderMessage.NeedRecompile = false;
      return true;
    }
    return false;
  }
  void SetNeedRecompile(bool needToRecompile) {
    shaderMessage.NeedRecompile = needToRecompile;
  }
  void SplitUrl(std::string* host, std::string* roomname, std::string* name) {
    std::string FullUrl((const char*)Network::GetUrl());
    std::size_t PathPartPtr = FullUrl.find('/', 6);
    std::size_t HandlePtr = FullUrl.find('/', PathPartPtr + 1);
    *host = FullUrl.substr(0, PathPartPtr);
    *roomname = FullUrl.substr(PathPartPtr + 1, HandlePtr - PathPartPtr - 1);
    *name = FullUrl.substr(HandlePtr + 1, FullUrl.size() - HandlePtr);
  }
  void UpdateShaderFileName(const char** shaderName) {
    if (config.Mode == OFFLINE) return;
    std::string HostPort, RoomName, NickName, filename;
    Network::SplitUrl(&HostPort, &RoomName, &NickName);
    if (config.Mode == SENDER) {
      filename = SHADER_FILENAME("sender");
    }
    else if(config.Mode == GRABBER) {
      filename = SHADER_FILENAME("grabber");
    }
    *shaderName = _strdup(filename.c_str());
  }
  void UpdateShader(ShaderEditor* mShaderEditor, float shaderTime, std::map<int, std::string> *midiRoutes) {
    if (Network::config.Mode != Network::NetworkMode::OFFLINE) { // If we arn't offline mode
      if (config.Mode == Network::GRABBER && Network::HasNewShader()) { // Grabber mode

        int PreviousTopLine = mShaderEditor->WndProc(SCI_GETFIRSTVISIBLELINE, 0, 0);
        int PreviousTopDocLine = mShaderEditor->WndProc(SCI_DOCLINEFROMVISIBLE, PreviousTopLine, 0);
        int PreviousTopLineTotal = PreviousTopDocLine;

        mShaderEditor->SetText(shaderMessage.Code.c_str());
        mShaderEditor->WndProc(SCI_SETCURRENTPOS, shaderMessage.CaretPosition, 0);
        mShaderEditor->WndProc(SCI_SETANCHOR, shaderMessage.AnchorPosition, 0);
        mShaderEditor->WndProc(SCI_SETFIRSTVISIBLELINE, PreviousTopLineTotal, 0);

        //if (bGrabberFollowCaret) {
        //mShaderEditor.WndProc(SCI_SETFIRSTVISIBLELINE, NewMessage.FirstVisibleLine, 0);
        mShaderEditor->WndProc(SCI_SCROLLCARET, 0, 0);
        //}


      }
      else if (config.Mode == Network::SENDER && shaderTime - shaderMessage.shaderTime > 0.1) {
        //std::cout << shaderTime<<"-"<<ShaderMessage.shaderTime << "="<< shaderTime - ShaderMessage.shaderTime << std::endl;

        mShaderEditor->GetText(szShader, 65535);
        shaderMessage.Code = std::string(szShader);

        shaderMessage.CaretPosition = mShaderEditor->WndProc(SCI_GETCURRENTPOS, 0, 0);
        shaderMessage.AnchorPosition = mShaderEditor->WndProc(SCI_GETANCHOR, 0, 0);
        int TopLine = mShaderEditor->WndProc(SCI_GETFIRSTVISIBLELINE, 0, 0);
        shaderMessage.FirstVisibleLine = mShaderEditor->WndProc(SCI_DOCLINEFROMVISIBLE, TopLine, 0);
        shaderMessage.shaderTime = shaderTime;
        jsonxx::Object Data;
        Data << "Code" << shaderMessage.Code;
        Data << "Compile" << shaderMessage.NeedRecompile;
        Data << "Caret" << shaderMessage.CaretPosition;
        Data << "Anchor" << shaderMessage.AnchorPosition;
        Data << "FirstVisibleLine" << shaderMessage.FirstVisibleLine;
        Data << "RoomName" << "RoomName";
        Data << "NickName" << "NickName";
        Data << "ShaderTime" << shaderMessage.shaderTime;

        if(config.sendMidiControls) { // Sending Midi Controls
          jsonxx::Object networkShaderParameters;
          for (std::map<int, std::string>::iterator it = midiRoutes->begin(); it != midiRoutes->end(); it++)
          {
            networkShaderParameters << it->second << MIDI::GetCCValue(it->first);
          }
          Data << "Parameters" << networkShaderParameters;
        }
        jsonxx::Object Message = jsonxx::Object("Data", Data);
        std::string TextJson = Message.json();

        if (connected) {
          mg_ws_send(c, TextJson.c_str(), TextJson.length(), WEBSOCKET_OP_TEXT);
          shaderMessage.NeedRecompile = false;
        }

      }
    }
  }
  bool IsGrabber() {
    return config.Mode == GRABBER;
  }
  bool IsSender() {
    return config.Mode = SENDER;
  }
  bool IsOffline() {
    return config.Mode = OFFLINE;
  }
  void GenerateWindowsTitle(char** originalTitle) {
    if (config.Mode == OFFLINE) {
      return;
    }
    std::string host, roomname, user, title(*originalTitle), newName;
    Network::SplitUrl(&host, &roomname, &user);
    if (config.Mode == GRABBER) {
      newName = title + " grabber " + user;
    } 
    if (config.Mode == SENDER) {
      newName = title + " sender " + user;
    }
    *originalTitle = _strdup(newName.c_str());
  }
  /* From here are methods for parsing json */
  void ParseNetworkGrabMidiControls(jsonxx::Object * network) {
    if (!network->has<jsonxx::Boolean>("grabMidiControls")) {
      LOG("Network Configuration", "Can't find 'grabMidiControls', set to false");
      config.grabMidiControls = false;
      return;
    }
    config.grabMidiControls = network->get<jsonxx::Boolean>("grabMidiControls");
  }
  void ParseNetworkSendMidiControls(jsonxx::Object* network) {
    if (!network->has<jsonxx::Boolean>("sendMidiControls")) {
      LOG("Network Configuration", "Can't find 'sendMidiControls', set to false");
      config.sendMidiControls = false;
      return;
    }
    config.sendMidiControls = network->get<jsonxx::Boolean>("sendMidiControls");
  }
  void ParseNetworkUpdateInterval(jsonxx::Object* network) {
    if (!network->has<jsonxx::Boolean>("updateInterval")) {
      LOG("Network Configuration", "Can't find 'updateInterval', set to 0.3");
      config.updateInterval = 0.3f;
      return;
    }
    
    config.updateInterval = network->get<jsonxx::Number>("updateInterval");
  }
  void ParseNetworkMode(jsonxx::Object* network) {
    if (!network->has<jsonxx::String>("networkMode")) {
      LOG("Network Configuration", "Can't find 'networkMode' Set to OFFLINE");
      config.Mode = OFFLINE;
      return;
    }

    const char* mode = network->get<jsonxx::String>("networkMode").c_str();
    bool isSenderMode = strcmp(mode, "sender");
    bool isGrabberMode = strcmp(mode, "grabber");
    if (!isSenderMode && !isGrabberMode) {
      LOG("Network Configuration", "networkMode is neither SENDER or GRABBER, fallback config to OFFLINE");
      config.Mode = OFFLINE;
      return;
    }
    if(isSenderMode){
      config.Mode = SENDER;
    }
    if(isGrabberMode){
      config.Mode = GRABBER;
    }

    // From now on, we have a minimal config working we can try to parse extra option
    ParseNetworkGrabMidiControls(network);
    ParseNetworkSendMidiControls(network);
    ParseNetworkUpdateInterval(network);
  }
  void ParseNetworkUrl(jsonxx::Object* network) {
    if (!network->has<jsonxx::String>("serverURL")) {
      LOG("Network Configuration", "Can't find 'serverURL', set to 'OFFLINE'");
      config.Mode = OFFLINE;
      config.Url = "";
      return;
    }
   
    config.Url = _strdup(network->get<jsonxx::String>("serverURL").c_str());

    ParseNetworkMode(network);

  }
  void ParseNetworkEnabled(jsonxx::Object* network) {
 
    if (!network->has<jsonxx::Boolean>("enabled")) {
      LOG("Network Configuration", "Can't find 'enabled', set to 'OFFLINE'");
      config.Mode = OFFLINE;
      config.Url = "";
      return;
    }

    if (!network->get<jsonxx::Boolean>("enabled")) {
      LOG("Network Configuration", "Set to 'OFFLINE'");
      config.Mode = OFFLINE;
      config.Url = "";
      // As we can activate this on setup dialog, let's try to get serverURL
      if (network->has<jsonxx::String>("serverURL")) {
        config.Url = _strdup(network->get<jsonxx::String>("serverURL").c_str());
      }
      return;
    }
    ParseNetworkUrl(network);


  }
  /*
    Parse the json settings. Cascading calls, not perfect but keep clear code
    - Check that Network block exists on json
    - Check that 'enabled' exists and is true
    - Check that 'serverUrl' exists
    - Check that 'networkMode' exists

    If something doesn't match above path, will fallback to OFFLINE Mode
    */
  void ParseSettings(jsonxx::Object* options) {
    
    LOG("Network Configuration", "Parsing network configuration data from json");
    if (!options->has<jsonxx::Object>("network")) {
      LOG("Network Configuration", "Can't find 'network' block, set to 'OFFLINE'");
      config.Mode = OFFLINE;
      config.Url = "";
      return;
    }
    jsonxx::Object network = options->get<jsonxx::Object>("network");
    ParseNetworkEnabled(&network);


  }
}