project(CFS_CAMERA C)


include_directories(mission_inc)
include_directories(platform_inc)
include_directories(src)

aux_source_directory(src APP_SRC_FILES)

# Create the app module
add_cfe_app(camera ${APP_SRC_FILES} )

