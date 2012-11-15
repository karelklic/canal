include_directories(${CANAL_SOURCE_DIR})
include_directories(${READLINE_INCLUDE_DIR})
link_directories(${CANAL_SOURCE_DIR}/lib)

add_executable(canal-tool Canal.cpp
CommandBreak.cpp
CommandCd.cpp
CommandContinue.cpp
CommandDump.cpp
Command.cpp
CommandFile.cpp
CommandFinish.cpp
CommandHelp.cpp
CommandInfo.cpp
CommandPrint.cpp
CommandPwd.cpp
CommandQuit.cpp
CommandRun.cpp
Commands.cpp
CommandShell.cpp
CommandShow.cpp
CommandStart.cpp
CommandStep.cpp
IteratorCallback.cpp
State.cpp
Utils.cpp)

target_link_libraries(canal-tool canal ${READLINE_LIBRARY} ${LIBELF_LIBRARIES})
SET_TARGET_PROPERTIES(canal-tool PROPERTIES OUTPUT_NAME canal)
