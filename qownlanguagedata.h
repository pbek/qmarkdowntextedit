#include <QMultiHash>

/* ------------------------
 * TEMPLATE FOR LANG DATA
 * -------------------------
 *
 * loadXXXData, where XXX is the language
 * keywords are the language keywords e.g, const
 * types are built-in types i.e, int, char, var
 * literals are words like, true false
 * builtin are the library functions
 * other can contain any other thing, for e.g, in cpp it contains the preprocessor

void loadXXXData(QMultiHash<QChar, QString> &types,
             QMultiHash<QChar, QString> &keywords,
             QMultiHash<QChar, QString> &literals,
             QMultiHash<QChar, QString> &builtin,
             QMultiHash<QChar, QString> &other){
    keywords = {
    };

    types = {
    };

    literals = {
    };

    builtin = {
    };

    other = {
    };
}

*/

void loadCppData(QMultiHash<QChar, QString> &types,
             QMultiHash<QChar, QString> &keywords,
             QMultiHash<QChar, QString> &literals,
             QMultiHash<QChar, QString> &builtin,
             QMultiHash<QChar, QString> &other){
    keywords = {
        {QLatin1Char('a'), QLatin1String("asm")},
        {QLatin1Char('a'), QLatin1String("auto")},
        {QLatin1Char('b'), QLatin1String("break")},
        {QLatin1Char('c'), QLatin1String("case")},
        {QLatin1Char('c'), QLatin1String("catch")},
        {QLatin1Char('c'), QLatin1String("const")},
        {QLatin1Char('c'), QLatin1String("const_cast")},
        {QLatin1Char('c'), QLatin1String("continue")},
        {QLatin1Char('d'), QLatin1String("default")},
        {QLatin1Char('d'), QLatin1String("delete")},
        {QLatin1Char('d'), QLatin1String("do")},
        {QLatin1Char('d'), QLatin1String("dynamic_cast")},
        {QLatin1Char('e'), QLatin1String("else")},
        {QLatin1Char('e'), QLatin1String("explicit")},
        {QLatin1Char('f'), QLatin1String("for")},
        {QLatin1Char('g'), QLatin1String("goto")},
        {QLatin1Char('i'), QLatin1String("if")},
        {QLatin1Char('m'), QLatin1String("mutable")},
        {QLatin1Char('n'), QLatin1String("namespace")},
        {QLatin1Char('n'), QLatin1String("new")},
        {QLatin1Char('o'), QLatin1String("operator")},
        {QLatin1Char('p'), QLatin1String("private")},
        {QLatin1Char('p'), QLatin1String("protected")},
        {QLatin1Char('p'), QLatin1String("public")},
        {QLatin1Char('r'), QLatin1String("register")},
        {QLatin1Char('r'), QLatin1String("reinterpret_cast")},
        {QLatin1Char('r'), QLatin1String("return")},
        {QLatin1Char('s'), QLatin1String("signal")},
        {QLatin1Char('s'), QLatin1String("signed")},
        {QLatin1Char('s'), QLatin1String("sizeof")},
        {QLatin1Char('s'), QLatin1String("slot")},
        {QLatin1Char('s'), QLatin1String("static")},
        {QLatin1Char('s'), QLatin1String("static_cast")},
        {QLatin1Char('s'), QLatin1String("switch")},
        {QLatin1Char('t'), QLatin1String("template")},
        {QLatin1Char('t'), QLatin1String("this")},
        {QLatin1Char('t'), QLatin1String("throw")},
        {QLatin1Char('t'), QLatin1String("try")},
        {QLatin1Char('t'), QLatin1String("typedef")},
        {QLatin1Char('u'), QLatin1String("unsigned")},
        {QLatin1Char('u'), QLatin1String("using")},
        {QLatin1Char('v'), QLatin1String("volatile")},
        {QLatin1Char('w'), QLatin1String("while")}
    };

    types = {
        {QLatin1Char('b'), QLatin1String("bool")},
        {QLatin1Char('c'), QLatin1String("char")},
        {QLatin1Char('c'), QLatin1String("class")},
        {QLatin1Char('d'), QLatin1String("double")},
        {QLatin1Char('e'), QLatin1String("enum")},
        {QLatin1Char('f'), QLatin1String("float")},
        {QLatin1Char('i'), QLatin1String("int")},
        {QLatin1Char('l'), QLatin1String("long")},
        {QLatin1Char('Q'), QLatin1String("QHash")},
        {QLatin1Char('Q'), QLatin1String("QList")},
        {QLatin1Char('Q'), QLatin1String("QMap")},
        {QLatin1Char('Q'), QLatin1String("QString")},
        {QLatin1Char('Q'), QLatin1String("QVector")},
        {QLatin1Char('s'), QLatin1String("short")},
        {QLatin1Char('s'), QLatin1String("string")},
        {QLatin1Char('s'), QLatin1String("struct")},
        {QLatin1Char('u'), QLatin1String("union")},
        {QLatin1Char('v'), QLatin1String("vector")},
        {QLatin1Char('v'), QLatin1String("void")},
        {QLatin1Char('w'), QLatin1String("wchar_t")},
    };

    literals = {
        {QLatin1Char('f'), QLatin1String("false")},
        {QLatin1Char('n'), QLatin1String("nullptr")},
        {QLatin1Char('N'), QLatin1String("NULL")},
        {QLatin1Char('t'), QLatin1String("true")}
    };

    builtin = {
        {QLatin1Char('s'), QLatin1String("std")},
        {QLatin1Char('s'), QLatin1String("string")},
        {QLatin1Char('w'), QLatin1String("wstring")},
        {QLatin1Char('c'), QLatin1String("cin")},
        {QLatin1Char('c'), QLatin1String("cout")},
        {QLatin1Char('c'), QLatin1String("cerr")},
        {QLatin1Char('c'), QLatin1String("clog")},
        {QLatin1Char('s'), QLatin1String("stdin")},
        {QLatin1Char('s'), QLatin1String("stdout")},
        {QLatin1Char('s'), QLatin1String("stderr")},
        {QLatin1Char('s'), QLatin1String("stringstream")},
        {QLatin1Char('i'), QLatin1String("istringstream")},
        {QLatin1Char('o'), QLatin1String("ostringstream")},
        {QLatin1Char('a'), QLatin1String("auto_ptr")},
        {QLatin1Char('d'), QLatin1String("deque")},
        {QLatin1Char('l'), QLatin1String("list")},
        {QLatin1Char('q'), QLatin1String("queue")},
        {QLatin1Char('s'), QLatin1String("stack")},
        {QLatin1Char('v'), QLatin1String("vector")},
        {QLatin1Char('m'), QLatin1String("map")},
        {QLatin1Char('s'), QLatin1String("set")},
        {QLatin1Char('b'), QLatin1String("bitset")},
        {QLatin1Char('m'), QLatin1String("multiset")},
        {QLatin1Char('m'), QLatin1String("multimap")},
        {QLatin1Char('u'), QLatin1String("unordered_set")},
        {QLatin1Char('u'), QLatin1String("unordered_map")},
        {QLatin1Char('u'), QLatin1String("unordered_multiset")},
        {QLatin1Char('u'), QLatin1String("unordered_multimap")},
        {QLatin1Char('a'), QLatin1String("array")},
        {QLatin1Char('s'), QLatin1String("shared_ptr")},
        {QLatin1Char('a'), QLatin1String("abort")},
        {QLatin1Char('t'), QLatin1String("terminate")},
        {QLatin1Char('a'), QLatin1String("abs")},
        {QLatin1Char('a'), QLatin1String("acos")},
        {QLatin1Char('a'), QLatin1String("asin")},
        {QLatin1Char('a'), QLatin1String("atan2")},
        {QLatin1Char('a'), QLatin1String("atan")},
        {QLatin1Char('c'), QLatin1String("calloc")},
        {QLatin1Char('c'), QLatin1String("ceil")},
        {QLatin1Char('c'), QLatin1String("cosh")},
        {QLatin1Char('c'), QLatin1String("cos")},
        {QLatin1Char('e'), QLatin1String("exit")},
        {QLatin1Char('e'), QLatin1String("exp")},
        {QLatin1Char('f'), QLatin1String("fabs")},
        {QLatin1Char('f'), QLatin1String("floor")},
        {QLatin1Char('f'), QLatin1String("fmod")},
        {QLatin1Char('f'), QLatin1String("fprintf")},
        {QLatin1Char('f'), QLatin1String("fputs")},
        {QLatin1Char('f'), QLatin1String("free")},
        {QLatin1Char('f'), QLatin1String("frexp")},
        {QLatin1Char('f'), QLatin1String("fscanf")},
        {QLatin1Char('f'), QLatin1String("future")},
        {QLatin1Char('i'), QLatin1String("isalnum")},
        {QLatin1Char('i'), QLatin1String("isalpha")},
        {QLatin1Char('i'), QLatin1String("iscntrl")},
        {QLatin1Char('i'), QLatin1String("isdigit")},
        {QLatin1Char('i'), QLatin1String("isgraph")},
        {QLatin1Char('i'), QLatin1String("islower")},
        {QLatin1Char('i'), QLatin1String("isprint")},
        {QLatin1Char('i'), QLatin1String("ispunct")},
        {QLatin1Char('i'), QLatin1String("isspace")},
        {QLatin1Char('i'), QLatin1String("isupper")},
        {QLatin1Char('i'), QLatin1String("isxdigit")},
        {QLatin1Char('t'), QLatin1String("tolower")},
        {QLatin1Char('t'), QLatin1String("toupper")},
        {QLatin1Char('l'), QLatin1String("labs")},
        {QLatin1Char('l'), QLatin1String("ldexp")},
        {QLatin1Char('l'), QLatin1String("log10")},
        {QLatin1Char('l'), QLatin1String("log")},
        {QLatin1Char('m'), QLatin1String("malloc")},
        {QLatin1Char('r'), QLatin1String("realloc")},
        {QLatin1Char('m'), QLatin1String("memchr")},
        {QLatin1Char('m'), QLatin1String("memcmp")},
        {QLatin1Char('m'), QLatin1String("memcpy")},
        {QLatin1Char('m'), QLatin1String("memset")},
        {QLatin1Char('m'), QLatin1String("modf")},
        {QLatin1Char('p'), QLatin1String("pow")},
        {QLatin1Char('p'), QLatin1String("printf")},
        {QLatin1Char('p'), QLatin1String("putchar")},
        {QLatin1Char('p'), QLatin1String("puts")},
        {QLatin1Char('s'), QLatin1String("scanf")},
        {QLatin1Char('s'), QLatin1String("sinh")},
        {QLatin1Char('s'), QLatin1String("sin")},
        {QLatin1Char('s'), QLatin1String("snprintf")},
        {QLatin1Char('s'), QLatin1String("sprintf")},
        {QLatin1Char('s'), QLatin1String("sqrt")},
        {QLatin1Char('s'), QLatin1String("sscanf")},
        {QLatin1Char('s'), QLatin1String("strcat")},
        {QLatin1Char('s'), QLatin1String("strchr")},
        {QLatin1Char('s'), QLatin1String("strcmp")},
        {QLatin1Char('s'), QLatin1String("strcpy")},
        {QLatin1Char('s'), QLatin1String("strcspn")},
        {QLatin1Char('s'), QLatin1String("strlen")},
        {QLatin1Char('s'), QLatin1String("strncat")},
        {QLatin1Char('s'), QLatin1String("strncmp")},
        {QLatin1Char('s'), QLatin1String("strncpy")},
        {QLatin1Char('s'), QLatin1String("strpbrk")},
        {QLatin1Char('s'), QLatin1String("strrchr")},
        {QLatin1Char('s'), QLatin1String("strspn")},
        {QLatin1Char('s'), QLatin1String("strstr")},
        {QLatin1Char('t'), QLatin1String("tanh")},
        {QLatin1Char('t'), QLatin1String("tan")},
        {QLatin1Char('v'), QLatin1String("vfprintf")},
        {QLatin1Char('v'), QLatin1String("vprintf")},
        {QLatin1Char('v'), QLatin1String("vsprintf")},
        {QLatin1Char('e'), QLatin1String("endl")},
        {QLatin1Char('i'), QLatin1String("initializer_list")},
        {QLatin1Char('u'), QLatin1String("unique_ptr")},
        {QLatin1Char('c'), QLatin1String("complex")},
        {QLatin1Char('i'), QLatin1String("imaginary")}
    };

    other = {
        {QLatin1Char('d'), QLatin1String("define")},
        {QLatin1Char('e'), QLatin1String("else")},
        {QLatin1Char('e'), QLatin1String("elif")},
        {QLatin1Char('e'), QLatin1String("endif")},
        {QLatin1Char('e'), QLatin1String("error")},
        {QLatin1Char('i'), QLatin1String("if")},
        {QLatin1Char('i'), QLatin1String("ifdef")},
        {QLatin1Char('i'), QLatin1String("ifndef")},
        {QLatin1Char('i'), QLatin1String("include")},
        {QLatin1Char('l'), QLatin1String("line")},
        {QLatin1Char('p'), QLatin1String("pragma")},
        {QLatin1Char('P'), QLatin1String("_Pragma")},
        {QLatin1Char('u'), QLatin1String("undef")},
        {QLatin1Char('w'), QLatin1String("warning")}
    };
}

void loadShellData(QMultiHash<QChar, QString> &types,
             QMultiHash<QChar, QString> &keywords,
             QMultiHash<QChar, QString> &literals,
             QMultiHash<QChar, QString> &builtin,
             QMultiHash<QChar, QString> &other){
    keywords = {
        {QLatin1Char('i'), QLatin1String("if")},
        {QLatin1Char('t'), QLatin1String("then")},
        {QLatin1Char('e'), QLatin1String("else")},
        {QLatin1Char('e'), QLatin1String("elif")},
        {QLatin1Char('f'), QLatin1String("fi")},
        {QLatin1Char('f'), QLatin1String("for")},
        {QLatin1Char('w'), QLatin1String("while")},
        {QLatin1Char('i'), QLatin1String("in")},
        {QLatin1Char('d'), QLatin1String("do")},
        {QLatin1Char('d'), QLatin1String("done")},
        {QLatin1Char('c'), QLatin1String("case")},
        {QLatin1Char('e'), QLatin1String("esac")},
        {QLatin1Char('f'), QLatin1String("function")}
    };

    types = {
    };

    literals = {
        {QLatin1Char('f'), QLatin1String("false")},
        {QLatin1Char('t'), QLatin1String("true")}
    };

    builtin = {
        {QLatin1Char('b'), QLatin1String("break")},
        {QLatin1Char('c'), QLatin1String("cd")},
        {QLatin1Char('c'), QLatin1String("continue")},
        {QLatin1Char('e'), QLatin1String("eval")},
        {QLatin1Char('e'), QLatin1String("exec")},
        {QLatin1Char('e'), QLatin1String("exit")},
        {QLatin1Char('e'), QLatin1String("export")},
        {QLatin1Char('g'), QLatin1String("getopts")},
        {QLatin1Char('h'), QLatin1String("hash")},
        {QLatin1Char('p'), QLatin1String("pwd")},
        {QLatin1Char('r'), QLatin1String("readonly")},
        {QLatin1Char('r'), QLatin1String("return")},
        {QLatin1Char('s'), QLatin1String("shift")},
        {QLatin1Char('t'), QLatin1String("test")},
        {QLatin1Char('t'), QLatin1String("timestrap")},
        {QLatin1Char('u'), QLatin1String("umask")},
        {QLatin1Char('u'), QLatin1String("unset")},
        {QLatin1Char('B'), QLatin1String("Bash")},
        {QLatin1Char('a'), QLatin1String("alias")},
        {QLatin1Char('b'), QLatin1String("bind")},
        {QLatin1Char('b'), QLatin1String("builtin")},
        {QLatin1Char('c'), QLatin1String("caller")},
        {QLatin1Char('c'), QLatin1String("command")},
        {QLatin1Char('d'), QLatin1String("declare")},
        {QLatin1Char('e'), QLatin1String("echo")},
        {QLatin1Char('e'), QLatin1String("enable")},
        {QLatin1Char('h'), QLatin1String("help")},
        {QLatin1Char('l'), QLatin1String("let")},
        {QLatin1Char('l'), QLatin1String("local")},
        {QLatin1Char('l'), QLatin1String("logout")},
        {QLatin1Char('m'), QLatin1String("mapfile")},
        {QLatin1Char('p'), QLatin1String("printfread")},
        {QLatin1Char('r'), QLatin1String("readarray")},
        {QLatin1Char('s'), QLatin1String("source")},
        {QLatin1Char('t'), QLatin1String("type")},
        {QLatin1Char('t'), QLatin1String("typeset")},
        {QLatin1Char('u'), QLatin1String("ulimit")},
        {QLatin1Char('u'), QLatin1String("unalias")},
        {QLatin1Char('m'), QLatin1String("modifiers")},
        {QLatin1Char('s'), QLatin1String("set")},
        {QLatin1Char('s'), QLatin1String("shopt")},
        {QLatin1Char('a'), QLatin1String("autoload")},
        {QLatin1Char('b'), QLatin1String("bg")},
        {QLatin1Char('b'), QLatin1String("bindkey")},
        {QLatin1Char('b'), QLatin1String("bye")},
        {QLatin1Char('c'), QLatin1String("cap")},
        {QLatin1Char('c'), QLatin1String("chdir")},
        {QLatin1Char('c'), QLatin1String("clone")},
        {QLatin1Char('c'), QLatin1String("comparguments")},
        {QLatin1Char('c'), QLatin1String("compcall")},
        {QLatin1Char('c'), QLatin1String("compctl")},
        {QLatin1Char('c'), QLatin1String("compdescribe")},
        {QLatin1Char('c'), QLatin1String("compfilescompgroups")},
        {QLatin1Char('c'), QLatin1String("compquote")},
        {QLatin1Char('c'), QLatin1String("comptags")},
        {QLatin1Char('c'), QLatin1String("comptry")},
        {QLatin1Char('c'), QLatin1String("compvalues")},
        {QLatin1Char('d'), QLatin1String("dirs")},
        {QLatin1Char('d'), QLatin1String("disable")},
        {QLatin1Char('d'), QLatin1String("disown")},
        {QLatin1Char('e'), QLatin1String("echotc")},
        {QLatin1Char('e'), QLatin1String("echoti")},
        {QLatin1Char('e'), QLatin1String("emulatefc")},
        {QLatin1Char('f'), QLatin1String("fg")},
        {QLatin1Char('f'), QLatin1String("float")},
        {QLatin1Char('f'), QLatin1String("functions")},
        {QLatin1Char('g'), QLatin1String("getcap")},
        {QLatin1Char('g'), QLatin1String("getln")},
        {QLatin1Char('h'), QLatin1String("history")},
        {QLatin1Char('i'), QLatin1String("integer")},
        {QLatin1Char('j'), QLatin1String("jobs")},
        {QLatin1Char('k'), QLatin1String("kill")},
        {QLatin1Char('l'), QLatin1String("limit")},
        {QLatin1Char('l'), QLatin1String("log")},
        {QLatin1Char('n'), QLatin1String("noglob")},
        {QLatin1Char('p'), QLatin1String("popd")},
        {QLatin1Char('p'), QLatin1String("printpushd")},
        {QLatin1Char('p'), QLatin1String("pushln")},
        {QLatin1Char('r'), QLatin1String("rehash")},
        {QLatin1Char('s'), QLatin1String("sched")},
        {QLatin1Char('s'), QLatin1String("setcap")},
        {QLatin1Char('s'), QLatin1String("setopt")},
        {QLatin1Char('s'), QLatin1String("stat")},
        {QLatin1Char('s'), QLatin1String("suspend")},
        {QLatin1Char('t'), QLatin1String("ttyctl")},
        {QLatin1Char('u'), QLatin1String("unfunction")},
        {QLatin1Char('u'), QLatin1String("unhash")},
        {QLatin1Char('u'), QLatin1String("unlimitunsetopt")},
        {QLatin1Char('v'), QLatin1String("vared")},
        {QLatin1Char('w'), QLatin1String("wait")},
        {QLatin1Char('w'), QLatin1String("whence")},
        {QLatin1Char('w'), QLatin1String("where")},
        {QLatin1Char('w'), QLatin1String("which")},
        {QLatin1Char('z'), QLatin1String("zcompile")},
        {QLatin1Char('z'), QLatin1String("zformat")},
        {QLatin1Char('z'), QLatin1String("zftp")},
        {QLatin1Char('z'), QLatin1String("zle")},
        {QLatin1Char('z'), QLatin1String("zmodload")},
        {QLatin1Char('z'), QLatin1String("zparseopts")},
        {QLatin1Char('z'), QLatin1String("zprof")},
        {QLatin1Char('z'), QLatin1String("zpty")},
        {QLatin1Char('z'), QLatin1String("zregexparse")},
        {QLatin1Char('z'), QLatin1String("zsocket")},
        {QLatin1Char('z'), QLatin1String("zstyle")},
        {QLatin1Char('z'), QLatin1String("ztcp")}
    };

    other = {
    };
}

void loadJSData(QMultiHash<QChar, QString> &types,
             QMultiHash<QChar, QString> &keywords,
             QMultiHash<QChar, QString> &literals,
             QMultiHash<QChar, QString> &builtin,
             QMultiHash<QChar, QString> &other){
    keywords = {
        {QLatin1Char('i'), QLatin1String("in")},
        {QLatin1Char('o'), QLatin1String("of")},
        {QLatin1Char('i'), QLatin1String("if")},
        {QLatin1Char('f'), QLatin1String("for")},
        {QLatin1Char('w'), QLatin1String("while")},
        {QLatin1Char('f'), QLatin1String("finally")},
        {QLatin1Char('n'), QLatin1String("new")},
        {QLatin1Char('f'), QLatin1String("function")},
        {QLatin1Char('d'), QLatin1String("do")},
        {QLatin1Char('r'), QLatin1String("return")},
        {QLatin1Char('v'), QLatin1String("void")},
        {QLatin1Char('e'), QLatin1String("else")},
        {QLatin1Char('b'), QLatin1String("break")},
        {QLatin1Char('c'), QLatin1String("catch")},
        {QLatin1Char('i'), QLatin1String("instanceof")},
        {QLatin1Char('w'), QLatin1String("with")},
        {QLatin1Char('t'), QLatin1String("throw")},
        {QLatin1Char('c'), QLatin1String("case")},
        {QLatin1Char('d'), QLatin1String("default")},
        {QLatin1Char('t'), QLatin1String("try")},
        {QLatin1Char('t'), QLatin1String("this")},
        {QLatin1Char('s'), QLatin1String("switch")},
        {QLatin1Char('c'), QLatin1String("continue")},
        {QLatin1Char('t'), QLatin1String("typeof")},
        {QLatin1Char('d'), QLatin1String("delete")},
        {QLatin1Char('l'), QLatin1String("let")},
        {QLatin1Char('y'), QLatin1String("yield")},
        {QLatin1Char('c'), QLatin1String("const")},
        {QLatin1Char('e'), QLatin1String("export")},
        {QLatin1Char('s'), QLatin1String("super")},
        {QLatin1Char('d'), QLatin1String("debugger")},
        {QLatin1Char('a'), QLatin1String("as")},
        {QLatin1Char('a'), QLatin1String("async")},
        {QLatin1Char('a'), QLatin1String("await")},
        {QLatin1Char('s'), QLatin1String("static")},
        {QLatin1Char('i'), QLatin1String("import")},
        {QLatin1Char('f'), QLatin1String("from")},
        {QLatin1Char('a'), QLatin1String("as")}
    };

    types = {
        {QLatin1Char('v'), QLatin1String("var")},
        {QLatin1Char('c'), QLatin1String("class")},
        {QLatin1Char('b'), QLatin1String("byte")},
        {QLatin1Char('e'), QLatin1String("enum")},
        {QLatin1Char('f'), QLatin1String("float")},
        {QLatin1Char('s'), QLatin1String("short")},
        {QLatin1Char('l'), QLatin1String("long")},
        {QLatin1Char('i'), QLatin1String("int")},
        {QLatin1Char('v'), QLatin1String("void")},
        {QLatin1Char('b'), QLatin1String("boolean")},
        {QLatin1Char('d'), QLatin1String("double")}
    };

    literals = {
        {QLatin1Char('f'), QLatin1String("false")},
        {QLatin1Char('n'), QLatin1String("null")},
        {QLatin1Char('t'), QLatin1String("true")},
        {QLatin1Char('u'), QLatin1String("undefined")},
        {QLatin1Char('N'), QLatin1String("NaN")},
        {QLatin1Char('I'), QLatin1String("Infinity")}
    };

    builtin = {
        {QLatin1Char('e'), QLatin1String("eval")},
        {QLatin1Char('i'), QLatin1String("isFinite")},
        {QLatin1Char('i'), QLatin1String("isNaN")},
        {QLatin1Char('p'), QLatin1String("parseFloat")},
        {QLatin1Char('p'), QLatin1String("parseInt")},
        {QLatin1Char('d'), QLatin1String("decodeURI")},
        {QLatin1Char('d'), QLatin1String("decodeURIComponent")},
        {QLatin1Char('e'), QLatin1String("encodeURI")},
        {QLatin1Char('e'), QLatin1String("encodeURIComponent")},
        {QLatin1Char('e'), QLatin1String("escape")},
        {QLatin1Char('u'), QLatin1String("unescape")},
        {QLatin1Char('O'), QLatin1String("Object")},
        {QLatin1Char('F'), QLatin1String("Function")},
        {QLatin1Char('B'), QLatin1String("Boolean")},
        {QLatin1Char('E'), QLatin1String("Error")},
        {QLatin1Char('E'), QLatin1String("EvalError")},
        {QLatin1Char('I'), QLatin1String("InternalError")},
        {QLatin1Char('R'), QLatin1String("RangeError")},
        {QLatin1Char('R'), QLatin1String("ReferenceError")},
        {QLatin1Char('S'), QLatin1String("StopIteration")},
        {QLatin1Char('S'), QLatin1String("SyntaxError")},
        {QLatin1Char('T'), QLatin1String("TypeError")},
        {QLatin1Char('U'), QLatin1String("URIError")},
        {QLatin1Char('N'), QLatin1String("Number")},
        {QLatin1Char('M'), QLatin1String("Math")},
        {QLatin1Char('D'), QLatin1String("Date")},
        {QLatin1Char('S'), QLatin1String("String")},
        {QLatin1Char('R'), QLatin1String("RegExp")},
        {QLatin1Char('A'), QLatin1String("Array")},
        {QLatin1Char('F'), QLatin1String("Float32Array")},
        {QLatin1Char('F'), QLatin1String("Float64Array")},
        {QLatin1Char('I'), QLatin1String("Int16Array")},
        {QLatin1Char('I'), QLatin1String("Int32Array")},
        {QLatin1Char('I'), QLatin1String("Int8Array")},
        {QLatin1Char('U'), QLatin1String("Uint16Array")},
        {QLatin1Char('U'), QLatin1String("Uint32Array")},
        {QLatin1Char('U'), QLatin1String("Uint8Array")},
        {QLatin1Char('U'), QLatin1String("Uint8ClampedArray")},
        {QLatin1Char('A'), QLatin1String("ArrayBuffer")},
        {QLatin1Char('D'), QLatin1String("DataView")},
        {QLatin1Char('J'), QLatin1String("JSON")},
        {QLatin1Char('I'), QLatin1String("Intl")},
        {QLatin1Char('a'), QLatin1String("arguments")},
        {QLatin1Char('r'), QLatin1String("require")},
        {QLatin1Char('m'), QLatin1String("module")},
        {QLatin1Char('c'), QLatin1String("console")},
        {QLatin1Char('w'), QLatin1String("window")},
        {QLatin1Char('d'), QLatin1String("document")},
        {QLatin1Char('S'), QLatin1String("Symbol")},
        {QLatin1Char('S'), QLatin1String("Set")},
        {QLatin1Char('M'), QLatin1String("Map")},
        {QLatin1Char('W'), QLatin1String("WeakSet")},
        {QLatin1Char('W'), QLatin1String("WeakMap")},
        {QLatin1Char('P'), QLatin1String("Proxy")},
        {QLatin1Char('R'), QLatin1String("Reflect")},
        {QLatin1Char('P'), QLatin1String("Promise")}
    };

    other = {
    };

}

void loadPHPData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
            "array", "var", "class"
    };

    keywords = QStringList{
       "__halt_compiler", "abstract", "and", "as", "break", "callable", "case",
       "catch", "clone", "const", "continue", "declare", "default",
       "die", "do", "echo", "else", "elseif", "empty", "enddeclare", "endfor", "endforeach",
       "endif", "endswitch", "endwhile", "eval", "exit", "extends", "final", "finally", "for",
       "foreach", "function", "global", "goto", "if", "implements", "instanceof", "insteadof",
       "interface", "isset", "list", "namespace", "new", "or", "print", "private", "protected",
       "public", "return", "static", "switch", "throw", "trait", "try", "unset", "use", "while",
       "xor", "yield", "from"
    };

    preproc = QStringList{
       "include", "include_once", "require", "require_once", "__CLASS__", "__DIR__",
       "__FILE__", "__FUNCTION__", "__LINE__", "__METHOD__", "__NAMESPACE__", "__TRAIT__"

    };
}

//in accordance with https://doc.qt.io/qt-5/qtqml-syntax-objectattributes.html
void loadQMLData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
        "Rectangle", "Text", "color", "Item", "url", "Component", "Button", "TextInput",
        "ListView", "",
    };

    keywords = QStringList{
        "default", "property", "int", "string", "var", "true", "false",
        "function", "readonly", "MouseArea", "delegate", "enum", "if", "else"
    };

    preproc = QStringList{
        "import"
    };
}


void loadPythonData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
            "class"
    };

    keywords = QStringList{
       "False", "None", "True", "and", "as", "assert", "break", "continue",
       "def", "del", "elif", "else", "except", "finally", "for", "from", "global",
       "if", "in", "is", "lambda", "nonlocal", "not", "or", "pass", "raise",
       "return", "try", "while", "with", "yield"
    };

    preproc = QStringList{
            "import"
    };
}

void loadRustData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
            "enum", "struct", "union"
    };

    keywords = QStringList{
            "as", "use", "extern", "crate", "break", "const", "continue", "crate",
            "else", "if", "let", "extern", "false", "fn", "for", "impl", "in",
            "loop", "match", "mod", "move", "mut", "pub", "ref", "return",
            "self", "static", "super", "trait", "true", "type", "unsafe",
            "where", "while", "abstract", "alignof", "become", "box",
            "do", "final", "offsetof", "override", "priv", "proc", "pure",
            "sizeof", "typeof", "unsized", "virtual", "yield", "dyn", "async",
            "await", "try", "macro"
    };

    preproc = QStringList{
    };
}

void loadJavaData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
            "boolean", "byte", "enum", "int", "long", "short", "float", "void",
            "char", "class", "double", "String"
    };

    keywords = QStringList{
            "break", "case", "catch", "const", "for", "continue", "default",
            "do", "else", "extends", "while", "true",	"false", "null", "assert", "abstract",
            "final", "finally", "goto", "if", "implements",
            "instanceof", "interface", "native", "new", "package", "private", "protected",
            "public", "return", "static", "strictfp", "super", "switch", "synchronized",
            "this",	"throw", "throws", "transient",	"try", "volatile"
    };

    preproc = QStringList{
            "import"
    };
}

void loadCSharpData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
            "bool", "byte", "char", "decimal",
            "double", "enum", "float", "int",
            "long", "sbyte", "short", "struct",
            "unit", "ulong", "ushort", "class"
    };

    keywords = QStringList{
            "abstract", "as", "base", "break", "case", "catch", "checked",
            "const", "continue", "default", "delegate", "do", "else",
            "event", "explicit", "extern", "false", "finally", "fixed",
            "for", "foreach", "goto", "if", "implicit", "in", "interface",
            "internal", "is", "lock", "namespace", "new", "null", "object",
            "operator", "out", "override", "params", "private", "protected",
            "public", "readonly", "ref", "return", "sealed", "short",
            "sizeof", "stackalloc", "static", "switch", "this", "throw",
            "true", "try", "typeof", "unit", "unchecked", "unsafe",
            "using", "static", "virtual", "volatile", "while"
    };

    preproc = QStringList{

    };
}

void loadGoData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
           "var", "struct", "map"
    };

    keywords = QStringList{
            "break", "default", "func", "interface", "select",
            "case", "defer", "go", "chan", "else", "goto", "package",
            "switch", "const", "fallthrough", "if", "range", "type",
            "continue", "for", "import", "return"
    };

    preproc = QStringList{

    };
}

void loadVData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
           "struct", "map",
            "bool", "string", "i8", "i16",  "int",  "i64",  "i128",
            "byte",  "u16",  "u32",  "u64", "u128",
            "rune", "f32", "f64", "byteptr", "voidptr"
    };

    keywords = QStringList{
            "break", "const", "continue", "defer", "else", "enum",
            "fn", "for", "go", "goto", "if", "import", "in",
            "interface", "match", "module", "mut", "none",
            "or", "pub", "return", "type"
    };

    preproc = QStringList{

    };
}

void loadSQLData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
    };

    keywords = QStringList{
            "ABSOLUTE", "ACTION", "ADD", "ADMIN",
            "AFTER", "AGGREGATE", "ALIAS", "ALL",
            "ALLOCATE", "ALTER", "AND", "ANY",
            "ARE", "ARRAY", "AS", "ASC",
            "ASSERTION", "ASSERTION", "AT", "ATOMIC",
            "AUTHORIZATION", "BEFORE", "BEGIN", "BIGINT",
            "BINARY", "BIT", "BLOB", "BOOLEAN",
            "BOTH", "BREADTH", "BY", "CALL",
            "CASCADE", "CASCADED", "CASE", "CAST",
            "CATALOG", "CHAR", "CHARACTER", "CHECK",
            "CLASS", "CLOB", "CLOSE", "COLLATE",
            "COLLATION", "COLLECT", "COLUMN", "COMMIT",
            "COMPLETION", "CONDITION", "CONNECT", "CONNECTION",
            "CONSTRAINT", "CONSTRAINTS", "CONSTRUCTOR", "CONTAINS",
            "CONTINUE", "CORRESPONDING", "CREATE", "CROSS",
            "CUBE", "CURRENT", "CURRENT_DATE", "CURRENT_PATH",
            "CURRENT_ROLE", "CURRENT_TIME", "CURRENT_TIMESTAMP", "CURRENT_USER",
            "CURSOR", "CYCLE", "DATA", "DATALINK",
            "DATE", "DAY", "DEALLOCATE", "DEC",
            "DECIMAL", "DECLARE", "DEFAULT", "DEFERRABLE",
            "DELETE", "DEPTH", "DEREF", "DESC",
            "DESCRIPTOR", "DESTRUCTOR", "DIAGNOSTICS", "DICTIONARY",
            "DISCONNECT", "DO", "DOMAIN", "DOUBLE",
            "DROP", "ELEMENT", "END", "EXEC", "EQUALS",
            "ESCAPE", "EXCEPT", "EXCEPTION", "EXECUTE",
            "EXIT", "EXPAND", "EXPANDING", "FALSE",
            "FIRST", "FLOAT", "FOR", "FOREIGN",
            "FREE", "FROM", "FUNCTION", "FUSION",
            "GENERAL", "GET", "GLOBAL", "GOTO",
            "GROUP", "GROUPING", "HANDLER", "HASH",
            "HOUR", "IDENTITY", "IF", "IGNORE",
            "IMMEDIATE", "IN", "INDICATOR", "INITIALIZE",
            "INITIALLY", "INNER", "INOUT", "INPUT",
            "INSERT", "INT", "INTEGER", "INTERSECT",
            "INTERSECTION", "INTERVAL", "INTO", "IS",
            "ISOLATION", "ITERATE", "JOIN", "KEY",
            "LANGUAGE", "LARGE", "LAST", "LATERAL",
            "LEADING", "LEAVE", "LEFT", "LESS",
            "LEVEL", "LIKE", "LIMIT", "LOCAL",
            "LOCALTIME", "LOCALTIMESTAMP", "LOCATOR", "LOOP",
            "MATCH", "MEMBER", "MEETS", "MERGE",
            "MINUTE", "MODIFIES", "MODIFY", "MODULE",
            "MONTH", "MULTISET", "NAMES", "NATIONAL",
            "NATURAL", "NCHAR", "NCLOB", "NEW",
            "NEXT", "NO", "NONE", "NORMALIZE",
            "NOT", "NULL", "NUMERIC", "OBJECT",
            "OF", "OFF", "OLD", "ON",
            "ONLY", "OPEN", "OPERATION", "OPTION",
            "OR", "ORDER", "ORDINALITY", "OUT",
            "OUTER", "OUTPUT", "PAD", "PARAMETER",
            "PARAMETERS", "PARTIAL", "PATH", "PERIOD",
            "POSTFIX", "PRECEDES", "PRECISION", "PREFIX",
            "PREORDER", "PREPARE", "PRESERVE", "PRIMARY",
            "PRIOR", "PRIVILEGES", "PROCEDURE", "PUBLIC",
            "READ", "READS", "REAL", "RECURSIVE",
            "REDO", "REF", "REFERENCES", "REFERENCING",
            "RELATIVE", "REPEAT", "RESIGNAL", "RESTRICT",
            "RESULT", "RETURN", "RETURNS", "REVOKE",
            "RIGHT", "ROLE", "ROLLBACK", "ROLLUP",
            "ROUTINE", "ROW", "ROWS", "SAVEPOINT",
            "SCHEMA", "SCROLL", "SEARCH", "SECOND",
            "SECTION", "SELECT", "SEQUENCE", "SESSION",
            "SESSION_USER", "SET", "SETS", "SIGNAL",
            "SIZE", "SMALLINT", "SPECIFIC", "SPECIFICTYPE",
            "SQL", "SQLEXCEPTION", "SQLSTATE", "SQLWARNING",
            "START", "STATE", "STATIC", "STRUCTURE",
            "SUBMULTISET", "SUCCEEDS", "SUM", "SYSTEM_USER",
            "TABLE", "TABLESAMPLE", "TEMPORARY", "TERMINATE",
            "THAN", "THEN", "TIME", "TIMESTAMP",
            "TIMEZONE_HOUR", "TIMEZONE_MINUTE", "TO", "TRAILING",
            "TRANSACTION", "TRANSLATION", "TREAT", "TRIGGER",
            "TRUE", "UESCAPE", "UNDER", "UNDO",
            "UNION", "UNIQUE", "UNKNOWN", "UNTIL",
            "UPDATE", "USAGE", "USER", "USING",
            "VALUE", "VALUES", "VARCHAR", "VARIABLE",
            "VARYING", "VIEW", "WHEN", "WHENEVER",
            "WHERE", "WHILE", "WITH", "WRITE",
            "YEAR", "ZONE"
    };

    preproc = QStringList{

    };
}
