cmake_minimum_required(VERSION 3.14.0)
project(Pico_Bidir_DShot VERSION 1.0.2 LANGUAGES CXX)

add_library(Pico_Bidir_DShot STATIC)

file(GLOB_RECURSE LIB_SOURCES 
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

target_sources(Pico_Bidir_DShot
    PRIVATE
    ${LIB_SOURCES}
)

target_include_directories(Pico_Bidir_DShot
    PUBLIC
        src/
)

target_link_libraries(Pico_Bidir_DShot hardware_pio)


