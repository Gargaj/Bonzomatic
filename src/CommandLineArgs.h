#ifndef CMD_ARGS_H_
#define CMD_ARGS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define assert_tuple_arg (assert(i < argc && "Expecting value"))

namespace CommandLineArgs
{

    struct {
        bool skipDialog;
        const char* configFile;
        const char* shaderFile;
       
    } Args;

    void parse_args(int argc,const char *argv[]) {
        Args.skipDialog = false;
        Args.configFile = "config.json";
        Args.shaderFile = "shader.glsl";
        //Network::config.Mode = Network::NetworkMode::OFFLINE;
        //Network::config.Url = "ws://drone.alkama.com:9000/roomname/username";

        for(size_t i=0;i<argc;++i) {

            if(strcmp(argv[i],"skipdialog")==0){
                Args.skipDialog = true;
                continue;
            }

            if(strcmp(argv[i],"configfile")==0) {
                i++;
                assert_tuple_arg;
                Args.configFile = argv[i];
                continue;
            }

            if(strcmp(argv[i],"shader")==0) {
                i++;
                assert_tuple_arg;
                Args.shaderFile = argv[i];
                continue;
            }

            if(strcmp(argv[i],"serverURL")==0) {
                i++;
                assert_tuple_arg;
                //Network::NetworkConfig.Url = argv[i];
                continue;
            }

            if(strcmp(argv[i],"networkMode")==0) {
                i++;
                assert_tuple_arg;
                if(strcmp(argv[i],"grabber")){
                    //Network::config.Mode = Network::NetworkMode::GRABBER;
                    continue;
                }
                if(strcmp(argv[i],"sender")){
                    //Network::config.Mode = Network::NetworkMode::SENDER;
                    continue;
                }
                if(strcmp(argv[i],"offline")){
                   // Network::config.Mode = Network::NetworkMode::OFFLINE;
                    continue;
                }
            }        

        }
    }

    void print_args_to_config() {

        fprintf(stdout,"skipDialog: %i\n",CommandLineArgs::Args.skipDialog);
        fprintf(stdout,"configFile: %s\n",CommandLineArgs::Args.configFile);
        fprintf(stdout,"shaderFile: %s\n",CommandLineArgs::Args.shaderFile);

    }
}

#endif