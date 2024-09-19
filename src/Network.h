#ifndef BONZOMATIC_NETWORK_H
#define BONZOMATIC_NETWORK_H
#pragma once
#include <string>
#include "mongoose.h"
#include <thread>
#include <jsonxx.h>
#include "ShaderEditor.h"
namespace Network {
  enum NetworkMode {
    OFFLINE,
    SENDER,
    GRABBER
  };

  struct NetworkConfig {
    char* Url;
    NetworkMode Mode;
    float updateInterval = 0.3;
  };
  struct ShaderMessage {
    std::string Code;
    int CaretPosition;
    int AnchorPosition;
    int FirstVisibleLine;
    bool NeedRecompile;
    float shaderTime = 0.0;
  };


 
    
    void PrintConfig();
    bool HasNewShader();
    bool ReloadShader();
    void RecieveShader(size_t size, char* data);
    static void fn(struct mg_connection* c, int ev, void* ev_data);
    
    void Create();
    void Init();

    void ParseSettings(jsonxx::Object* options);
    void UpdateShader(ShaderEditor* mShaderEditor, float shaderTime);
    char* GetUrl();
    void SetUrl(char*);
}
#endif // BONZOMATIC_NETWORK_H