# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources
	"common/common.cpp"
	"devices/AllDevices.cpp"
	"devices/Device.cpp"
	"devices/K210/K210.cpp"
	"main.cpp"
	"modules/AllModules.cpp"
	"modules/Module.cpp"
	"modules/HttpServer/HttpServer.cpp"
	"modules/k210_esp32_com/K210ESP32Communication.cpp"
	"modules/wifi/WifiModule.cpp"
	"tasks/tasks.cpp"
)

idf_component_register(SRCS ${app_sources})
