### CMakeLists.txt ---
## Description: CMakeLists for moc client
### Code:

cmake_minimum_required ( VERSION 2.6 )
project ( moc_client )

IF ( WIN32 )
    MESSAGE (STATUS "Please use Unix Makefile generator. ")
ENDIF ( WIN32 )

# CGLAGS and CPPFLAGS
set (CMAKE_C_FLAGS "-g -Wall -std=c99 -D_XOPEN_SOURCE=600 -fno-strict-aliasing -D_GNU_SOURCE -D_DARWIN_C_SOURCE")

if ( NEL EQUAL 1 )
    set ( EVENTLOOP 0 )
else ( NEL EQUAL 1 )
    set ( EVENTLOOP 1 )
endif ( NEL EQUAL 1 )

# head file directory
include_directories ( clearsilver )

add_subdirectory ( clearsilver )

set ( SRC_LIST internal.c lerr.c moc.c mtrace.c packet.c tcp.c )

if ( EVENTLOOP )
    message (STATUS "building library with eventloop supported.")
    set ( SRC_LIST ${SRC_LIST} eloop.c mcbk.c mscli.c )
    set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DEVENTLOOP" )
endif ( EVENTLOOP )

message ( ${SRC_LIST} )

add_library ( moc SHARED ${SRC_LIST} )

target_link_libraries ( moc cs )
