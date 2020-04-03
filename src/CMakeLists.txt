# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"common/common.cpp"
	"devices/Device.cpp"
	"devices/K210/K210.cpp"
	"main.cpp"
	"modules/Module.cpp"
	"modules/wifi/WifiModule.cpp"
)

idf_component_register(SRCS ${app_sources})
