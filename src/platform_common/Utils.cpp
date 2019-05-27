#include <vector>
#include <string>
#include <fstream>

namespace Utils
{

	bool read_file( const std::string& filename, std::vector< char >& out )
	{
		std::ifstream fp( filename );
		if( !fp.is_open() ) return false;
		fp.seekg( 0, std::ios::end );
		size_t file_size_in_byte = fp.tellg();
		out.resize( file_size_in_byte );
		fp.seekg( 0, std::ios::beg );
		fp.read( &out[ 0 ], file_size_in_byte );
		return true;
	}

}