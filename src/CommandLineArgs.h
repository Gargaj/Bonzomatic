#ifndef CMD_ARGS_H_
#define CMD_ARGS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define assert_tuple_arg (assert(i < argc && "Expecting value"))

namespace CommandLineArgs
{
    enum NetworkMode {
        SENDER,
        GRABBER,
        OFFLINE
    };
    struct {
        bool skipDialog;
        const char* configFile;
        const char* shaderFile;
        const char* serverURL;
        NetworkMode networkMode;
    } Args;

    void parse_args(int argc,const char *argv[]) {
        Args.skipDialog = false;
        Args.configFile = "config.json";
        Args.shaderFile = "shader.glsl";
        Args.networkMode = OFFLINE;
        Args.serverURL = "ws://drone.alkama.com:9000/roomname/username";
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
                Args.serverURL = argv[i];
                continue;
            }

            if(strcmp(argv[i],"networkMode")==0) {
                i++;
                assert_tuple_arg;
                if(strcmp(argv[i],"grabber")){
                    Args.networkMode = GRABBER;
                    continue;
                }
                if(strcmp(argv[i],"sender")){
                    Args.networkMode = SENDER;
                    continue;
                }
                if(strcmp(argv[i],"offline")){
                    Args.networkMode = OFFLINE;
                    continue;
                }
            }        

        }
    }

    void print_args_to_config() {

        fprintf(stdout,"skipDialog: %i\n",CommandLineArgs::Args.skipDialog);
        fprintf(stdout,"configFile: %s\n",CommandLineArgs::Args.configFile);
        fprintf(stdout,"shaderFile: %s\n",CommandLineArgs::Args.shaderFile);
        fprintf(stdout,"serverURL: %s\n",CommandLineArgs::Args.serverURL);
        fprintf(stdout,"networkMode: %i\n",CommandLineArgs::Args.networkMode);

    }
}

#endif