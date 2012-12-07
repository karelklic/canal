#include "Wrapper.h"
#include <clang/Driver/Driver.h>
#include <clang/Driver/Option.h>
#include <clang/Frontend/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Config/config.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/Timer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/system_error.h>

static llvm::sys::Path
GetExecutablePath(const char *Argv0, bool CanonicalPrefixes)
{
  if (!CanonicalPrefixes)
    return llvm::sys::Path(Argv0);

  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *P = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::Path::GetMainExecutable(Argv0, P);
}

static const char *
SaveStringInSet(std::set<std::string> &SavedStrings,
                llvm::StringRef S)
{
  return SavedStrings.insert(S).first->c_str();
}

static void ExpandArgsFromBuf(const char *Arg,
                              llvm::SmallVectorImpl<const char*> &ArgVector,
                              std::set<std::string> &SavedStrings)
{
  const char *FName = Arg + 1;
  llvm::OwningPtr<llvm::MemoryBuffer> MemBuf;
  if (llvm::MemoryBuffer::getFile(FName, MemBuf)) {
    ArgVector.push_back(SaveStringInSet(SavedStrings, Arg));
    return;
  }

  const char *Buf = MemBuf->getBufferStart();
  char InQuote = ' ';
  std::string CurArg;

  for (const char *P = Buf; ; ++P) {
    if (*P == '\0' || (isspace(*P) && InQuote == ' ')) {
      if (!CurArg.empty()) {

        if (CurArg[0] != '@') {
          ArgVector.push_back(SaveStringInSet(SavedStrings, CurArg));
        } else {
          ExpandArgsFromBuf(CurArg.c_str(), ArgVector, SavedStrings);
        }

        CurArg = "";
      }
      if (*P == '\0')
        break;
      else
        continue;
    }

    if (isspace(*P)) {
      if (InQuote != ' ')
        CurArg.push_back(*P);
      continue;
    }

    if (*P == '"' || *P == '\'') {
      if (InQuote == *P)
        InQuote = ' ';
      else if (InQuote == ' ')
        InQuote = *P;
      else
        CurArg.push_back(*P);
      continue;
    }

    if (*P == '\\') {
      ++P;
      if (*P != '\0')
        CurArg.push_back(*P);
      continue;
    }
    CurArg.push_back(*P);
  }
}

static void
ExpandArgv(int argc, const char **argv,
           llvm::SmallVectorImpl<const char*> &ArgVector,
           std::set<std::string> &SavedStrings)
{
    for (int i = 0; i < argc; ++i)
    {
        const char *Arg = argv[i];
        if (Arg[0] != '@')
        {
            ArgVector.push_back(SaveStringInSet(SavedStrings, std::string(Arg)));
            continue;
        }

        ExpandArgsFromBuf(Arg, ArgVector, SavedStrings);
    }
}

static void
ParseProgName(llvm::SmallVectorImpl<const char *> &ArgVector,
              std::set<std::string> &SavedStrings,
              clang::driver::Driver &TheDriver)
{
  // Try to infer frontend type and default target from the program name.

  // suffixes[] contains the list of known driver suffixes.
  // Suffixes are compared against the program name in order.
  // If there is a match, the frontend type is updated as necessary (CPP/C++).
  // If there is no match, a second round is done after stripping the last
  // hyphen and everything following it. This allows using something like
  // "clang++-2.9".

  // If there is a match in either the first or second round,
  // the function tries to identify a target as prefix. E.g.
  // "x86_64-linux-clang" as interpreted as suffix "clang" with
  // target prefix "x86_64-linux". If such a target prefix is found,
  // is gets added via -ccc-host-triple as implicit first argument.
  static const struct {
    const char *Suffix;
    bool IsCXX;
    bool IsCPP;
  } suffixes [] = {
    { "clang", false, false },
    { "clang++", true, false },
    { "clang-c++", true, false },
    { "clang-cc", false, false },
    { "clang-cpp", false, true },
    { "clang-g++", true, false },
    { "clang-gcc", false, false },
    { "cc", false, false },
    { "cpp", false, true },
    { "++", true, false },
  };
  std::string ProgName(llvm::sys::path::stem(ArgVector[0]));
  llvm::StringRef ProgNameRef(ProgName);
  llvm::StringRef Prefix;

  for (int Components = 2; Components; --Components) {
    bool FoundMatch = false;
    size_t i;

    for (i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
      if (ProgNameRef.endswith(suffixes[i].Suffix)) {
        FoundMatch = true;
        if (suffixes[i].IsCXX)
          TheDriver.CCCIsCXX = true;
        if (suffixes[i].IsCPP)
          TheDriver.CCCIsCPP = true;
        break;
      }
    }

    if (FoundMatch) {
        llvm::StringRef::size_type LastComponent = ProgNameRef.rfind('-',
                                                                     ProgNameRef.size() - strlen(suffixes[i].Suffix));

        if (LastComponent != llvm::StringRef::npos)
            Prefix = ProgNameRef.slice(0, LastComponent);

        break;
    }

    llvm::StringRef::size_type LastComponent = ProgNameRef.rfind('-');
    if (LastComponent == llvm::StringRef::npos)
      break;
    ProgNameRef = ProgNameRef.slice(0, LastComponent);
  }

  if (Prefix.empty())
    return;

  std::string IgnoredError;
  if (llvm::TargetRegistry::lookupTarget(Prefix, IgnoredError)) {
      llvm::SmallVectorImpl<const char *>::iterator it = ArgVector.begin();
    if (it != ArgVector.end())
      ++it;
    ArgVector.insert(it, SaveStringInSet(SavedStrings, Prefix));
    ArgVector.insert(it,
      SaveStringInSet(SavedStrings, std::string("-ccc-host-triple")));
  }
}

Wrapper::Wrapper(int argc_, const char **argv_)
{
    llvm::sys::PrintStackTraceOnErrorSignal();
    llvm::PrettyStackTraceProgram X(argc_, argv_);

    std::set<std::string> SavedStrings;
    llvm::SmallVector<const char*, 256> argv;

    ExpandArgv(argc_, argv_, argv, SavedStrings);

    bool CanonicalPrefixes = true;
    for (int i = 1, size = argv.size(); i < size; ++i) {
        if (llvm::StringRef(argv[i]) == "-no-canonical-prefixes") {
            CanonicalPrefixes = false;
            break;
        }
    }

    llvm::sys::Path Path = GetExecutablePath(argv[0], CanonicalPrefixes);

    clang::TextDiagnosticPrinter *DiagClient
        = new clang::TextDiagnosticPrinter(llvm::errs(), clang::DiagnosticOptions());
    DiagClient->setPrefix(llvm::sys::path::stem(Path.str()));
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, DiagClient);

#ifdef CLANG_IS_PRODUCTION
    const bool IsProduction = true;
#else
    const bool IsProduction = false;
#endif
    clang::driver::Driver TheDriver(Path.str(), llvm::sys::getHostTriple(),
                                    "a.out", IsProduction, Diags);

    // Attempt to find the original path used to invoke the driver, to determine
    // the installed path. We do this manually, because we want to support that
    // path being a symlink.
    {
        llvm::SmallString<128> InstalledPath(argv[0]);

        // Do a PATH lookup, if there are no directory components.
        if (llvm::sys::path::filename(InstalledPath) == InstalledPath) {
            llvm::sys::Path Tmp = llvm::sys::Program::FindProgramByName(
                llvm::sys::path::filename(InstalledPath.str()));
            if (!Tmp.empty())
                InstalledPath = Tmp.str();
        }
        llvm::sys::fs::make_absolute(InstalledPath);
        InstalledPath = llvm::sys::path::parent_path(InstalledPath);
        bool exists;
        if (!llvm::sys::fs::exists(InstalledPath.str(), exists) && exists)
            TheDriver.setInstalledDir(InstalledPath);
    }

    llvm::InitializeAllTargets();
    ParseProgName(argv, SavedStrings, TheDriver);

    llvm::OwningPtr<clang::driver::Compilation> C(TheDriver.BuildCompilation(argv));
    int Res = 0;
    const clang::driver::Command *FailingCommand = 0;
    if (C.get())
        Res = TheDriver.ExecuteCompilation(*C, FailingCommand);

    // If result status is < 0, then the driver command signalled an error.
    // In this case, generate additional diagnostic information if possible.
    if (Res < 0)
        TheDriver.generateCompilationDiagnostics(*C, FailingCommand);

    // If any timers were active but haven't been destroyed yet, print their
    // results now.  This happens in -disable-free mode.
    llvm::TimerGroup::printAll(llvm::errs());

    llvm::llvm_shutdown();

    //return Res;
}

int
Wrapper::run()
{
    return 0;
}
