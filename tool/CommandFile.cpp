#include "CommandFile.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/MemoryBuffer.h>
#include <cstdio>
#include <sys/types.h>
#include <dirent.h>
#include <libelf.h>
#include <gelf.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>

CommandFile::CommandFile(Commands &commands)
    : Command("file",
              "",
              "Use FILE as program to be interpreted",
              "Use FILE as program to be interpreted.  Both binary and textual LLVM bitcode files can be loaded.",
              commands)
{
}

std::vector<std::string>
CommandFile::getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const
{
    std::vector<std::string> result;
    std::string arg = args[pointArg].substr(0, pointArgOffset);
    std::string dirPath("./");
    bool defaultDirPath = true;
    size_t dirPos = arg.rfind("/");
    if (dirPos != std::string::npos)
    {
        dirPath = arg.substr(0, dirPos + 1);
        defaultDirPath = false;
        if (arg.length() > dirPos + 1)
            arg = arg.substr(dirPos + 1);
        else
            arg = "";
    }

    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
        return result;

    struct dirent *dirent;
    bool lastIsDir = false;
    for (dirent = readdir(dir); dirent != NULL; dirent = readdir(dir))
    {
        if (!dirent || !dirent->d_name)
            continue;

        if (0 == strncmp(dirent->d_name, arg.c_str(), arg.length()))
        {
            std::stringstream ss;
            if (!defaultDirPath)
                ss << dirPath;
            ss << dirent->d_name;
            if (dirent->d_type == DT_DIR)
            {
                lastIsDir = true;
                ss << "/";
            }

            result.push_back(ss.str());
        }
    }

    if (result.size() == 1 && lastIsDir)
    {
        dirPath = result[0].substr(0, result[0].length());
        DIR *dir = opendir(dirPath.c_str());
        struct dirent *dirent;
        result.clear();
        for (dirent = readdir(dir);
             dirent != NULL;
             dirent = readdir(dir))
        {
            if (!dirent || !dirent->d_name)
                continue;

            std::stringstream ss;
            ss << dirPath << dirent->d_name;
            result.push_back(ss.str());
        }
    }

    return result;
}

void
CommandFile::run(const std::vector<std::string> &args)
{
    if (args.size() > 2)
    {
        puts("Too many parameters.");
        return;
    }
    else if (args.size() < 2)
    {
        puts("Argument required (module file).");
        return;
    }

    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::SMDiagnostic err;
    llvm::Module *module = NULL;

    // Open the ELF file.
    int fd = open(args[1].c_str(), O_RDONLY, 0);
    if (fd == -1)
    {
        printf("Cannot open `%s': %s\n", args[1].c_str(), strerror(errno));
        return;
    }

    // Get Elf object for the file.
    elf_version(EV_CURRENT);
    Elf *elf = elf_begin(fd, ELF_C_READ, NULL);

    if (!elf)
    {
        printf("Cannot create ELF descriptor: %s\n", elf_errmsg(-1));
        close(fd);
        return;
    }

    if (elf_kind(elf) == ELF_K_ELF)
    {
        // Get Elf header.
        GElf_Ehdr ehdr_mem, *ehdr = gelf_getehdr (elf, &ehdr_mem);
        if (!ehdr)
        {
            printf("Cannot get ELF header: %s\n", elf_errmsg (-1));
            elf_end(elf);
            close(fd);
            return;
        }

        Elf_Scn *section = NULL;
        while ((section = elf_nextscn(elf, section)) != NULL)
        {
            GElf_Shdr shdr_mem, *shdr;

            /* Get the section header data.  */
            shdr = gelf_getshdr(section, &shdr_mem);
            if (shdr->sh_type == SHT_NOBITS)
                continue;

            if ((shdr->sh_flags & SHF_GROUP) != 0)
                /* Ignore the section.  */
                continue;

            const char *section_name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
            if (!section_name)
                continue;

            if (0 != strcmp(section_name, ".note.llvm"))
                continue;

            Elf_Data *data = elf_getdata(section, NULL);
            llvm::StringRef data_ref((const char*)data->d_buf, data->d_size);
            llvm::MemoryBuffer *data_buffer = llvm::MemoryBuffer::getMemBufferCopy(data_ref);
            module = llvm::ParseIR(data_buffer, err, context);
            break;
        }

        if (!module)
        {
            puts("Failed to find .llvm section in the ELF file.");
            return;
        }

        elf_end(elf);
        close(fd);
    }
    else
    {
        module = llvm::ParseIRFile(args[1], err, context);
        if (!module)
        {
            puts("Failed to load the module.");
            std::string s;
            llvm::raw_string_ostream os(s);
#if (LLVM_MAJOR == 3 && LLVM_MINOR >= 1) || LLVM_MAJOR > 3
            err.print(NULL, os, false);
#else
            err.Print(NULL, os);
#endif
            os.flush();
            printf("%s", s.c_str());
            return;
        }
    }

    if (mCommands.getState() && mCommands.getState()->isInterpreting())
    {
        puts("A program is being interpreted already.");
        bool agreed = askYesNo("Are you sure you want to change the file?");
        if (!agreed)
        {
            puts("File not changed.");
            delete module;
            return;
        }
    }

    mCommands.createState(module);
    puts("Module loaded.");
}
