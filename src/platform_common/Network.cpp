#include "Network.h"

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
    std::thread network(Create);
    tNetwork = &network;
    tNetwork->detach();
  }
  bool ReloadShader() {
    if (config.Mode == GRABBER && shaderMessage.NeedRecompile) {
      shaderMessage.NeedRecompile = false;
      return true;
    }
    return false;
  }

  void UpdateShader(ShaderEditor* mShaderEditor, float shaderTime) {
    if (Network::config.Mode != Network::NetworkMode::OFFLINE) {
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

        jsonxx::Object Message = jsonxx::Object("Data", Data);
        std::string TextJson = Message.json();
        if (connected) {

          mg_ws_send(c, TextJson.c_str(), TextJson.length(), WEBSOCKET_OP_TEXT);
          shaderMessage.NeedRecompile = false;
        }

      }
    }
  }

  void ParseSettings(jsonxx::Object* options) {

    if (options->has<jsonxx::Object>("network")) {
      jsonxx::Object network = options->get<jsonxx::Object>("network");
      if (network.has<jsonxx::String>("serverURL")) {
        config.Url = strdup(network.get<jsonxx::String>("serverURL").c_str());
      }
      if (network.get<jsonxx::Boolean>("enabled")) {

        if (network.has<jsonxx::String>("networkMode")) {
          const char* mode = network.get<jsonxx::String>("networkMode").c_str();
          if (strcmp(mode, "sender") == 0) {
            config.Mode = SENDER;
          }
          else if (strcmp(mode, "grabber") == 0) {
            config.Mode = GRABBER;
          }
          else {
            config.Mode = GRABBER;
            printf("Can't find 'networkMode', set to 'GRABBER'\n");
          }
        }
        else {
          printf("Can't find 'networkMode', set to 'OFFLINE'\n");
        }
      }


    }
    else {
      config.Mode = OFFLINE;
    }

  }
}