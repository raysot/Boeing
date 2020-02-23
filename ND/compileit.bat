cl /Ob2gity /Gs /MD /c nd.cpp compass.cpp ..\splash\tgaload.cpp ..\common\utils.cpp ..\common\fonts.cpp 
link nd.obj compass.obj tgaload.obj utils.obj fonts.obj user32.lib FSUIPC_User.lib /OUT:Release\nd.exe
