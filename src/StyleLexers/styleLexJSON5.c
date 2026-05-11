#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_JSON5 =
{
    // JSON5 Keywords (per https://spec.json5.org/ § BooleanLiteral / NullLiteral / NumericLiteral)
    "+Infinity -Infinity +NaN -NaN Infinity NaN false null true",

    // JSON-LD Keywords (W3C JSON-LD 1.1 § 1.7 + JSON-LD 1.1 Framing § 1.5)
    "@base @container @context @default @direction @embed @explicit @graph @id @import @included "
    "@index @json @language @list @nest @none @null @omitDefault @prefix @propagate @protected "
    "@requireAll @reverse @set @type @value @version @vocab",

    NULL
};


EDITLEXER lexJSON5 =
{
    SCLEX_JSON5, "json5", IDS_LEX_JSON5, L"JSON5", L"json; json5; har; ipynb; wxcp; jshintrc; eslintrc; babelrc; prettierrc; stylelintrc; jsonld; jsonc; arcconfig; arclint; jscop", L"",
    &KeyWords_JSON5, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_JSON5_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_JSON5_LINECOMMENT,SCE_JSON5_BLOCKCOMMENT,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#646464", L"" },
        { {SCE_JSON5_KEYWORD}, IDS_LEX_STR_Keyword, L"Keyword", L"bold; fore:#957000", L"" },
        { {SCE_JSON5_LDKEYWORD}, IDS_LEX_STR_63365, L"LD Keyword", L"bold; fore:#A61D04", L"" },
        { {SCE_JSON5_STRING}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {SCE_JSON5_STRINGEOL}, IDS_LEX_STR_63379, L"Unclosed String", L"fore:#FFFFFF; back:#FF8040; eolfilled", L"" },
        { {SCE_JSON5_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_JSON5_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#B000B0", L"" },
        { {SCE_JSON5_PROPERTYNAME}, IDS_LEX_STR_63364, L"Property Name", L"fore:#002697", L"" },
        { {SCE_JSON5_ESCAPESEQUENCE}, IDS_LEX_STR_63366, L"ESC Sequence", L"fore:#0B982E", L"" },
        { {SCE_JSON5_URI}, IDS_LEX_STR_63380, L"URL/IRI", L"fore:#0000FF", L"" },
        { {SCE_JSON5_COMPACTIRI}, IDS_LEX_STR_63381, L"Compact IRI", L"fore:#002697", L"" },
        { {SCE_JSON5_ERROR}, IDS_LEX_STR_63252, L"Parsing Error", L"fore:#FFFF00; back:#A02020; eolfilled", L"" },
        EDITLEXER_SENTINEL
    }
};

