#pragma once

#include "jsonxx.h"

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

		ApplicationSettings( const std::string& cfg_filename );

		void load();

		const jsonxx::Object get_options() const { return _options; }
		const std::string get_config_directory() const { return _config_directory; }
		const std::string get_data_directory() const { return _data_directory; }

	private:
		void find_location();
		void read_config();

		Location _location;
		std::string _config_filename;
		jsonxx::Object _options;

		std::string _config_directory;
		std::string _data_directory;
	};





}