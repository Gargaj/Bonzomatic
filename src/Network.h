#ifndef BONZOMATIC_NETWORK_H
#define BONZOMATIC_NETWORK_H
#include <string>
#include "mongoose.h"
#include <thread>
namespace Network {
  enum NetworkMode {
      OFFLINE, 
      SENDER,
      GRABBER
    };

    struct mg_mgr mgr;
    struct mg_connection* c;
    bool done = false;
    std::thread* tNetwork;
    bool NewShader = false;
    char szShader[65535];
    bool connected = false;
    struct {
       char* Url;
      NetworkMode Mode;
      float updateInterval = 0.3;
    } NetworkConfig;

    struct {
        std::string Code;
        int CaretPosition;
        int AnchorPosition;
        int FirstVisibleLine;
        bool NeedRecompile;
    } ShaderMessage;

    void PrintConfig() {
      std::cout << "******************* Network Config ********************" << std::endl;
      std::cout << Network::NetworkConfig.Url << std::endl;
      if (NetworkConfig.Mode == NetworkMode::OFFLINE) {
        std::cout << "OFFLINE" << std::endl;
      } else if (NetworkConfig.Mode == NetworkMode::SENDER) {
        std::cout << "SENDER" << std::endl;
      } else if (NetworkConfig.Mode == NetworkMode::GRABBER) {
        std::cout << "GRABBER" << std::endl;
      }

    }
    bool HasNewShader() {
      if (NewShader) {
        NewShader = false;
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
        ShaderMessage.Code = Data.get < jsonxx::String>("Code");
        ShaderMessage.AnchorPosition = Data.get<jsonxx::Number>("Anchor");
        ShaderMessage.CaretPosition = Data.get<jsonxx::Number>("Caret");
        ShaderMessage.NeedRecompile = Data.get<jsonxx::Boolean>("Compile");
        Network::NewShader = true;
 
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
      else if (ev == MG_EV_WS_MSG) {
        // When we get echo response, print it
   
        struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
        RecieveShader((int)wm->data.len, wm->data.buf);
       // printf("GOT ECHO REPLY: [%.*s]\n", (int)wm->data.len, wm->data.buf);
      }

      /*/if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_MSG) {
        *(bool*)c->fn_data = true;  // Signal that we're done
      }*/
    }
    
    void Create(){
            fprintf(stdout,"[Network]: Try to connect to %s\n", NetworkConfig.Url);

            mg_mgr_init(&mgr);
            c = mg_ws_connect(&mgr, NetworkConfig.Url, fn, &done, NULL);
            if (c == NULL) {
              fprintf(stderr, "Invalid address\n");
              return;
            }
            while (true) {
              mg_mgr_poll(&mgr, 1000);
            }
            mg_mgr_free(&mgr);
    }

    void Init() {
      std::thread network(Create);
      tNetwork = &network;
      tNetwork->detach();
      
    }
    bool ReloadShader() {
      if (ShaderMessage.NeedRecompile) {
        ShaderMessage.NeedRecompile = false;
        return true;
      }
      return false;
    }
    void UpdateShader(ShaderEditor* mShaderEditor) {

      if (NetworkConfig.Mode == Network::GRABBER && Network::HasNewShader()) { // Grabber mode
          
          int PreviousTopLine = mShaderEditor->WndProc(SCI_GETFIRSTVISIBLELINE, 0, 0);
          int PreviousTopDocLine = mShaderEditor->WndProc(SCI_DOCLINEFROMVISIBLE, PreviousTopLine, 0);
          int PreviousTopLineTotal = PreviousTopDocLine;

          mShaderEditor->SetText(ShaderMessage.Code.c_str());
          mShaderEditor->WndProc(SCI_SETCURRENTPOS, ShaderMessage.CaretPosition, 0);
          mShaderEditor->WndProc(SCI_SETANCHOR, ShaderMessage.AnchorPosition, 0);
          mShaderEditor->WndProc(SCI_SETFIRSTVISIBLELINE, PreviousTopLineTotal, 0);

          //if (bGrabberFollowCaret) {
          //mShaderEditor.WndProc(SCI_SETFIRSTVISIBLELINE, NewMessage.FirstVisibleLine, 0);
          mShaderEditor->WndProc(SCI_SCROLLCARET, 0, 0);
          //}
      
      
      } else if(NetworkConfig.Mode == Network::SENDER) {
        mShaderEditor->GetText(szShader, 65535);
        ShaderMessage.Code = std::string(szShader);
        ShaderMessage.NeedRecompile = true;
        ShaderMessage.CaretPosition = mShaderEditor->WndProc(SCI_GETCURRENTPOS, 0, 0);
        ShaderMessage.AnchorPosition = mShaderEditor->WndProc(SCI_GETANCHOR, 0, 0);
        int TopLine = mShaderEditor->WndProc(SCI_GETFIRSTVISIBLELINE, 0, 0);
        ShaderMessage.FirstVisibleLine = mShaderEditor->WndProc(SCI_DOCLINEFROMVISIBLE, TopLine, 0);
        jsonxx::Object Data;
        Data << "Code" << std::string(ShaderMessage.Code);
        Data << "Compile" << ShaderMessage.NeedRecompile;
        Data << "Caret" << ShaderMessage.CaretPosition;
        Data << "Anchor" << ShaderMessage.AnchorPosition;
        Data << "FirstVisibleLine" << ShaderMessage.FirstVisibleLine;
        Data << "RoomName" << "RoomName";
        Data << "NickName" << "NickName";
        Data << "ShaderTime" << 1;

        jsonxx::Object Message = jsonxx::Object("Data", Data);
        std::string TextJson = Message.json();
        if(connected){
        mg_ws_send(c, TextJson.c_str(), TextJson.length() , WEBSOCKET_OP_TEXT);
        }
      }
    }

    void ParseSettings(jsonxx::Object *options) {

      if(options->has<jsonxx::Object>("network")) {
        jsonxx::Object network = options->get<jsonxx::Object>("network");
        if(network.has<jsonxx::String>("serverURL")){
            NetworkConfig.Url = strdup(network.get<jsonxx::String>("serverURL").c_str());
        }
        if (network.get<jsonxx::Boolean>("enabled")) {
      
          if (network.has<jsonxx::String>("networkMode")) {
            const char *  mode = network.get<jsonxx::String>("networkMode").c_str();
            if (strcmp(mode, "sender") == 0) {
              NetworkConfig.Mode = SENDER;
            }
            else if (strcmp(mode, "grabber") == 0) {
              NetworkConfig.Mode = GRABBER;
            }
            else {
              NetworkConfig.Mode = GRABBER;
              printf("Can't find 'networkMode', set to 'GRABBER'\n");
            }
          } else {
            printf("Can't find 'networkMode', set to 'OFFLINE'\n");
          }
        }

        
      } else {
        NetworkConfig.Mode = OFFLINE;
      }

    }
}
#endif