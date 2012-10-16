#include "CommandFile.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include "lib/Config.h"
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/MemoryBuffer.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>

#if (defined HAVE_LIBELF_H && defined HAVE_GELF_H && defined HAVE_LIBELF)
#  define WITH_ELFUTILS
#endif

#ifdef WITH_ELFUTILS
#include <libelf.h>
#include <gelf.h>
#endif // WITH_ELFUTILS

CommandFile::CommandFile(Commands &commands)
    : Command("file",
              "",
              "Use FILE as program to be interpreted",
#ifdef WITH_ELFUTILS
              "Use FILE as program to be interpreted.  FILE can be a LLVM "
              "bitcode file (both binary and textual form is allowed), an ELF "
              "binary with .note.llvm section containing the LLVM bitcode, "
              "or a standalone source file with .c or .cpp extension.",
#else // WITH_ELFUTILS
              "Use FILE as program to be interpreted.  FILE can be a LLVM "
              "bitcode file (both binary and textual form is allowed), "
              "or a standalone source file with .c or .cpp extension.",
#endif // WITH_ELFUTILS
              commands)
{
}

std::vector<std::string>
CommandFile::getCompletionMatches(const std::vector<std::string> &args,
                                  int pointArg,
                                  int pointArgOffset) const
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

static void
printError(const llvm::SMDiagnostic &err)
{
    std::string s;
    llvm::raw_string_ostream os(s);
#if (LLVM_MAJOR == 3 && LLVM_MINOR >= 1) || LLVM_MAJOR > 3
    err.print(NULL, os, false);
#else
    err.Print(NULL, os);
#endif
    os.flush();
    printf("%s", s.c_str());
}

static llvm::Module *
loadAsElfFile(const std::string &path, bool &error)
{
    error = false;

#ifdef WITH_ELFUTILS
    // Open the ELF file.
    int fd = open(path.c_str(), O_RDONLY, 0);
    if (fd == -1)
    {
        printf("Cannot open `%s': %s\n", path.c_str(), strerror(errno));
        error = true;
        return NULL;
    }

    // Get Elf object for the file.
    elf_version(EV_CURRENT);
    Elf *elf = elf_begin(fd, ELF_C_READ, NULL);

    if (!elf)
    {
        printf("Cannot create ELF descriptor: %s\n", elf_errmsg(-1));
        close(fd);
        error = true;
        return NULL;
    }

    llvm::Module *module = NULL;
    if (elf_kind(elf) == ELF_K_ELF)
    {
        // Get Elf header.
        GElf_Ehdr ehdr_mem, *ehdr = gelf_getehdr (elf, &ehdr_mem);
        if (ehdr)
        {
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
                llvm::StringRef data_ref((const char*)data->d_buf,
                                         data->d_size);

                llvm::MemoryBuffer *data_buffer =
                    llvm::MemoryBuffer::getMemBufferCopy(data_ref);

                llvm::LLVMContext &context = llvm::getGlobalContext();
                llvm::SMDiagnostic err;
                module = llvm::ParseIR(data_buffer, err, context);
                if (!module)
                {
                    puts("Failed to load the module.");
                    printError(err);
                    error = true;
                }
                break;
            }

            if (!module)
            {
                puts("Failed to find .note.llvm section in the ELF file.");
                error = true;
            }
        }
        else
        {
            printf("Cannot get ELF header: %s\n", elf_errmsg (-1));
            error = true;
        }
    }

    elf_end(elf);
    close(fd);
    return module;
#else // WITH_ELFUTILS
    return NULL;
#endif // WITH_ELFUTILS

}

static llvm::Module *
loadAsBitcodeFile(const std::string &path, bool &error)
{
    error = false;
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::SMDiagnostic err;
    llvm::Module *module = llvm::ParseIRFile(path, err, context);
    if (!module)
    {
        error = true;
        puts("Failed to load the module.");
        printError(err);
    }

    return module;
}

static llvm::Module *
loadAsSourceFile(const std::string &path, bool &error)
{
    const char *extensions[] = { ".c", ".cpp", ".C", ".cxx", NULL };
    error = false;

    enum {
        PlainC,
        CPlusPlus,
        Unknown
    } type = Unknown;
    std::string bitcodePath;

    for (int i = 0; extensions[i]; ++i)
    {
        if (path.length() <= strlen(extensions[i]))
            continue;

        if (path.substr(path.length() - strlen(extensions[i])) == extensions[i])
        {
            type = (i == 0) ? PlainC : CPlusPlus;
            bitcodePath = path.substr(0, path.length() - strlen(extensions[i])) + ".s";
            break;
        }
    }

    if (type == Unknown)
        return NULL;

    // Compile via clang and obtain bitcode.
    char **argv;
    argv = (char**)malloc(7 * sizeof(char*));
    argv[0] = (char*)"clang";
    argv[1] = (char*)"-emit-llvm";
    argv[2] = (char*)"-S";
    argv[3] = (char*)path.c_str();
    argv[4] = (char*)"-o";
    argv[5] = (char*)bitcodePath.c_str();
    argv[6] = NULL;

    // Print the command.
    for (int i = 0; argv[i]; ++i)
    {
        if (i > 0)
            printf(" ");
        printf("%s", argv[i]);
    }
    puts("");

    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Failed to fork: %s\n", strerror(errno));
        free(argv);
        error = true;
        return NULL;
    }

    if (pid == 0)
    {
        execvp(argv[0], argv);
        _exit(127);
    }

    safeWaitPid(pid, NULL, 0);
    free(argv);

    return loadAsBitcodeFile(bitcodePath, error);
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
        puts("Argument required (a file).");
        return;
    }

    bool error;
    llvm::Module *module = loadAsElfFile(args[1], error);
    if (error)
        return;

    if (!module)
    {
        module = loadAsSourceFile(args[1], error);
        if (error)
            return;

        if (!module)
        {
            module = loadAsBitcodeFile(args[1], error);
            if (error || !module)
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
