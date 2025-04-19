from __future__ import annotations

import re
import sys
from collections.abc import Iterable
from types import FunctionType, MethodType
from typing import TYPE_CHECKING, Generator, Generic, TypeVar, Union

if TYPE_CHECKING:
    from _typeshed import SupportsWrite

_T = TypeVar("_T")


def ad_escape(s: str) -> str:
    return re.sub(r"([^<]*)<([^>]*)>", r"\g<1>\\<\g<2>\\>", s)


class DocInfo(Generic[_T]):
    def __init__(self, name: str, ob: _T) -> None:
        docstring = (ob.__doc__ or "").strip()

        self.desc = docstring
        self.short_desc = docstring and docstring.splitlines()[0]
        self.name = name
        self.ob = ob


class ArgInfo(DocInfo[Union[FunctionType, MethodType]]):
    def __init__(self, name: str, ob: FunctionType | MethodType, default: str) -> None:
        super().__init__(name, ob)
        self.desc = name
        self.short_desc = name
        self.default = default


def BuildArgInfos(ob: FunctionType | MethodType) -> list[ArgInfo]:
    ret: list[ArgInfo] = []
    # Reversed for easier default checking.
    # Since arguments w/ default can only be at the end of a function.
    vars = reversed(ob.__code__.co_varnames[: ob.__code__.co_argcount])
    defs = list(ob.__defaults__ or [])
    for n in vars:
        default = ""
        if defs:
            default = repr(defs.pop())
            # the default may be an object, so the repr gives '<...>'
            # and the angle brackets screw AutoDuck.
            default = default.replace("<", "").replace(">", "")
        ret.append(ArgInfo(n, ob, default))
    ret.reverse()
    return ret


def should_build_function(fn: FunctionType | MethodType) -> bool:
    return bool(fn.__doc__) and not fn.__name__.startswith("_")


# docstring aware paragraph generator.
# Isn't there something in docutils we can use?
def gen_paras(val: str) -> Generator[list[str], None, None]:
    chunks: list[str] = []
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


def format_desc(desc: str) -> str:
    # A little complicated!  Given the docstring for a module, we want to:
    # write:
    # 'first_para_of_docstring'
    # '@comm next para of docstring'
    # '@comm next para of docstring' ... etc
    # BUT - also handling embedded doctests, where we write
    # '@iex >>> etc.'
    if not desc:
        return ""
    paragraphs = gen_paras(desc)
    first_paragraph = next(paragraphs)
    chunks = [first_paragraph[0], *[f"// {line}" for line in first_paragraph[1:]]]
    for lines in paragraphs:
        first = lines[0]
        if first.startswith(">>> "):
            prefix = "// @iex \n// "
        else:
            prefix = "\n// @comm "
        chunks.append(prefix + first)
        chunks.extend(["// " + l for l in lines[1:]])
    return "\n".join(chunks)


def build_module(mod_name: str) -> None:
    __import__(mod_name)
    mod = sys.modules[mod_name]
    functions: list[DocInfo[FunctionType]] = []
    classes: list[DocInfo[type]] = []
    constants: list[tuple[str, int | str]] = []
    for name, ob in mod.__dict__.items():
        if name.startswith("_"):
            continue
        if hasattr(ob, "__module__") and ob.__module__ != mod_name:
            continue
        if type(ob) == type:
            classes.append(DocInfo(name, ob))
        elif isinstance(ob, FunctionType):
            if should_build_function(ob):
                functions.append(DocInfo(name, ob))
        elif name.upper() == name and isinstance(ob, (int, str)):
            constants.append((name, ob))
    module_info = DocInfo(mod_name, mod)
    print(f"// @module {mod_name}|{format_desc(module_info.desc)}")
    for ob in functions:
        print(f"// @pymeth {ob.name}|{ob.short_desc}")
    for ob in classes:
        # only classes with docstrings get printed.
        if not ob.ob.__doc__:
            continue
        ob_name = mod_name + "." + ob.name
        print(f"// @pyclass {ob.name}|{ob.short_desc}")
    for ob in functions:
        print(
            f"// @pymethod |{mod_name}|{ob.name}|{format_desc(ob.desc)}",
        )
        for ai in BuildArgInfos(ob.ob):
            print(f"// @pyparm |{ai.name}|{ai.default}|{ai.short_desc}")

    for ob in classes:
        # only classes with docstrings get printed.
        if not ob.ob.__doc__:
            continue
        ob_name = mod_name + "." + ob.name
        print(f"// @object {ob_name}|{format_desc(ob.desc)}")
        func_infos: list[DocInfo[FunctionType | MethodType]] = []
        # We need to iter the keys then to a getattr() so the funky descriptor
        # things work.
        for n in ob.ob.__dict__:
            o = getattr(ob.ob, n)
            if isinstance(o, (FunctionType, MethodType)):
                if should_build_function(o):
                    func_infos.append(DocInfo(n, o))
        for fi in func_infos:
            print(f"// @pymeth {fi.name}|{fi.short_desc}")
        for fi in func_infos:
            print(f"// @pymethod |{ob_name}|{fi.name}|{format_desc(fi.desc)}")
            if hasattr(fi.ob, "im_self") and fi.ob.im_self is ob.ob:
                print("// @comm This is a @classmethod.")
            print(f"// @pymethod |{ob_name}|{fi.name}|{format_desc(fi.desc)}")
            for ai in BuildArgInfos(fi.ob):
                print(f"// @pyparm |{ai.name}|{ai.default}|{ai.short_desc}")

    for name, val in constants:
        desc = f"{name} = {val!r}"
        if isinstance(val, int):
            desc += f" (0x{val:x})"
        print(f"// @const {mod_name}|{name}|{desc}")


def main(args: Iterable[str]) -> None:
    print("// @doc")
    for arg in args:
        build_module(arg)


if __name__ == "__main__":
    main(sys.argv[1:])
