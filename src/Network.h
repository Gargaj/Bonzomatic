#ifndef BONZOMATIC_NETWORK_H
#define BONZOMATIC_NETWORK_H
#include <string>
#include "mongoose.h"
#include <thread>
namespace Network {
    struct mg_mgr mgr;
    struct mg_connection* c;
    bool done = false;
    std::thread* tNetwork;
    static const char* s_url = "ws://drone.alkama.com:9000/roomname/handle";

    bool NewShader = false;
    struct {
        std::string Code;
        int CaretPosition;
        int AnchorPosition;
        int FirstVisibleLine;
        bool NeedRecompile;
    } ShaderMessage;


    enum NetworkMode {
        SENDER,
        GRABBER,
        OFFLINE
    };
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
            fprintf(stdout,"[Network]: Try to connect to %s\n", s_url);

            mg_mgr_init(&mgr);
            c = mg_ws_connect(&mgr, s_url, fn, &done, NULL);
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
      if (Network::HasNewShader()) {
     
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
      
      
      }
    }
}
#endif