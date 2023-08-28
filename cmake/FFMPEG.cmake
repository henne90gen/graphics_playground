
# FFMPEG
function(find_ffmpeg_libraries)
    set(FFMPEG_FOUND TRUE)
    foreach (lib ${ARGV})
        message("-- Looking for ${lib}")
        find_library(${lib}_LIBRARY ${lib})

        if (NOT ${lib}_LIBRARY)
            message("-- Loooking for ${lib} - not found")
            set(FFMPEG_FOUND FALSE)
            break()
        endif ()

        add_library(${lib} SHARED IMPORTED)
        set_target_properties(${lib} PROPERTIES IMPORTED_LOCATION ${${lib}_LIBRARY})
        list(APPEND FFMPEG_LIBRARY ${lib})

        find_path(${lib}_INCLUDE_DIR lib${lib}/${lib}.h)
        if (${lib}_INCLUDE_DIR)
            list(APPEND FFMPEG_INCLUDE_DIR ${${lib}_INCLUDE_DIR})
        endif ()

        message("-- Looking for ${lib} - found")
    endforeach ()

    if (FFMPEG_FOUND)
        set(FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} PARENT_SCOPE)
        set(FFMPEG_LIBRARY ${FFMPEG_LIBRARY} PARENT_SCOPE)
        add_definitions(-DFFMPEG_FOUND=1)
    else ()
        add_definitions(-DFFMPEG_FOUND=0)
    endif ()
endfunction()

find_ffmpeg_libraries(x264 swscale avutil avformat avcodec)
