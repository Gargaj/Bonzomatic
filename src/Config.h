#pragma once

#include "jsonxx.h"

#include "Utils.h"

#include <string>

namespace Config
{

	class ApplicationSettings
	{
	public:
		enum Location
		{
			SYSTEM,
			USER,
			DIRECTORY,
			NONE
		};

		ApplicationSettings( const std::string& cfg_filename )
			: location_( Location::NONE ), config_filename_( cfg_filename ) {}

		void load()
		{
			find_location();
			if( location_ != Location::NONE )
			{
				read_config();
			}
		}

		const jsonxx::Object get_options() const { return options_; }
		const std::string get_config_directory_() const { return config_directory_; }
		const std::string get_data_directory() const { return data_directory_; }

	private:
		void find_location();
		void read_config()
		{
			std::string config_path( config_directory_ + "/" + config_filename_ );
			std::vector< char > data;
			Utils::read_file( config_path, data );
			options_.parse( &data[ 0 ] );
		}

		Location location_;
		std::string config_filename_;
		jsonxx::Object options_;

		std::string config_directory_;
		std::string data_directory_;
	};





}