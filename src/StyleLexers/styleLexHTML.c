#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_HTML =
{
// HTML elements and attributes
    "!doctype ^aria- ^data- a abbr accept accept-charset accesskey acronym action address align alink alt and "
    "applet archive area article aside async audio autocomplete autofocus autoplay axis b background base "
    "basefont bb bdi bdo bgcolor big blockquote body border bordercolor br buffered button canvas caption "
    "cellpadding cellspacing center challenge char charoff charset checkbox checked cite class classid clear "
    "code codebase codetype col colgroup color cols colspan command compact content contenteditable "
    "contextmenu controls coords crossorigin data datafld dataformatas datagrid datalist datapagesize datasrc "
    "datetime dd declare default defer del details dfn dialog dir dirname disabled div dl download draggable "
    "dropzone dt em embed enctype event eventsource face fieldset figcaption figure file font footer for form "
    "formaction formenctype formmethod formnovalidate formtarget frame frameborder frameset h1 h2 h3 h4 h5 h6 "
    "head header headers height hgroup hidden high hr href hreflang hspace html http-equiv i icon id iframe "
    "image img input ins integrity isindex ismap itemprop itemscope itemtype kbd keygen keytype kind label "
    "lang language leftmargin legend li link list longdesc loop low main manifest map marginheight marginwidth "
    "mark max maxlength media mediagroup menu menuitem meta meter method min multiple muted name nav noframes "
    "nohref noresize noscript noshade novalidate nowrap object ol onabort onafterprint onbeforeprint "
    "onbeforeunload onblur oncancel oncanplay oncanplaythrough onchange onclick onclose oncontextmenu "
    "oncuechange ondblclick ondrag ondragend ondragenter ondragleave ondragover ondragstart ondrop "
    "ondurationchange onemptied onended onerror onfocus onformchange onforminput onhashchange oninput "
    "oninvalid onkeydown onkeypress onkeyup onload onloadeddata onloadedmetadata onloadstart onmessage "
    "onmousedown onmousemove onmouseout onmouseover onmouseup onmousewheel onoffline ononline onpagehide "
    "onpageshow onpause onplay onplaying onpopstate onprogress onratechange onreadystatechange onredo onreset "
    "onresize onscroll onseeked onseeking onselect onshow onstalled onstorage onsubmit onsuspend ontimeupdate "
    "onundo onunload onvolumechange onwaiting open optgroup optimum option output p param password pattern "
    "picture ping placeholder poster pre prefix preload profile progress prompt property pubdate public q "
    "radio radiogroup readonly rel required reset rev reversed role rows rowspan rp rt ruby rules s samp "
    "sandbox scheme scope scoped script scrolling seamless section select selected shape size sizes slot small "
    "source span spellcheck src srcdoc srclang standby start step strike strong style sub submit summary sup "
    "tabindex table target tbody td template text textarea tfoot th thead time title topmargin tr track "
    "translate tt type typemustmatch u ul usemap valign value valuetype var version video vlink vspace wbr "
    "width wrap xml xmlns"
    " abbrev accel ackno affiliation age allowfullscreen allowpaymentrequest allowtransparency alphabet anchor answer appendix appendixinfo application arch areaset areaspec arg articleinfo artpagenums assign attribution audiodata audioobject author authorblurb authorgroup authorinitials beep beginpage bgsound bibliocoverage bibliodiv biblioentry bibliography bibliographyinfo biblioid bibliomisc bibliomixed bibliomset bibliorelation biblioset bibliosource blink block blockinfo book bookinfo bottommargin bridge bridgehead callout calloutlist category caution chapter chapterinfo choice citation citebiblioid citerefentry citetitle city classname classsynopsis classsynopsisinfo cmdsynopsis co collab collabname colophon colspec computeroutput cond condition confdates confgroup confnum conformance confsponsor conftitle connecttimeout constant constraint constraintdef constructorsynopsis contour contractnum contractsponsor contrib copyright coref corpauthor corpname count country database dedication dest destexpr destructorsynopsis disconnect dtmf dtmfterm duration edition editor email emphasis endterm entry entrytbl enumerate envar epigraph equation errorcode errorname errortext errortype eventexpr example exceptionname expr expritem fax fetchtimeout field fieldsynopsis filename filled finalsilence firstname firstterm footnote footnoteref foreignphrase formalpara framespacing funcdef funcparams funcprototype funcsynopsis funcsynopsisinfo gender glossary glossaryinfo glossdef glossdiv glossentry glosslist glosssee glossseealso glossterm grammar graphic graphicco group guibutton guiicon guilabel guimenu guimenuitem guisubmenu hardware help highlights holder honorific htm imagedata imageobject imageobjectco important index indexdiv indexentry indexinfo indexterm informalequation informalexample informalfigure informaltable initial initializer inlineequation inlinegraphic inlinemediaobject interfacename invpartnumber isbn issn issuenum itemid itemizedlist itemref iteral itermset jobtitle keycap keycode keycombo keysym keyword keywordset legalnotice level lhs lineage lineannotation linkend listing listitem literallayout log lot lotentry lowsrc manvolnum markup marquee maxage maxstale maxtime medialabel mediaobject mediaobjectco member menuchoice message messageexpr methodname methodparam methods methodsynopsis mime minlength mm modal mode modespec modifier msg msgaud msgentry msgexplan msginfo msglevel msgmain msgorig msgrel msgset msgsub msgtext multicol namelist nextitem nobr noembed noinput nomatch nonce nonterminal note objectinfo olink onaddtrack onauxclick oncopy oncut ondragexit onenter onexit onlanguagechange onloadend onmouseenter onmouseleave onpaste onrejectionhandled onremovetrack ontoggle onunhandledrejection onwheel ooclass ooexception oointerface orderedlist orgdiv orgname os otheraddr othercredit othername ousebutton pagenums para paragraph paramdef parameter part partinfo partintro personblurb personname ph phone phoneme phrase pitch plaintext pob postcode preface prefaceinfo primary primaryie printhistory procedure production productionrecap productionset productname productnumber programlisting programlistingco prosody publisher publishername pubsnumber qandadiv qandaentry qandaset question quote range rate rb record refclass refdescriptor refentry refentryinfo refentrytitle reference referenceinfo referrerpolicy refmeta refmiscinfo refname refnamediv refpurpose refsect1 refsect1info refsect2 refsect2info refsect3 refsect3info refsection refsectioninfo refsynopsisdiv refsynopsisdivinfo releaseinfo remap remark replaceable reprompt returnvalue revdescription revhistory revision revisionflag revnumber revremark rhs rightmargin row rtc say-as sbr screen screenco screeninfo screenshot secondary secondaryie sect1 sect1info sect2 sect2info sect3 sect3info sect4 sect4info sect5 sect5info sectioninfo security see seealso seealsoie seeie seg seglistitem segmentedlist segtitle sentence seriesvolnums setindex setindexinfo setinfo sgmltag shortaffil shortcut sidebar sidebarinfo simpara simplelist simplemsgentry simplesect sizeexpr skiplist space spacer spanspec srcexpr srcset state status street structfield structname subdialog subject subjectset subjectterm subscript substeps subtitle superscript surname sv symbol synopfragment synopfragmentref synopsis systemitem term tertiary tertiaryie textdata textobject tgroup timeexpr timeout tip titleabbrev toc tocback tocchap tocentry tocfront toclevel1 toclevel2 toclevel3 toclevel4 toclevel5 tocpart token trademark transfer transferaudio ulink urn userinput userlevel varargs variablelist varlistentry varname vendor videodata videoobject voice volume volumenum vxml warning wordasword xml:lang xmp xref xreflabel year",
// JavaScript keywords (sync with lexJS::KeyWords_JS)
    NP3_LEXER_JS_KEYWORD_LIST,
// VBScript keywords
    NP3_LEXER_VB_KEYWORD_LIST,
// Python keywords
    ""
    " block doc endclass endexcept endfinally endtry inherit page",
// PHP keywords
    "__callstatic __class__ __compiler_halt_offset__ __dir__ __file__ __function__ __get __halt_compiler __isset "
    "__line__ __method__ __namespace__ __set __sleep __trait__ __unset __wakeup abstract and argc argv array "
    "as break callable case catch cfunction class clone closure const continue declare default define die "
    "directory do e_all e_compile_error e_compile_warning e_core_error e_core_warning e_deprecated e_error "
    "e_fatal e_notice e_parse e_strict e_user_deprecated e_user_error e_user_notice e_user_warning e_warning "
    "echo else elseif empty enddeclare endfor endforeach endif endswitch endwhile eval exception exit extends "
    "false final for foreach function global goto http_cookie_vars http_env_vars http_get_vars http_post_files "
    "http_post_vars http_server_vars if implements include include_once instanceof insteadof interface isset "
    "list namespace new not null old_function or parent php_self print private protected public require "
    "require_once return static stdclass switch this throw trait true try unset use var virtual while xor"
    " __php_incomplete_class __property__ arithmeticerror assertionerror bool divisionbyzeroerror errorexception fn generator iterable match mixed never numeric parseerror php_user_filter resource self throwable typeerror",
// SGML and DTD keywords
    ""
    " ATTLIST DOCTYPE ELEMENT ENTITY NOTATION",
    NULL,
};


EDITLEXER lexHTML =
{
    SCLEX_HTML, "hypertext", IDS_LEX_WEB_SRC, L"Web Source Code", L"html; htm; asp; aspx; shtml; htd; xhtml; php; php3; phtml; htt; cfm; tpl; dtd; hta; htc; jsp; mht; jd", L"",
    &KeyWords_HTML, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_H_TAG,SCE_H_TAGEND,0,0)}, IDS_LEX_STR_63136, L"HTML Tag", L"fore:#648000", L"" },
        { {SCE_H_TAGUNKNOWN}, IDS_LEX_STR_63137, L"HTML Unknown Tag", L"fore:#C80000; back:#FFFF80", L"" },
        { {SCE_H_ATTRIBUTE}, IDS_LEX_STR_63138, L"HTML Attribute", L"fore:#FF4000", L"" },
        { {SCE_H_ATTRIBUTEUNKNOWN}, IDS_LEX_STR_63139, L"HTML Unknown Attribute", L"fore:#C80000; back:#FFFF80", L"" },
        { {SCE_H_VALUE}, IDS_LEX_STR_63140, L"HTML Value", L"fore:#3A6EA5", L"" },
        { {MULTI_STYLE(SCE_H_DOUBLESTRING,SCE_H_SINGLESTRING,0,0)}, IDS_LEX_STR_63141, L"HTML String", L"fore:#3A6EA5", L"" },
        { {SCE_H_OTHER}, IDS_LEX_STR_63142, L"HTML Other Inside Tag", L"fore:#3A6EA5", L"" },
        { {MULTI_STYLE(SCE_H_COMMENT,SCE_H_XCCOMMENT,0,0)}, IDS_LEX_STR_63143, L"HTML Comment", L"fore:#646464", L"" },
        { {SCE_H_ENTITY}, IDS_LEX_STR_63144, L"HTML Entity", L"fore:#B000B0", L"" },
        { {SCE_H_DEFAULT}, IDS_LEX_STR_63256, L"HTML Element Text", L"", L"" },
        { {MULTI_STYLE(SCE_H_XMLSTART,SCE_H_XMLEND,0,0)}, IDS_LEX_STR_63145, L"XML Identifier", L"bold; fore:#881280", L"" },
        { {SCE_H_SGML_DEFAULT}, IDS_LEX_STR_63237, L"SGML", L"fore:#881280", L"" },
        { {SCE_H_SGML_COMMAND}, IDS_LEX_STR_SGMLCmd, L"SGML Command", L"bold; fore:#881280", L"" },
        { {SCE_H_SGML_1ST_PARAM}, IDS_LEX_STR_SGMLParam, L"SGML 1st Param", L"fore:#A46000", L"" },
        { {MULTI_STYLE(SCE_H_SGML_DOUBLESTRING,SCE_H_SGML_SIMPLESTRING,0,0)}, IDS_LEX_STR_SGMLStr, L"SGML String", L"fore:#3A6EA5", L"" },
        { {SCE_H_SGML_ERROR}, IDS_LEX_STR_SGMLErr, L"SGML Error", L"fore:#FFFF00; back:#A00000; eolfilled", L"" },
        { {SCE_H_SGML_SPECIAL}, IDS_LEX_STR_SGMLSpc, L"SGML Special", L"fore:#AA00AA", L"" },
        { {SCE_H_SGML_ENTITY}, IDS_LEX_STR_SGMLEnt, L"SGML Entity", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_H_SGML_COMMENT,SCE_H_SGML_1ST_PARAM_COMMENT,0,0)}, IDS_LEX_STR_SGMLCmt, L"SGML Comment", L"italic; fore:#646464", L"" },
        { {SCE_H_SGML_BLOCK_DEFAULT}, IDS_LEX_STR_SGMLBlk, L"SGML Block Default", L"", L"" },
        { {SCE_H_CDATA}, IDS_LEX_STR_63147, L"CDATA", L"fore:#646464", L"" },
        { {MULTI_STYLE(SCE_H_ASP,SCE_H_ASPAT,0,0)}, IDS_LEX_STR_63146, L"ASP Start Tag", L"bold; fore:#000080", L"" },
        //{ {SCE_H_SCRIPT}, L"Script", L"", L"" },
        { {SCE_H_QUESTION}, IDS_LEX_STR_63148, L"PHP Start Tag", L"bold; fore:#000080", L"" },
        { {SCE_HPHP_DEFAULT}, IDS_LEX_STR_63149, L"PHP Default", L"", L"" },
        { {MULTI_STYLE(SCE_HPHP_COMMENT,SCE_HPHP_COMMENTLINE,0,0)}, IDS_LEX_STR_63157, L"PHP Comment", L"fore:#FF8000", L"" },
        { {SCE_HPHP_WORD}, IDS_LEX_STR_63152, L"PHP Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_HPHP_HSTRING}, IDS_LEX_STR_63150, L"PHP String", L"fore:#008000", L"" },
        { {SCE_HPHP_SIMPLESTRING}, IDS_LEX_STR_63151, L"PHP Simple String", L"fore:#008000", L"" },
        { {SCE_HPHP_NUMBER}, IDS_LEX_STR_63153, L"PHP Number", L"fore:#FF0000", L"" },
        { {SCE_HPHP_OPERATOR}, IDS_LEX_STR_63158, L"PHP Operator", L"fore:#B000B0", L"" },
        { {SCE_HPHP_VARIABLE}, IDS_LEX_STR_63154, L"PHP Variable", L"italic; fore:#000080", L"" },
        { {SCE_HPHP_HSTRING_VARIABLE}, IDS_LEX_STR_63155, L"PHP String Variable", L"italic; fore:#000080", L"" },
        { {SCE_HPHP_COMPLEX_VARIABLE}, IDS_LEX_STR_63156, L"PHP Complex Variable", L"italic; fore:#000080", L"" },
        { {MULTI_STYLE(SCE_HJ_DEFAULT,SCE_HJ_START,0,0)}, IDS_LEX_STR_63159, L"JS Default", L"", L"" },
        { {MULTI_STYLE(SCE_HJ_COMMENT,SCE_HJ_COMMENTLINE,SCE_HJ_COMMENTDOC,0)}, IDS_LEX_STR_63160, L"JS Comment", L"fore:#646464", L"" },
        { {SCE_HJ_KEYWORD}, IDS_LEX_STR_63163, L"JS Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_HJ_WORD}, IDS_LEX_STR_63162, L"JS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HJ_DOUBLESTRING,SCE_HJ_SINGLESTRING,SCE_HJ_STRINGEOL,SCE_HJ_TEMPLATELITERAL)}, IDS_LEX_STR_63164, L"JS String", L"fore:#008000", L"" },
        { {SCE_HJ_REGEX}, IDS_LEX_STR_63166, L"JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_HJ_NUMBER}, IDS_LEX_STR_63161, L"JS Number", L"fore:#FF0000", L"" },
        { {SCE_HJ_SYMBOLS}, IDS_LEX_STR_63165, L"JS Symbols", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_HJA_DEFAULT,SCE_HJA_START,0,0)}, IDS_LEX_STR_63167, L"ASP JS Default", L"", L"" },
        { {MULTI_STYLE(SCE_HJA_COMMENT,SCE_HJA_COMMENTLINE,SCE_HJA_COMMENTDOC,0)}, IDS_LEX_STR_63168, L"ASP JS Comment", L"fore:#646464", L"" },
        { {SCE_HJA_KEYWORD}, IDS_LEX_STR_63171, L"ASP JS Keyword", L"bold; fore:#A46000", L"" },
        { {SCE_HJA_WORD}, IDS_LEX_STR_63170, L"ASP JS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HJA_DOUBLESTRING,SCE_HJA_SINGLESTRING,SCE_HJA_STRINGEOL,SCE_HJA_TEMPLATELITERAL)}, IDS_LEX_STR_63172, L"ASP JS String", L"fore:#008000", L"" },
        { {SCE_HJA_REGEX}, IDS_LEX_STR_63174, L"ASP JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { {SCE_HJA_NUMBER}, IDS_LEX_STR_63169, L"ASP JS Number", L"fore:#FF0000", L"" },
        { {SCE_HJA_SYMBOLS}, IDS_LEX_STR_63173, L"ASP JS Symbols", L"fore:#B000B0", L"" },
        { {MULTI_STYLE(SCE_HB_DEFAULT,SCE_HB_START,0,0)}, IDS_LEX_STR_63175, L"VBS Default", L"", L"" },
        { {SCE_HB_COMMENTLINE}, IDS_LEX_STR_63176, L"VBS Comment", L"fore:#646464", L"" },
        { {SCE_HB_WORD}, IDS_LEX_STR_63178, L"VBS Keyword", L"bold; fore:#B000B0", L"" },
        { {SCE_HB_IDENTIFIER}, IDS_LEX_STR_63180, L"VBS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HB_STRING,SCE_HB_STRINGEOL,0,0)}, IDS_LEX_STR_63179, L"VBS String", L"fore:#008000", L"" },
        { {SCE_HB_NUMBER}, IDS_LEX_STR_63177, L"VBS Number", L"fore:#FF0000", L"" },
        { {MULTI_STYLE(SCE_HBA_DEFAULT,SCE_HBA_START,0,0)}, IDS_LEX_STR_63181, L"ASP VBS Default", L"", L"" },
        { {SCE_HBA_COMMENTLINE}, IDS_LEX_STR_63182, L"ASP VBS Comment", L"fore:#646464", L"" },
        { {SCE_HBA_WORD}, IDS_LEX_STR_63184, L"ASP VBS Keyword", L"bold; fore:#B000B0", L"" },
        { {SCE_HBA_IDENTIFIER}, IDS_LEX_STR_63186, L"ASP VBS Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_HBA_STRING,SCE_HBA_STRINGEOL,0,0)}, IDS_LEX_STR_63185, L"ASP VBS String", L"fore:#008000", L"" },
        { {SCE_HBA_NUMBER}, IDS_LEX_STR_63183, L"ASP VBS Number", L"fore:#FF0000", L"" },
        EDITLEXER_SENTINEL
    }
};

