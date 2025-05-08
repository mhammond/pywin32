import re
import sys
import types


def ad_escape(s):
    return re.sub(r"([^<]*)<([^>]*)>", r"\g<1>\\<\g<2>\\>", s)


class DocInfo:
    def __init__(self, name, ob):
        self.name = name
        self.ob = ob
        self.short_desc = ""
        self.desc = ""


def BuildArgInfos(ob):
    ret = []
    vars = list(ob.__code__.co_varnames[: ob.__code__.co_argcount])
    vars.reverse()  # for easier default checking.
    defs = list(ob.__defaults__ or [])
    for i, n in enumerate(vars):
        info = DocInfo(n, ob)
        info.short_desc = info.desc = n
        info.default = ""
        if defs:
            default = repr(defs.pop())
            # the default may be an object, so the repr gives '<...>' - and
            # the angle brackets screw autoduck.
            info.default = default.replace("<", "").replace(">", "")
        ret.append(info)
    ret.reverse()
    return ret


def BuildInfo(name, ob):
    ret = DocInfo(name, ob)
    docstring = ob.__doc__ or ""
    ret.desc = ret.short_desc = docstring.strip()
    if ret.desc:
        ret.short_desc = ret.desc.splitlines()[0]
    return ret


def should_build_function(build_info):
    return build_info.ob.__doc__ and not build_info.ob.__name__.startswith("_")


# docstring aware paragraph generator.  Isn't there something in docutils
# we can use?
def gen_paras(val):
    chunks = []
    in_docstring = False
    for line in val.splitlines():
        line = ad_escape(line.strip())
        if not line or (not in_docstring and line.startswith(">>> ")):
            if chunks:
                yield chunks
            chunks = []
            if not line:
                in_docstring = False
                continue
            in_docstring = True
        chunks.append(line)
    yield chunks or [""]


def format_desc(desc):
    # A little complicated!  Given the docstring for a module, we want to:
    # write:
    # 'first_para_of_docstring'
    # '@comm next para of docstring'
    # '@comm next para of docstring' ... etc
    # BUT - also handling embedded doctests, where we write
    # '@iex >>> etc.'
    if not desc:
        return ""
    g = gen_paras(desc)
    first = next(g)
    chunks = [first[0]]
    chunks.extend(["// " + l for l in first[1:]])
    for lines in g:
        first = lines[0]
        if first.startswith(">>> "):
            prefix = "// @iex \n// "
        else:
            prefix = "\n// @comm "
        chunks.append(prefix + first)
        chunks.extend(["// " + l for l in lines[1:]])
    return "\n".join(chunks)


def build_module(fp, mod_name):
    __import__(mod_name)
    mod = sys.modules[mod_name]
    functions = []
    classes = []
    constants = []
    for name, ob in mod.__dict__.items():
        if name.startswith("_"):
            continue
        if hasattr(ob, "__module__") and ob.__module__ != mod_name:
            continue
        if type(ob) == type:
            classes.append(BuildInfo(name, ob))
        elif isinstance(ob, types.FunctionType):
            functions.append(BuildInfo(name, ob))
        elif name.upper() == name and isinstance(ob, (int, str)):
            constants.append((name, ob))
    info = BuildInfo(mod_name, mod)
    print(f"// @module {mod_name}|{format_desc(info.desc)}", file=fp)
    functions = [f for f in functions if should_build_function(f)]
    for ob in functions:
        print(f"// @pymeth {ob.name}|{ob.short_desc}", file=fp)
    for ob in classes:
        # only classes with docstrings get printed.
        if not ob.ob.__doc__:
            continue
        ob_name = mod_name + "." + ob.name
        print(f"// @pyclass {ob.name}|{ob.short_desc}", file=fp)
    for ob in functions:
        print(
            f"// @pymethod |{mod_name}|{ob.name}|{format_desc(ob.desc)}",
            file=fp,
        )
        for ai in BuildArgInfos(ob.ob):
            print(f"// @pyparm |{ai.name}|{ai.default}|{ai.short_desc}", file=fp)

    for ob in classes:
        # only classes with docstrings get printed.
        if not ob.ob.__doc__:
            continue
        ob_name = mod_name + "." + ob.name
        print(f"// @object {ob_name}|{format_desc(ob.desc)}", file=fp)
        func_infos = []
        # We need to iter the keys then to a getattr() so the funky descriptor
        # things work.
        for n in ob.ob.__dict__:
            o = getattr(ob.ob, n)
            if isinstance(o, (types.FunctionType, types.MethodType)):
                info = BuildInfo(n, o)
                if should_build_function(info):
                    func_infos.append(info)
        for fi in func_infos:
            print(f"// @pymeth {fi.name}|{fi.short_desc}", file=fp)
        for fi in func_infos:
            print(
                f"// @pymethod |{ob_name}|{fi.name}|{format_desc(fi.desc)}",
                file=fp,
            )
            if hasattr(fi.ob, "im_self") and fi.ob.im_self is ob.ob:
                print("// @comm This is a @classmethod.", file=fp)
            print(
                f"// @pymethod |{ob_name}|{fi.name}|{format_desc(fi.desc)}",
                file=fp,
            )
            for ai in BuildArgInfos(fi.ob):
                print(
                    f"// @pyparm |{ai.name}|{ai.default}|{ai.short_desc}",
                    file=fp,
                )

    for name, val in constants:
        desc = f"{name} = {val!r}"
        if isinstance(val, int):
            desc += f" (0x{val:x})"
        print(f"// @const {mod_name}|{name}|{desc}", file=fp)


def main(fp, args):
    print("// @doc", file=fp)
    for arg in args:
        build_module(sys.stdout, arg)


if __name__ == "__main__":
    main(sys.stdout, sys.argv[1:])
