import argparse
import config

parser = argparse.ArgumentParser()
parser.add_argument("-c", action="store_true")
parser.add_argument("-o")
parser.add_argument("-I", action="append")
parser.add_argument("-v", action="store_true")

def is_linking(args, remainder):
    if args.c:
        return False

    hasObject = False
    hasSource = False
    for r in remainder:
        if r.endswith(".c"):
            hasSource = True
        if r.endswith(".o"):
            hasObject = True

    return hasObject or hasSource

def get_linking_commands(args, remainder):
    subresult = [config.LLVM_LINK]

    # Output
    linker_output = args.o if args.o else "a.out"
    llvm_output = "{0}.llvm".format(linker_output)
    subresult.append("-o={0}".format(llvm_output))

    # Input
    for r in remainder:
        if r.endswith(".c"):
            subresult.append("{0}.o.llvm".format(r[:-2]))
        if r.endswith(".o"):
            subresult.append("{0}.llvm".format(r))

    result = [subresult]
    result.append(["objcopy",
                   "--add-section",
                   ".note.llvm={0}".format(llvm_output),
                   linker_output])

    return result

def is_compile(args, remainder):
    for r in remainder:
        if r.endswith(".c"):
            return True

    return False

def get_compile_commands(args, remainder):
    result = []

    cfiles = filter(lambda r: r.endswith(".c"), remainder)
    for cfile in cfiles:
        subresult = [config.CLANG]

        subresult.append(cfile)
        subresult.append("-c")
        subresult.append("-emit-llvm")
        subresult.append("-o")
        subresult.append("{0}.o.llvm".format(cfile[:-2]))

        if args.I:
            for i in args.I:
                subresult.append("-I{0}".format(i))

        if args.v:
            subresult.append("-v")

        for r in remainder:
            if r.endswith(".o") or r.endswith(".c"):
                pass
            else:
                subresult.append(r)

        result.append(subresult)

    return result


def is_other(args, remainder):
    return not is_linking(args, remainder) and not is_compile(args, remainder)

def get_other_command(args, remainder):
    result = [config.CLANG]
    if args.v:
        result.append("-v")

    for r in remainder:
        result.append(r)

    return result
