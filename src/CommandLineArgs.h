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
        for(size_t i=0;i<argc;++i) {

            if(strcmp(argv[i],"skipdialog")==0){
                Args.skipDialog = true;
            }

            if(strcmp(argv[i],"configfile")==0) {
                i++;
                assert_tuple_arg;
                Args.configFile = argv[i];
            }

            if(strcmp(argv[i],"shader")==0) {
                i++;
                assert_tuple_arg;
                Args.shaderFile = argv[i];
            }
        }
    }

    void apply_args_to_config(int argc,const char *argv[],const char **configFile ) {
        parse_args(argc,argv);
        fprintf(stdout,"skipDialog: %i\n",CommandLineArgs::Args.skipDialog);
        fprintf(stdout,"configFile: %s\n",CommandLineArgs::Args.configFile);
        fprintf(stdout,"shaderFile: %s\n",CommandLineArgs::Args.shaderFile);
        *configFile = Args.configFile;
    }
}

#endif