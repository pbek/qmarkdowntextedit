#include <QMultiHash>

void loadCppData(QMultiHash<QChar, QString> &types,
             QMultiHash<QChar, QString> &keywords,
             QMultiHash<QChar, QString> &literals,
             QMultiHash<QChar, QString> &builtin,
             QMultiHash<QChar, QString> &other){
    keywords = {
        {'a', "asm"},
        {'a', "auto"},
        {'b', "break"},
        {'c', "case"},
        {'c', "catch"},
        {'c', "const"},
        {'c', "const_cast"},
        {'c', "continue"},
        {'d', "default"},
        {'d', "delete"},
        {'d', "do"},
        {'d', "dynamic_cast"},
        {'e', "else"},
        {'e', "explicit"},
        {'f', "for"},
        {'g', "goto"},
        {'i', "if"},
        {'m', "mutable"},
        {'n', "namespace"},
        {'n', "new"},
        {'o', "operator"},
        {'p', "private"},
        {'p', "protected"},
        {'p', "public"},
        {'r', "register"},
        {'r', "reinterpret_cast"},
        {'r', "return"},
        {'s', "signal"},
        {'s', "signed"},
        {'s', "sizeof"},
        {'s', "slot"},
        {'s', "static"},
        {'s', "static_cast"},
        {'s', "switch"},
        {'t', "template"},
        {'t', "this"},
        {'t', "throw"},
        {'t', "try"},
        {'t', "typedef"},
        {'u', "unsigned"},
        {'u', "using"},
        {'v', "volatile"},
        {'w', "while"}
    };

    types = {
        {'b', "bool"},
        {'c', "char"},
        {'c', "class"},
        {'d', "double"},
        {'e', "enum"},
        {'f', "float"},
        {'i', "int"},
        {'l', "long"},
        {'Q', "QHash"},
        {'Q', "QList"},
        {'Q', "QMap"},
        {'Q', "QString"},
        {'Q', "QVector"},
        {'s', "short"},
        {'s', "string"},
        {'s', "struct"},
        {'u', "union"},
        {'v', "vector"},
        {'v', "void"},
        {'w', "wchar_t"},
    };

    literals = {
        {'f',"false"},
        {'n',"nullptr"},
        {'N',"NULL"},
        {'t',"true"}
    };

    other = {
        {'d', "define"},
        {'e', "else"},
        {'e', "elif"},
        {'e', "endif"},
        {'e', "error"},
        {'i', "if"},
        {'i', "ifdef"},
        {'i', "ifndef"},
        {'i', "include"},
        {'l', "line"},
        {'p', "pragma"},
        {'P', "_Pragma"},
        {'u', "undef"},
        {'w', "warning"}
    };
}

void loadCData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
        "int", "float", "string", "double", "long", "vector",
        "short", "char", "void", "bool", "wchar_t",
        "class", "struct", "union", "enum"
    };

    keywords = QStringList{
        "while", "if", "for", "do", "return", "else", "switch",
        "case", "break", "continue", "auto", "sizeof", "extern"
        "unsigned", "const", "static", "signed", "typedef",
        "asm ", "volatile", "false", "true", "register", "default",
        "goto"
    };

    preproc = QStringList{
        "ifndef", "ifdef", "include", "define", "endif", "NULL"
    };
}

void loadJSData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
            "byte", "class", "enum", "float", "short", "long", "int", "var", "void",
             "boolean", "double"

    };

    keywords = QStringList{
            "abstract", "arguments", "await", "break", "case", "catch",
            "char", "const", "continue", "debugger", "default", "delete", "do",
            "else", "eval", "export", "extends","false", "final", "finally",  "for",
            "function", "goto", "if", "implements", "in", "instanceof", "interface",
            "let",  "native", "new", "null", "package", "private", "protected",
            "public", "return", "static", "super", "switch", "synchronized", "this",
            "throw", "throws", "transient", "true", "try", "typeof", "volatile", "while",
            "with", "yield"
    };

    preproc = QStringList{
            "import"
    };
}

void loadShData(QStringList &types, QStringList &keywords, QStringList &preproc) {
    types = QStringList{
    };

    keywords = QStringList{
       "if", "then", "else", "elif", "fi", "case", "esac", "for", "select",
       "while", "until", "do", "done", "in", "function"
    };

    preproc = QStringList{
       "time"
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
