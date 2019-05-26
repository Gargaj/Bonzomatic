#include "Config.h"

#include <fstream>
#include <iostream>
#include <vector>

#include <unistd.h>


namespace Config
{
	ApplicationSettings::ApplicationSettings( const std::string& cfg_filename )
		: _location( Location::NONE ), _config_filename( cfg_filename )
	{

	}

	void ApplicationSettings::load()
	{
		find_location();

		if( _location != Location::NONE )
		{
			read_config();
		}


	}

	void ApplicationSettings::read_config()
	{
		std::string config_path( _config_directory + "/" + _config_filename );
		std::ifstream fconf( config_path );
		fconf.seekg( 0, std::ios::end );
		size_t file_size_in_byte = fconf.tellg();
		std::vector< char > data;
		data.resize( file_size_in_byte );
		fconf.seekg( 0, std::ios::beg );
		fconf.read( &data[0], file_size_in_byte );
		_options.parse( &data[ 0 ] );
	}

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
		std::ifstream cfgFile( _config_filename );
		if( cfgFile.is_open() )
		{
			_config_directory = curDir;
			_data_directory = curDir;
			cfgFile.close();
			_location = Location::DIRECTORY;
			std::cout << "Config file found in current directory (" << _config_directory << ")...\n";
			return;
		}

	  	// 2. search in user config path
		std::ifstream userCfgFile( localPath + std::string( "/" ) + _config_filename );
		if( userCfgFile.is_open() )
		{
			_config_directory = userPath;
			_data_directory = userPath;
			userCfgFile.close();
			_location = Location::USER;
			std::cout << "Config file found in user directory (" << _config_directory << ")...\n";
			return;
		}

	  	// 3. search in system config path
		std::ifstream sysCfgFile( sysPath + std::string( "/" ) + _config_filename );
		if( sysCfgFile.is_open() )
		{
			_config_directory = sysPath;
			_data_directory = "/usr/share/bonzomatic";
			sysCfgFile.close();
			_location = Location::SYSTEM;
			std::cout << "Config file found in system config path (" << _config_directory << ")...\n";
			return;
		}

		_config_directory = curDir;
		_location = Location::NONE;
		std::cout << "No config file found, using default ApplicationSettings...\n";
	}
}