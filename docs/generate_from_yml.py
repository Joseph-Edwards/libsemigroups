#!/usr/bin/env python3
"""
This module generates the documentation pages in docs/source/_generated from
the yml files in docs/yml.
"""

import itertools
import os
import re
import sys
from datetime import datetime
from functools import lru_cache
from os.path import isfile

import bs4
import yaml
from bs4 import BeautifulSoup

# Function names follow the pattern outputtype_something_inputtype, so
# e.g. rst_something_yml

########################################################################
# Global variables
########################################################################

_COPYRIGHT_NOTICE = """.. Copyright (c) 2019-22, J. D. Mitchell

   Distributed under the terms of the GPL license version 3.

   The full license is in the file LICENSE, distributed with this software.

   This file was auto-generated by docs/generate_from_yml.py, do not edit.
"""

__REWRITES_ATTEMPTED = 0
__REWRITES_ACTUAL = 0
__WARNINGS = 0

__DOXY_DICT = {}
__RST_FILES_WRITTEN = {}

########################################################################
# Internal functions
########################################################################


def __accepts(*types):
    def check_accepts(f):
        assert len(types) == f.__code__.co_argcount

        def new_f(*args, **kwds):
            for (a, t) in zip(args, types):
                assert isinstance(a, t), "arg %r does not match %s" % (a, t)
            return f(*args, **kwds)

        new_f.__name__ = f.__name__
        return new_f

    return check_accepts


@__accepts(str, str)
def __warn(fname, msg):
    global __WARNINGS
    __WARNINGS += 1
    sys.stderr.write(
        "\033[38;5;208mWARNING in {}: {}\033[0m\n".format("docs/" + fname, msg)
    )


@__accepts(str, (str, None))
def __info(msg, fname=None):
    assert isinstance(msg, str)
    if fname is not None:
        sys.stdout.write("%s: %s\n" % (fname, msg))
    else:
        sys.stdout.write("%s\n" % msg)


@__accepts(float)
def __time_since_epoch_to_human(n):
    try:
        return datetime.fromtimestamp(n).strftime("%Y-%m-%d %H:%M:%S.%f")
    except:
        return "N/A"


@__accepts(str, str)
def __write_file_if_changed(fname, contents):
    global __REWRITES_ATTEMPTED, __REWRITES_ACTUAL
    __REWRITES_ATTEMPTED += 1
    __RST_FILES_WRITTEN[fname] = True
    if os.path.exists(fname) and os.path.isfile(fname):
        with open(fname, "r") as f:
            file_contents = f.read()
            if file_contents == contents:
                return
    with open(fname, "w") as f:
        __info("Rewriting docs/%s ..." % fname)
        __REWRITES_ACTUAL += 1
        f.write(contents)


def __summary():
    global __REWRITES_ACTUAL, __REWRITES_ATTEMPTED, __WARNINGS
    __info(
        "Summary: %d / %d files rewritten"
        % (__REWRITES_ACTUAL, __REWRITES_ATTEMPTED)
        + " and %d warnings!!" % __WARNINGS
    )


def __clean_up():
    for fname in os.listdir("source/_generated"):
        fname = os.path.join("source/_generated", fname)
        if fname not in __RST_FILES_WRITTEN:
            __info(fname, "deleting!!!")
            os.remove(fname)


########################################################################
# Filename functions
########################################################################


@__accepts(str)
def filename_from_cppname(name):
    p = re.compile(r"operator\s*\*")
    name = p.sub("operator_star", name)
    p = re.compile(r"operator!=")
    name = p.sub("operator_not_eq", name)
    p = re.compile(r"operator\(\)")
    name = p.sub("call_operator", name)
    p = re.compile(r"operator<$")
    name = p.sub("operator_less", name)
    p = re.compile(r"operator\<\<")
    name = p.sub("insertion_operator", name)
    p = re.compile(r"operator==")
    name = p.sub("operator_equal_to", name)
    p = re.compile(r"operator\>")
    name = p.sub("operator_greater", name)
    p = re.compile(r"[\W]")
    name = p.sub("_", name)
    return name.lower()


@__accepts(str)
def overview_filename(name):
    return os.path.join(
        "source/_generated", filename_from_cppname(name) + ".rst"
    )


@__accepts(str, str)
def subpage_filename(class_, name):
    return os.path.join(
        "source/_generated",
        filename_from_cppname(class_ + "::" + name) + ".rst",
    )


########################################################################
# yml manipulation
########################################################################


@__accepts(str)
def extract_yml_func_signature(yml_entry):
    yml_entry = yml_entry.strip()
    lpos = yml_entry.find("(")
    if lpos == -1:
        return yml_entry, None
    rpos = yml_entry.rfind(")")
    params = (
        "("
        + doxy_normalize_yml_params(yml_entry[lpos + 1 : rpos])
        + yml_entry[rpos:]
    )
    if yml_entry.startswith("template"):
        return (yml_entry[yml_entry.rfind(" ", 0, lpos) + 1 : lpos], params)
    return yml_entry[:lpos], params


# TODO accepts
# Check that there isn't any doc in the doxygen output that's not in a yml-file
def compare_yml_to_doxy(ymlfname, ymldic):
    class_ = next(iter(ymldic))
    try:
        doxy_xml(class_)  # to ensure that __DOXY_DICT is populated
    except:
        return

    yml = next(iter(ymldic.values()))
    if yml is None:
        yml = {}
    else:
        yml = [next(iter(x.values())) for x in yml]
        yml = itertools.chain.from_iterable(yml)
        yml = [x for x in yml if isinstance(x, str)]
        yml = [extract_yml_func_signature(x) for x in yml]
        yml = [x if x[1] is not None else (x[0]) for x in yml]
        yml = ["".join(x) for x in yml]
        yml = {class_ + "::" + x: True for x in yml}
    yml[class_] = True

    dict_keys = type({}.keys())

    doxy = [
        (k, v)
        for k, v in doxy_dict().items()
        if k.startswith(class_ + "::" or k == class_)
    ]
    doxy = [(k, v.keys()) if isinstance(v, dict) else (k, v) for k, v in doxy]
    doxy = [
        ["".join([k, x]) for x in v] if isinstance(v, dict_keys) else (k, None)
        for k, v in doxy
    ]
    doxy = itertools.chain.from_iterable(doxy)
    doxy = [x for x in doxy if x is not None]
    destructor = "~" + class_.split("::")[-1]
    doxy = [x for x in doxy if not destructor in x]
    doxy = [x for x in doxy if not "::::" in x]
    doxy = [x for x in doxy if not x.endswith("= 0")]
    for x in doxy:
        if not x in yml:
            __warn(
                ymlfname,
                'missing doc, found "%s" in doxygen output but not in yml file'
                % x,
            )


########################################################################
# String manipulation
########################################################################


@__accepts(str)
def strip_libsemigroups_prefix(name):
    if len(name) == 0:
        return name
    s = name.split("::")
    if s[0] == "libsemigroups":
        return "::".join(s[1:])
    else:
        return name


@__accepts(str)
def unqualified_name(name):
    if len(name) == 0:
        return name
    return name.split("::")[-1]


########################################################################
# Doxygen functions
########################################################################


def doxy_dict():
    return __DOXY_DICT


def doxy_run():
    if not os.path.exists("build/xml"):
        __info("The folder docs/build/xml does not exist!")
        return True
    last_changed_source = [0, ""]
    for f in os.listdir("../include/libsemigroups"):
        if not f.startswith("."):
            f = os.path.join("../include/libsemigroups", f)
            if os.path.isfile(f) and f.endswith(".hpp"):
                if os.path.getmtime(f) > last_changed_source[0]:
                    last_changed_source = [os.path.getmtime(f), f]

    __info(
        "The last changed header file is:\t"
        + last_changed_source[1]
        + "\nLast modified:\t\t\t\t"
        + __time_since_epoch_to_human(last_changed_source[0])
    )
    last_changed_source = last_changed_source[0]
    first_built_file = [float("inf"), ""]
    for root, dirs, files in os.walk("build/xml"):
        for f in files:
            f = os.path.join(root, f)
            if os.path.getmtime(f) < first_built_file[0]:
                first_built_file = [os.path.getmtime(f), f]
    __info(
        "The first built xml file is:\t\t"
        + first_built_file[1]
        + "\nlast modified:\t\t\t\t"
        + __time_since_epoch_to_human(first_built_file[0])
    )
    first_built_file = first_built_file[0]
    if __time_since_epoch_to_human(first_built_file) == "N/A":
        return True
    return first_built_file < last_changed_source


@lru_cache(maxsize=None)
@__accepts(str)
def doxy_filename(name):
    name = re.sub("_", "__", name)
    # TODO use re.sub
    p = re.compile(r"::")
    name = p.sub("_1_1", name)
    p = re.compile(r"([A-Z])")
    name = p.sub(r"_\1", name).lower()
    if isfile("build/xml/class" + name + ".xml"):
        return "build/xml/class" + name + ".xml"
    else:
        return "build/xml/struct" + name + ".xml"


@lru_cache(maxsize=None)
@__accepts(str, str, (str, type(None)))
def doxy_warn(fname, name, params=None):
    params = "" if params is None else params
    __warn(fname, "no doxygen output found for %s%s" % (name, params))


@lru_cache(maxsize=None)
@__accepts(str, (str, type(None)))
def doxy_xml(name, params=None):
    global __DOXY_DICT
    if not name in __DOXY_DICT:
        class_ = name
        pos = class_.rfind("::")
        while pos != -1 and not isfile(doxy_filename(class_)):
            class_, pos = class_[:pos], class_.rfind("::")
        if isfile(doxy_filename(class_)) and not class_ in __DOXY_DICT:
            xml = BeautifulSoup(open(doxy_filename(class_), "r"), "xml")
            __DOXY_DICT[class_] = xml.find("compounddef")
            for x in xml.find_all("memberdef"):
                if "prot" in x.attrs and x.attrs["prot"] != "public":
                    continue
                mem_def_name = class_ + "::" + x.find("name").text
                if (
                    "kind" in x.attrs
                    and x.attrs["kind"] != "function"
                    and x.attrs["kind"] != "friend"
                ):
                    assert mem_def_name not in __DOXY_DICT, "unexpected key!!"
                    __DOXY_DICT[mem_def_name] = x
                    continue
                tparam = x.find("templateparamlist")
                if tparam is not None:
                    tparam = tparam.find_all("param")
                    tparam = [x.find("type").text.strip() for x in tparam]
                param = x.find_all("param")
                param = [x.find("type").text.strip() for x in param]
                if tparam is not None:
                    param = [x for x in param if x not in tparam]
                param = "(" + ",".join(param) + ")"
                if "const" in x.attrs and x.attrs["const"] == "yes":
                    param += " const"
                elif x.argsstring.text.endswith(" const"):
                    param += " const"
                if "noexcept" in x.attrs and x.attrs["noexcept"] == "yes":
                    param += " noexcept"
                if x.argsstring.text.endswith("=default"):
                    param += " = default"
                if x.argsstring.text.endswith("=delete"):
                    param += " = delete"
                if x.argsstring.text.endswith(" override"):
                    param += " override"
                if x.argsstring.text.endswith("=0"):
                    param += " = 0"
                if not mem_def_name in __DOXY_DICT:
                    __DOXY_DICT[mem_def_name] = {}
                # Turns out that somethings are duplicated in the doxygen
                # output, so no checks if param already belongs in the dict
                # for example, FroidurePin::minimal_factorisation(word_type
                # &amp;word, element_index_type pos) const)
                __DOXY_DICT[mem_def_name][param] = x

        elif not class_ in __DOXY_DICT:
            for fname in os.listdir("build/xml"):
                if not fname.startswith("namespace"):
                    continue
                xml = BeautifulSoup(open("build/xml/" + fname, "r"), "xml")
                ns = xml.find("compoundname").text
                for x in xml.find_all("memberdef"):
                    y = x.find("name").text
                    y = ns + "::" + y
                    if y not in __DOXY_DICT:
                        __DOXY_DICT[y] = x
    if params is not None:
        return __DOXY_DICT[name][params]
    else:
        return __DOXY_DICT[name]


@lru_cache(maxsize=None)
@__accepts(str, str, (str, type(None)))
def doxy_brief(ymlfname, name, params=None):
    try:
        xml = doxy_xml(name, params)
        return convert_to_rst(
            next(x for x in xml if x.name == "briefdescription")
        )
    except:
        doxy_warn(ymlfname, name, params)
        return None


@lru_cache(maxsize=None)
@__accepts(str, str, (str, type(None)))
def doxy_kind(ymlfname, name, params=None):
    xml = doxy_xml(name, params)
    if isinstance(xml, bs4.element.Tag) and "kind" in xml.attrs:
        result = xml.attrs["kind"]
        return result if result != "friend" else "function"
    else:
        __warn(ymlfname, "could not determine the kind of " + name)
        raise Exception("could not determine the kind of " + name)


@lru_cache(maxsize=None)
@__accepts(str, str, (str, type(None)))
def doxy_is_inherited(ymlfname, name, params=None):
    xml = doxy_xml(name, params)
    if isinstance(xml, bs4.element.Tag) and xml.find("definition") is not None:
        defn = xml.find("definition").text.split(" ")
        defn = defn[0] if len(defn) == 1 else defn[1]
        class_ = "::".join(name.split("::")[:-1]) + "::"
        return not defn.startswith(class_)
    else:
        __warn(ymlfname, "could not determine if %s is inherited" % name)
        raise Exception("could not determine if %s is inherited" % name)


@lru_cache(maxsize=None)
@__accepts(str, str, (str, type(None)))
def doxy_is_typedef(ymlfname, name, params=None):
    xml = doxy_xml(name, params)
    if isinstance(xml, bs4.element.Tag):
        return "kind" in xml.attrs and xml["kind"] == "typedef"
    else:
        __warn(ymlfname, "could not determine if %s is a typedef" % name)
        raise Exception("could not determine if %s is a typedef" % name)


@lru_cache(maxsize=None)
@__accepts(bs4.element.Tag)
def xml_is_deprecated(xml):
    if xml.find("definition") is not None:
        defn = xml.find("definition").text
        return defn.startswith("LIBSEMIGROUPS_DEPRECATED")
    return False


@lru_cache(maxsize=None)
@__accepts(str)
def doxy_normalize_yml_params(params):
    params = params.strip()
    # replace more than 1 space by a single space
    params = re.sub(r"\s{2,}", " ", params)
    # Add space after < if it's a non-space
    params = re.sub(r"(?<=[<])(?=[^\s])", " ", params)
    # Add space before > if it's a non-space
    params = re.sub(r"(?<=[^\s])(?=[>])", " ", params)
    # Add space before & if it's a non-space and not &
    params = re.sub(r"(?<=[^\s\&])(?=[\&])", " ", params)
    # Add space after & if it's a non-space and not &
    params = re.sub(r"(?<=[\&])(?=[^\s\&])", " ", params)
    # remove whitespace around commas
    params = re.sub(r"\s*,\s*", ",", params)

    # remove some of the spaces introduced above if for example the parameters
    # are: std::function<void(bool&)>, then doxygen does not want the spaces at
    # the end in "& ) >" for some reason
    if params.endswith("& ) >"):
        params = re.sub(r"\& \) >$", "&)>", params)
    return params


@lru_cache(maxsize=None)
@__accepts(str, str, (str, type(None)))
def doxy_returns(ymlfname, name, params=None):
    xml = doxy_xml(name, params)
    if doxy_is_typedef(ymlfname, name, params):
        return ""
    try:
        return next(x for x in xml if x.name == "type").text
    except StopIteration:
        return ""


@lru_cache(maxsize=None)
@__accepts(str, str, (str, type(None)))
def doxy_tparams(ymlfname, name, params=None):
    try:
        xml = doxy_xml(name, params)
        if isinstance(xml, dict):
            raise KeyError
    except KeyError:
        # TODO roll this try-except into doxy_xml
        doxy_warn(ymlfname, name, params)
        return ""

    xml = xml.find("templateparamlist")
    if xml is None:
        return ""
    tparams = []
    for x in xml.find_all("param"):
        y = x.type.text
        if x.declname is not None:
            y += " " + x.declname.text
        if x.defval is not None:
            y += " = " + x.defval.text
        tparams.append(y)

    return "template <%s> %s " % (
        ", ".join(tparams),
        doxy_returns(ymlfname, name, params),
    )


# TODO this is way too general for what it's used for here
@__accepts(bs4.element.Tag, list)
def convert_to_rst(xml, context=[]):
    context.append(xml.name)
    if "kind" in xml.attrs and xml.attrs["kind"] == "enum":
        context.append(xml.attrs["kind"])

    def indent(context):
        n = context.count("memberdef")
        n += context.count("compounddef")
        n += context.count("parameterdescription")
        n += context.count("programlisting")
        return " " * (3 * n)

    result = ""
    if xml.name == "compounddef":
        result += PREFIXES[xml.attrs["kind"]]
        try:
            t = next((x for x in xml if x.name == "templateparamlist"))
            xml = [t] + [x for x in xml if x.name != "templateparamlist"]
        except StopIteration:
            pass
    elif xml.name == "memberdef":
        result += PREFIXES[xml.attrs["kind"]]
    if (
        not isinstance(xml, list)
        and "kind" in xml.attrs
        and xml.attrs["kind"] == "enum"
    ):
        n = next((x for x in xml if x.name == "name"))
        bd = ""
        try:
            bd = next((x for x in xml if x.name == "briefdescription"))
        except StopIteration:
            pass
        xml = [n, bd] + [
            x for x in xml if x.name != "briefdescription" and x.name != "name"
        ]

    for x in xml:
        if isinstance(x, str):
            x = x.strip()
            result += " " if x != "." and x != "" and not x[0].isupper() else ""
            result += x
        elif "enum" in context and x.name == "name":
            result += x.text.strip()
        elif "enum" in context and x.name == "enumvalue":
            result += "\n\n" + indent(context) + ".. cpp:enumerator:: "
            result += convert_to_rst(x, context)
        # elif context and x.name == "initializer":
        #    result += x.text.strip()
        elif x.name == "definition":
            if x.text.startswith("using"):
                lhs, rhs = x.text.split("=")
                lhs = lhs[lhs.rfind("::") + 2 :].strip()
                if rhs.find("detail::") == -1:
                    rhs = re.sub("typename", "", rhs).strip()
                    rhs = re.sub("typedef", "", rhs).strip()
                    result += lhs + " = " + rhs
                else:
                    lhs = re.sub("using", "", lhs).strip()
                    result += lhs
            else:
                y = x.text.split("::")
                if y[-1] == "operator=":
                    # assignment constructor
                    result += y[0][: y[0].find("&") + 1]
                elif not y[-2].startswith(y[-1]):
                    # not constructor
                    return_type = []
                    for z in y:
                        return_type.append(z)
                        if z.endswith("libsemigroups"):
                            break
                    return_type = "::".join(return_type)
                    result += return_type[: return_type.rfind(" ") + 1]
                result += y[-1]  # unqualified name
        elif x.name == "argsstring":
            result += x.text
        elif x.name == "briefdescription":
            result += "\n\n" + indent(context) + convert_to_rst(x, context)
        elif x.name == "detaileddescription":
            result += "\n" + indent(context) + convert_to_rst(x, context)
        elif x.name == "templateparamlist":
            params = []
            for y in x.find_all("param"):
                z = y.type.text
                if y.declname is not None:
                    z += " " + y.declname.text
                params.append(z)
            result += "template <" + ", ".join(params) + ">"
        elif x.name == "computeroutput":
            if len(x.text) != 0:
                result += " ``" + x.text + "``"
        elif x.name == "formula":
            result += " :math:`" + x.text.replace("$", "") + "`"
        elif x.name == "title":
            result += "\n\n%s:%s: " % (indent(context), x.text.lower())
        elif x.name == "para":
            result += convert_to_rst(x, context)
            if len(context) > 0 and (
                context[-1] == "detaileddescription"
                or context[-1] == "briefdescription"
            ):
                result += "\n\n" + indent(context)
        elif x.name == "simplesect" and x.attrs["kind"] == "return":
            result += (
                "\n\n"
                + indent(context)
                + ":returns: "
                + convert_to_rst(x, context)
            )
        elif x.name == "simplesect" and x.attrs["kind"] == "par":
            result += convert_to_rst(x, context)
        elif x.name == "parameterlist" and x.attrs["kind"] == "templateparam":
            for y in x.find_all("parameteritem"):
                result += "\n\n" + indent(context)
                result += ":tparam %s: %s" % (
                    y.find("parametername").text,
                    convert_to_rst(y.find("parameterdescription"), context),
                )
        elif x.name == "parameterlist" and x.attrs["kind"] == "param":
            for y in x.find_all("parameteritem"):
                result += "\n\n" + indent(context)
                result += ":param %s: %s" % (
                    y.find("parametername").text,
                    convert_to_rst(x.find("parameterdescription"), context),
                )
        elif x.name == "parameterlist" and x.attrs["kind"] == "exception":
            for y in x.find_all("parameteritem"):
                result += "\n\n" + indent(context)
                result += ":throws:\n" + indent(context) + " " * 3
                result += convert_to_rst(y.find("parametername"), context)
                result += convert_to_rst(
                    y.find("parameterdescription"), context
                )
        elif x.name == "simplesect" and x.attrs["kind"] == "see":
            result += (
                "\n\n"
                + indent(context)
                + ".. seealso:: "
                + convert_to_rst(x, context)
            )
        elif x.name == "ref":
            if "kindref" in x.attrs and x.attrs["kindref"] == "member":
                kindref = "member"
            else:
                kindref = "any"
            result += " :cpp:%s:`%s` " % (kindref, x.text)
        elif x.name == "emphasis":
            result += " *%s*" % x.text
        elif x.name == "bold":
            result += "\n\n" + indent(context) + "**%s**" % x.text
        elif x.name == "compoundname":
            result += x.text[x.text.rfind("::") + 2 :]
        elif x.name == "ulink":
            result += " `%s <%s>`_" % (x.text, x.attrs["url"])
        elif x.name == "itemizedlist":
            result += "\n" + convert_to_rst(x, context)
        elif x.name == "listitem":
            result += "\n" + indent(context) + "* " + convert_to_rst(x, context)
        elif x.name == "programlisting":
            result += (
                "\n\n"
                + indent(context)
                + ".. code-block::\n"
                + convert_to_rst(x)
            )
        elif x.name == "codeline":
            result += "\n" + indent(context) + convert_to_rst(x)
        elif x.name == "highlight":
            result += convert_to_rst(x)
        elif x.name == "sp":
            result += " "

    if len(context) > 0 and context[-1] == "enum":
        context.pop()
    if context.pop() == "itemizedlist":
        result += "\n\n" + indent(context)

    return result


########################################################################
# Rst functions
########################################################################


@__accepts(str, str)
def rst_section(name, sym="="):
    underline = sym * len(name)
    return "\n" + name + "\n" + underline + "\n"


@__accepts(str, str, (str, None))
def rst_doxy(kind, class_, yml_entry=None):
    if yml_entry is not None:
        return """\n.. doxygen%s:: %s::%s
   :project: libsemigroups\n""" % (
            kind,
            class_,
            yml_entry,
        )
    else:
        return """\n.. doxygen%s:: %s
   :project: libsemigroups\n""" % (
            kind,
            class_,
        )


@__accepts(str)
def rst_generate_overview(ymlfname):
    with open(ymlfname, "r") as f:
        out = _COPYRIGHT_NOTICE
        ymldic = yaml.load(f, Loader=yaml.FullLoader)
        compare_yml_to_doxy(ymlfname, ymldic)
        name = next(iter(ymldic))  # object name
        out += rst_section(strip_libsemigroups_prefix(name))
        try:
            out += rst_doxy(doxy_kind(ymlfname, name), name)
        except:
            __warn(ymlfname, "no doxygen output found for " + name)
            return
        out += "\n.. cpp:namespace:: %s\n\n" % name
        toc = "\n.. toctree::\n   :hidden:\n"
        if ymldic[name] is not None:
            for sectdic in ymldic[name]:
                subname = next(iter(sectdic))
                out += rst_section(subname, "-")
                fnam = subpage_filename(name, subname)
                toc += "\n   " + fnam[fnam.rfind("/") + 1 :]
                if sectdic[subname] is not None:
                    out += ".. list-table::\n"
                    out += "   :widths: 50 50\n"
                    out += "   :header-rows: 0\n\n"
                    things = sectdic[subname]
                    if isinstance(things[0], list):
                        things = things[1:]
                    for thing in sorted(things):
                        thing_name, thing_params = extract_yml_func_signature(
                            thing
                        )
                        title = ""
                        if thing == unqualified_name(name) + "()":
                            #  Special case for the default constructor
                            title = thing
                            thing = "%s::%s" % (
                                thing_name,
                                thing,
                            )
                        thing_name = name + "::" + thing_name
                        tparams = doxy_tparams(
                            ymlfname, thing_name, thing_params
                        )
                        if tparams != "":
                            # Escape first template < !
                            title = re.sub("<", r"\<", thing, 1)

                        if title != "":
                            # Use different title and link text
                            out += (
                                "   * - :cpp:member:`%s <%s%s>`\n     - %s\n"
                                % (
                                    title,
                                    tparams,
                                    thing,
                                    doxy_brief(
                                        ymlfname, thing_name, thing_params
                                    ),
                                )
                            )
                        else:
                            out += "   * - :cpp:member:`%s`\n     - %s\n" % (
                                thing,
                                doxy_brief(ymlfname, thing_name, thing_params),
                            )

            out += toc + "\n"
        __write_file_if_changed(overview_filename(name), out)


@__accepts(str)
def rst_generate_subpages(ymlfname):
    with open(ymlfname, "r") as f:
        ymldic = yaml.load(f, Loader=yaml.FullLoader)
        name = next(iter(ymldic))
        if ymldic[name] is None:
            return
        for sectiondic in ymldic[name]:
            subname = next(iter(sectiondic))
            if sectiondic[subname] is None:
                continue
            rstfname = subpage_filename(name, subname)
            out = _COPYRIGHT_NOTICE + rst_section(subname)
            out += ".. cpp:namespace:: libsemigroups\n\n"
            things = sectiondic[subname]
            if isinstance(things[0], list):
                assert (
                    len(things[0]) == 1
                ), "expected the length of the first entry to be 1"
                assert isinstance(
                    things[0][0], str
                ), "expected the first entry to be a string"
                out += things[0][0] + "\n\n"
                things = things[1:]
            out += ".. cpp:namespace-pop::\n\n"
            for thing in sorted(things):
                try:
                    (
                        thing_name,
                        thing_params,
                    ) = extract_yml_func_signature(thing)

                    if thing_params == "(bool(*)())":
                        # TODO improve this
                        thing = thing_name + "(bool (*func)())"
                    thing_name = name + "::" + thing_name
                    out += rst_doxy(
                        doxy_kind(ymlfname, thing_name, thing_params),
                        name,
                        thing,
                    )
                except:
                    doxy_warn(ymlfname, thing_name, thing_params)
            __write_file_if_changed(rstfname, out)


########################################################################
# The main function
########################################################################


def main():
    if sys.version_info[0] < 3:
        raise Exception("Python 3 is required")
    if doxy_run():
        sys.stdout.write("\033[2m")
        sys.stderr.write("\033[2m")
        __info("Running doxygen!")
        os.system("doxygen")
        os.system("touch build/xml/*.xml")
        sys.stderr.write("\033[0m")
        sys.stdout.write("\033[0m")
    else:
        __info("Not running doxygen!")
    __info("Generating sphinx rst files from docs/yml . . .")
    try:
        os.mkdir("source/_generated")
    except FileExistsError:
        pass
    for fname in sorted(os.listdir("yml")):
        if fname[0] != ".":
            __info("Processing %s . . ." % fname)
            fname = os.path.join("yml", fname)
            rst_generate_overview(fname)
            rst_generate_subpages(fname)
    __clean_up()
    __summary()


if __name__ == "__main__":
    main()
