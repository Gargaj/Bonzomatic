#include "Config.h"

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif


namespace Config
{

	void ApplicationSettings::find_location()
	{
		char cwd[ 256 ];
		getcwd( cwd, 255 );
		std::string curDir( cwd );

		std::ifstream cfgFile( config_filename_ );
		if( cfgFile.is_open() )
		{
			config_directory_ = curDir;
			data_directory_ = curDir;
			cfgFile.close();
			location_ = Location::DIRECTORY;
			std::cout << "Config file found in current directory (" << config_directory_ << ")...\n";
			return;
		}

		config_directory_ = curDir;
		location_ = Location::NONE;
		std::cout << "No config file found, using default settings...\n";
	}
}