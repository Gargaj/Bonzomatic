#include "Config.h"

#include <fstream>
#include <iostream>
#include <vector>

#include <unistd.h>
#include <stdlib.h>


namespace Config
{

	void ApplicationSettings::find_location()
	{
		const std::string sysPath( "/etc/bonzomatic" );
		const std::string userPath( getenv( "HOME" ) );
		const std::string localConfigPath = "/.config/bonzomatic";
		std::string localPath = userPath + localConfigPath;

		char cwd[ 256 ];
		getcwd( cwd, 255 );
		std::string curDir( cwd );

	  	// 1. search in current directory
		std::ifstream cfgFile( config_filename_.c_str() );
		if( cfgFile.is_open() )
		{
			config_directory_ = curDir;
			data_directory_ = curDir;
			cfgFile.close();
			location_ = LOC_DIRECTORY;
			std::cout << "Config file found in current directory (" << config_directory_ << ")...\n";
			return;
		}

	  	// 2. search in user config path
	  	std::string userCfgPath( localPath + std::string( "/" ) + config_filename_ );
		std::ifstream userCfgFile( userCfgPath.c_str() );
		if( userCfgFile.is_open() )
		{
			config_directory_ = userPath;
			data_directory_ = userPath;
			userCfgFile.close();
			location_ = LOC_USER;
			std::cout << "Config file found in user directory (" << config_directory_ << ")...\n";
			return;
		}

	  	// 3. search in system config path
	  	std::string sysCfgPath( sysPath + std::string( "/" ) + config_filename_ );
		std::ifstream sysCfgFile(  sysCfgPath.c_str());
		if( sysCfgFile.is_open() )
		{
			config_directory_ = sysPath;
			data_directory_ = "/usr/share/bonzomatic";
			sysCfgFile.close();
			location_ = LOC_SYSTEM;
			std::cout << "Config file found in system config path (" << config_directory_ << ")...\n";
			return;
		}

		config_directory_ = curDir;
		location_ = LOC_NONE;
		std::cout << "No config file found, using default settings...\n";
	}

}