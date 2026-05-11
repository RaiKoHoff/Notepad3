#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_LUA =
{
// Keywords
    "and break do else elseif end false for function goto if in local nil not or repeat return "
    "then true until while",
// Basic Functions
    "_ALERT _ERRORMESSAGE _G _INPUT _OUTPUT _PROMPT _STDERR _STDIN _STDOUT _VERSION assert call collectgarbage "
    "coroutine debug dofile dostring error foreach foreachi gcinfo getfenv getmetatable getn globals io ipairs "
    "load loadfile loadlib loadstring math module newtype next os pairs pcall print rawequal rawget rawset "
    "require select setfenv setmetatable sort string table tinsert tonumber tostring tremove type unpack xpcall"
    " LUA_PATH _ENV _LOADED _REQUIREDNAME bit32 package rawlen utf8",
// String Manipulation, Table Manipulation, Mathematical Functions
    "abs acos asin atan atan2 ceil cos deg exp floor format frexp gsub ldexp log log10 math.abs math.acos "
    "math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp math.floor math.fmod "
    "math.frexp math.huge math.ldexp math.log math.log10 math.max math.min math.mod math.modf math.pi math.pow "
    "math.rad math.random math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh max min mod rad "
    "random randomseed sin sqrt strbyte strchar strfind string.byte string.char string.dump string.find "
    "string.format string.gfind string.gmatch string.gsub string.len string.lower string.match string.rep "
    "string.reverse string.sub string.upper strlen strlower strrep strsub strupper table.concat table.foreach "
    "table.foreachi table.getn table.insert table.maxn table.remove table.setn table.sort tan"
    " bit32.arshift bit32.band bit32.bnot bit32.bor bit32.btest bit32.bxor bit32.extract bit32.lrotate bit32.lshift bit32.replace bit32.rrotate bit32.rshift math.maxinteger math.mininteger math.tointeger math.type math.ult string.pack string.packsize string.unpack table.move table.pack table.unpack utf8.char utf8.charpattern utf8.codepoint utf8.codes utf8.len utf8.offset",
// Input and Output Facilities & System Facilities Coroutine Manipulation,
// Input and Output Facilities, System Facilities (coroutine & io & os)
    "appendto clock closefile coroutine.create coroutine.resume coroutine.running coroutine.status "
    "coroutine.wrap coroutine.yield date difftime execute exit flush getenv io.close io.flush io.input "
    "io.lines io.open io.output io.popen io.read io.stderr io.stdin io.stdout io.tmpfile io.type io.write "
    "openfile os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale "
    "os.time os.tmpname package.cpath package.loaded package.loadlib package.path package.preload "
    "package.seeall read readfrom remove rename seek setlocale time tmpfile tmpname write writeto"
    " coroutine.isyieldable package.config package.loaders package.searchers package.searchpath",
// User keywords 1 (set via WordListSet, populated by theme or user)
    "",
// User keywords 2
    "",
// User keywords 3
    "",
// User keywords 4
    "",
    NULL,
};


EDITLEXER lexLUA =
{
    SCLEX_LUA, "lua", IDS_LEX_LUA, L"Lua Script", L"lua; wlua; nse; luadoc; luax", L"",
    &KeyWords_LUA, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_LUA_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_LUA_COMMENT,SCE_LUA_COMMENTLINE,SCE_LUA_COMMENTDOC,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_LUA_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#008080", L"" },
        { {SCE_LUA_WORD}, IDS_LEX_STR_Keyword, L"Keyword", L"fore:#00007F", L"" },
        { {SCE_LUA_WORD2}, IDS_LEX_STR_BaseFct, L"Basic Functions", L"fore:#00007F", L"" },
        { {SCE_LUA_WORD3}, IDS_LEX_STR_63299, L"String, Table & Math Functions", L"fore:#00007F", L"" },
        { {SCE_LUA_WORD4}, IDS_LEX_STR_63300, L"Input, Output & System Facilities", L"fore:#00007F", L"" },
        { {MULTI_STYLE(SCE_LUA_STRING,SCE_LUA_STRINGEOL,SCE_LUA_CHARACTER,0)}, IDS_LEX_STR_String, L"String", L"fore:#B000B0", L"" },
        { {SCE_LUA_LITERALSTRING}, IDS_LEX_STR_LitStrg, L"Literal String", L"fore:#B000B0", L"" },
        { {SCE_LUA_PREPROCESSOR}, IDS_LEX_STR_PreProc, L"Preprocessor", L"fore:#FF8000", L"" },
        { {SCE_LUA_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"", L"" },
        { {SCE_LUA_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {SCE_LUA_LABEL}, IDS_LEX_STR_Label, L"Label", L"fore:#808000", L"" },
        //{ {SCE_LUA_WORD5}, IDS_LEX_STR_Word, L"Keyword 5", L"", L"" },
        //{ {SCE_LUA_WORD6}, IDS_LEX_STR_Word, L"Keyword 6", L"", L"" },
        //{ {SCE_LUA_WORD7}, IDS_LEX_STR_Word, L"Keyword 7", L"", L"" },
        //{ {SCE_LUA_WORD8}, IDS_LEX_STR_Word, L"Keyword 8", L"", L"" },
        EDITLEXER_SENTINEL
    }
};
