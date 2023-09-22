#include "Checksum.hpp"
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace OpenVic
{
    void Checksum::calculate_checksum(String directory_path)
    {  
        XXH3_state_t* const state = XXH3_createState();
        if (state == NULL) abort();

        size_t const bufferSize = 8192;
        PackedByteArray buffer;
        buffer.resize(bufferSize);

        /* Initialize state with selected seed */
        XXH64_hash_t const seed = 0;   /* or any other value */
        if (XXH3_64bits_reset_withSeed(state, seed) == XXH_ERROR) abort();
        checksum_dir(state, buffer, directory_path);
        
        XXH3_freeState(state);
    }

    void Checksum::checksum_dir(XXH3_state_t* const state, PackedByteArray buffer, String directory_path){
        // Needed to be separate function to get subdirectory access
        Ref<DirAccess> dir = DirAccess::open(directory_path);

        if (dir.is_valid() && DirAccess::get_open_error() == OK)
        {
            dir->list_dir_begin();
            String file_name = dir->get_next();

            while (file_name != "")
            {
                if(file_name == ".godot")
                {
                    file_name = dir->get_next();
                }                

                if (dir->current_is_dir())
                {
                    String file_path = directory_path + file_name + '/';
                    // Recurse into subdirectory
                    checksum_dir(state, buffer, file_path);
                }
                else
                {
                    String file_path = directory_path + file_name;
                    
                    Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::READ);
                    if (file.is_valid() && FileAccess::get_open_error() == OK)
                    {   
                        buffer = file->get_buffer(file->get_length());
                        if (XXH3_64bits_update(state, buffer.ptr(), buffer.size()) == XXH_ERROR) abort();
                        file->close();
                    }
                }

                file_name = dir->get_next();
            }
            hash_checksum = XXH3_64bits_digest(state);
            dir->list_dir_end();
        }
    }


    XXH64_hash_t Checksum::get_checksum() const
    {
        return hash_checksum;
    }

    void Checksum::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("calculate_directory_checksum"), &Checksum::calculate_checksum);
        ClassDB::bind_method(D_METHOD("get_checksum"), &Checksum::get_checksum);
    }

}
