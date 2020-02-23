cl /Ob2gity /Gs /MD /c boeing_eadi.cpp common_eadi.cpp compass.cpp ..\splash\tgaload.cpp ..\common\utils.cpp ..\common\fonts.cpp 
link boeing_eadi.obj common_eadi.obj compass.obj tgaload.obj utils.obj fonts.obj user32.lib FSUIPC_User.lib /OUT:Release\eadi.exe
