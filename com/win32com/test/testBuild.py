# Test win32com.client.build's code generation helpers.

import decimal
import os
import tempfile
import unittest

import pythoncom
from win32com.client import build, genpy

DEFAULT_FLAGS = (
    pythoncom.PARAMFLAG_FIN | pythoncom.PARAMFLAG_FOPT | pythoncom.PARAMFLAG_FHASDEFAULT
)


def MakeArgDesc(value, vt=pythoncom.VT_CY):
    # shape of a FUNCDESC entry for a single optional param with a default
    return (vt, DEFAULT_FLAGS, value)


def MakeFuncDesc(value, vt=pythoncom.VT_CY):
    return (1, 0, [MakeArgDesc(value, vt)], 0, 0, 0, 1)


class FakeTypeLib:
    def GetLibAttr(self):
        return (0,) * 20

    def GetDocumentation(self, index, *args):
        return ("", "", 0, "")


class TestDecimalDefaultArgs(unittest.TestCase):
    def test_make_default_arg_repr(self):
        for vt in (pythoncom.VT_CY, pythoncom.VT_DECIMAL):
            self.assertEqual(
                build.MakeDefaultArgRepr(MakeArgDesc(decimal.Decimal("1.5"), vt)),
                "decimal.Decimal('1.5')",
            )

    def test_generated_method_declares_default(self):
        params = build.BuildCallList(
            MakeFuncDesc(decimal.Decimal("1.5")),
            ["Pay", "Amount"],
            "defaultNamedOptArg",
            "defaultNamedNotOptArg",
            "defaultUnnamedArg",
            "pythoncom.Missing",
        )
        self.assertIn("Amount=decimal.Decimal('1.5')", params)

    def test_generated_method_evaluates_default(self):
        params = build.BuildCallList(
            MakeFuncDesc(decimal.Decimal("1.5")),
            ["Pay", "Amount"],
            "defaultNamedOptArg",
            "defaultNamedNotOptArg",
            "defaultUnnamedArg",
            "pythoncom.Missing",
        )
        namespace = {"decimal": decimal, "pythoncom": pythoncom}
        exec(f"def Pay(self{params}):\n    pass", namespace)
        self.assertEqual(namespace["Pay"].__defaults__, (decimal.Decimal("1.5"),))

    def test_dynamic_dispatch_namespace_has_decimal(self):
        from win32com.client import dynamic

        self.assertIs(dynamic.decimal, decimal)

    def test_generated_module_header_imports_decimal(self):
        generator = genpy.Generator(FakeTypeLib(), None, None)
        fd, path = tempfile.mkstemp(suffix=".py")
        os.close(fd)
        try:
            with open(path, "w", encoding="utf-8") as generator.file:
                generator.do_gen_file_header()
            with open(path, encoding="utf-8") as generated:
                self.assertIn("\nimport decimal\n", generated.read())
        finally:
            os.remove(path)
