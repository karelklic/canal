include_directories(${CANAL_SOURCE_DIR} ${READLINE_INCLUDE_DIRS} ${LIBELF_INCLUDE_DIRS})
link_directories(${CANAL_SOURCE_DIR}/lib)

add_executable(canal-tool
    Canal.cpp
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
    CommandSet.cpp
    CommandShell.cpp
    CommandShow.cpp
    CommandStart.cpp
    CommandStep.cpp
    IteratorCallback.cpp
    State.cpp
    Utils.cpp
    WrapperGcc.cpp)

target_link_libraries(canal-tool
    canal
    ${READLINE_LIBRARIES}
    ${LIBELF_LIBRARIES})

set_target_properties(canal-tool
    PROPERTIES
    OUTPUT_NAME canal)

