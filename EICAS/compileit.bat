cl /O2 /MD /c eicas.cpp ..\splash\tgaload.cpp ..\common\utils.cpp ..\common\fonts.cpp 
link eicas.obj tgaload.obj utils.obj fonts.obj user32.lib FSUIPC_User.lib 
