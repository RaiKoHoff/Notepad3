// Scintilla source code edit control
/**
 * @file LexJSON5.cxx
 * @date February 19, 2016
 * @brief Lexer for JSON and JSON-LD formats
 * @author nkmathew
 *
 * The License.txt file describes the conditions under which this software may
 * be distributed.
 *
 */

#include <cstdlib>
#include <cassert>
#include <cctype>
#include <cstdio>

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <functional>

#include "ILexer.h"
#include "Scintilla.h"
#include "WordList.h"
#include "LexAccessor.h"
#include "StyleContext.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "DefaultLexer.h"

#include "CharSetX.h"
#include "SciXLexer.h"

using namespace Scintilla;
using namespace Lexilla;

namespace {

const char *const JSON5WordListDesc[] = {
	"JSON Keywords",
	"JSON-LD Keywords",
	nullptr
};

/**
 * Used to detect compact IRI/URLs in JSON-LD without first looking ahead for the
 * colon separating the prefix and suffix
 *
 * https://www.w3.org/TR/json-ld/#dfn-compact-iri
 */
struct CompactIRI {
	int colonCount;
	bool foundInvalidChar;
	CharacterSet setCompactIRI;
	CompactIRI() {
		colonCount = 0;
		foundInvalidChar = false;
		setCompactIRI = CharacterSet(CharacterSet::setAlpha, "$_-");
	}
	void resetState() noexcept {
		colonCount = 0;
		foundInvalidChar = false;
	}
	void checkChar(int ch) noexcept {
		if (ch == ':') {
			colonCount++;
		} else {
			foundInvalidChar |= !setCompactIRI.Contains(ch);
		}
	}
	[[nodiscard]] bool shouldHighlight() const noexcept {
		return !foundInvalidChar && colonCount == 1;
	}
};

/**
 * Keeps track of escaped characters in strings as per:
 *
 * https://tools.ietf.org/html/rfc7159#section-7
 */
struct EscapeSequence {
	int digitsLeft;
	CharacterSet setHexDigits;
	EscapeSequence() {
		digitsLeft = 0;
		setHexDigits = CharacterSet(CharacterSet::setDigits, "ABCDEFabcdef");
	}
	// Validates the char immediately after '\' per JSON5 §5.5.4.
	// Sets digitsLeft for multi-char escapes (\u, \x); 0 for single-char.
	bool newSequence(int nextChar) noexcept {
		digitsLeft = 0;
		if (nextChar == 'u') {
			digitsLeft = 5;     // u + 4 hex digits
		} else if (nextChar == 'x') {
			digitsLeft = 3;     // x + 2 hex digits
		} else if (nextChar >= '1' && nextChar <= '9') {
			// legacy-octal escapes (\1..\9) are forbidden in JSON5
			return false;
		}
		// any other char is a valid 1-char escape (standard or identity)
		return true;
	}
	[[nodiscard]] bool atEscapeEnd() const noexcept {
		return digitsLeft <= 0;
	}
	[[nodiscard]] bool isInvalidChar(int currChar) const noexcept {
		return !setHexDigits.Contains(currChar);
	}
};

struct OptionsJSON5 {
	bool foldCompact;
	bool fold;
	bool allowComments;
	bool escapeSequence;
	OptionsJSON5() {
		foldCompact = false;
		fold = false;
		allowComments = false;
		escapeSequence = false;
	}
};

struct OptionSetJSON5 : public OptionSet<OptionsJSON5> {
	OptionSetJSON5() {
		DefineProperty("lexer.json5.escape.sequence", &OptionsJSON5::escapeSequence,
					   "Set to 1 to enable highlighting of escape sequences in strings");

		DefineProperty("lexer.json5.allow.comments", &OptionsJSON5::allowComments,
					   "Set to 1 to enable highlighting of line/block comments in JSON");

		DefineProperty("fold.compact", &OptionsJSON5::foldCompact);
		DefineProperty("fold", &OptionsJSON5::fold);
		DefineWordListSets(JSON5WordListDesc);
	}
};

class LexerJSON5 : public DefaultLexer {
	OptionsJSON5 options;
	OptionSetJSON5 optSetJSON5;
	EscapeSequence escapeSeq;
	WordList keywordsJSON5;
	WordList keywordsJSON5_LD;
	CharacterSet setOperators;
	CharacterSet setURL;
	CharacterSet setKeywordJSON5_LD;
	CharacterSet setKeywordJSON5;
	CompactIRI compactIRI;

	static bool IsNextNonWhitespace(LexAccessor &styler, Sci_PositionU start, char ch) {
		Sci_PositionU i = 0;
		while (++i < 60) {
			char const curr = styler.SafeGetCharAt(start+i, '\0');
			char const next = styler.SafeGetCharAt(start+i+1, '\0');
			bool const atEOL = (curr == '\r' && next != '\n') || (curr == '\n');
			if (curr == ch) {
				return true;
			} else if (!isspacechar(curr) || atEOL) {
				return false;
			}
		}
		return false;
	}

	/**
	 * Looks for the colon following the end quote
	 *
	 * Assumes property names of lengths no longer than a 120 characters.
	 * The colon is also expected to be less than 50 spaces after the end
	 * quote for the string to be considered a property name
	 */
	static constexpr bool IsPropChar(int ch) noexcept {
		// JSON5 / ECMAScript IdentifierStart + IdentifierPart (ASCII subset)
		return IsAlphaNumeric(ch) || ch == '$' || ch == '_';
	}

	static bool AtPropertyName(LexAccessor &styler, const Sci_PositionU start, bool bQuoted) {
		Sci_PositionU i = 0;
		bool escaped = false;
		while (++i < 120) {
			char curr = styler.SafeGetCharAt(start+i, '\0');
			if (escaped) {
				escaped = false;
				continue;
			}
			escaped = (curr == '\\');
			if (curr == ':' && !bQuoted) {
				return true;
			} else if ((curr == '"' || curr == '\'') && bQuoted) {
				return IsNextNonWhitespace(styler, start + i, ':');
			} else if (isspacechar(curr) && !bQuoted) {
				return IsNextNonWhitespace(styler, start + i, ':');
			} if (!curr || (!bQuoted && !IsPropChar(curr))) {
				return false;
			}
		}
		return false;
	}

	static bool IsNextWordInList(const WordList &keywordList, const CharacterSet &wordSet,
								 const StyleContext &context, LexAccessor &styler) {
		char word[51];
		const Sci_Position currPos = static_cast<Sci_Position>(context.currentPos);
		int i = 0;
		while (i < 50) {
			const char ch = styler.SafeGetCharAt(currPos + i);
			if (!wordSet.Contains(ch)) {
				break;
			}
			word[i] = ch;
			++i;
		}
		word[i] = '\0';
		return keywordList.InList(word);
	}

	static bool IsJSONNumber(const StyleContext& context, bool bChPrevIsOp) {

		bool numberStart =
			IsADigit(context.ch) && (context.chPrev == '+' ||
				context.chPrev == '-' ||
				context.atLineStart ||
				IsASpace(context.chPrev) ||
				bChPrevIsOp);
		bool hexaStart =
			tolower(context.ch) == 'x' &&
			context.chPrev == '0' &&
			IsADigit(context.chNext, 16);
		bool hexaPart =
			IsADigit(context.ch, 16) &&
			(IsADigit(context.chPrev, 16) || tolower(context.chPrev) == 'x');
		bool exponentPart =
			tolower(context.ch) == 'e' &&
			IsADigit(context.chPrev) &&
			(IsADigit(context.chNext) ||
				context.chNext == '+' ||
				context.chNext == '-');
		bool signPart =
			(context.ch == '-' || context.ch == '+') &&
			((tolower(context.chPrev) == 'e' && IsADigit(context.chNext)) ||
				((IsASpace(context.chPrev) || bChPrevIsOp)
					&& IsADigit(context.chNext)));
		bool adjacentDigit =
			IsADigit(context.ch) && IsADigit(context.chPrev);
		bool afterExponent = IsADigit(context.ch) && tolower(context.chPrev) == 'e';
		bool dotPart = context.ch == '.' &&
			(IsADigit(context.chPrev) || IsADigit(context.chNext));
		bool afterDot = IsADigit(context.ch) && context.chPrev == '.';

		return (
			numberStart ||
			hexaStart || hexaPart ||
			exponentPart ||
			signPart ||
			adjacentDigit ||
			dotPart ||
			afterExponent ||
			afterDot
		);
	}


	public:
	LexerJSON5() :
		DefaultLexer("json5", SCLEX_JSON5),
		setOperators(CharacterSet::setNone, "[{}]:,"),
		setURL(CharacterSet::setAlphaNum, "-._~:/?#[]@!$&'()*+,),="),
		setKeywordJSON5_LD(CharacterSet::setAlpha, ":@"),
		setKeywordJSON5(CharacterSet::setAlpha, "$_+-") {
	}
    virtual ~LexerJSON5() = default;
	int SCI_METHOD Version() const override {
		return lvRelease5;
	}
	void SCI_METHOD Release() override {
		delete this;
	}
	const char *SCI_METHOD PropertyNames() override {
		return optSetJSON5.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) override {
		return optSetJSON5.PropertyType(name);
	}
	const char *SCI_METHOD DescribeProperty(const char *name) override {
		return optSetJSON5.DescribeProperty(name);
	}
	Sci_Position SCI_METHOD PropertySet(const char *key, const char *val) override {
		if (optSetJSON5.PropertySet(&options, key, val)) {
			return 0;
		}
		return -1;
	}
	const char * SCI_METHOD PropertyGet(const char *key) override {
		return optSetJSON5.PropertyGet(key);
	}
	Sci_Position SCI_METHOD WordListSet(int n, const char *wl) override {
		WordList* wordListN = nullptr;
		switch (n) {
			case 0:
				wordListN = &keywordsJSON5;
				break;
			case 1:
				wordListN = &keywordsJSON5_LD;
				break;
		}
		Sci_Position firstModification = -1;
		if (wordListN) {
			if (wordListN->Set(wl)) {
				firstModification = 0;
			}
		}
		return firstModification;
	}
	void *SCI_METHOD PrivateCall(int, void *) override {
		return nullptr;
	}
	static ILexer5 *LexerFactoryJSON5() {
		return new LexerJSON5;
	}
	const char *SCI_METHOD DescribeWordListSets() override {
		return optSetJSON5.DescribeWordListSets();
	}
	void SCI_METHOD Lex(Sci_PositionU startPos,
								Sci_Position length,
								int initStyle,
								IDocument *pAccess) override;
	void SCI_METHOD Fold(Sci_PositionU startPos,
								 Sci_Position length,
								 int initStyle,
								 IDocument *pAccess) override;
};

void SCI_METHOD LexerJSON5::Lex(Sci_PositionU startPos,
							   Sci_Position length,
							   int initStyle,
							   IDocument *pAccess) {
	LexAccessor styler(pAccess);
	StyleContext context(startPos, length, initStyle, styler);
	int stringStyleBefore = SCE_JSON5_STRING;
	bool doubleQuotCntx = false;
	bool singleQuotCntx = false;
	while (context.More()) {
		switch (context.state) {
			case SCE_JSON5_BLOCKCOMMENT:
				if (context.Match("*/")) {
					context.Forward();
					context.ForwardSetState(SCE_JSON5_DEFAULT);
				}
				break;
			case SCE_JSON5_LINECOMMENT:
				if (context.MatchLineEnd()) {
					context.SetState(SCE_JSON5_DEFAULT);
				}
				break;
			case SCE_JSON5_STRINGEOL:
				if (context.atLineStart) {
					context.SetState(SCE_JSON5_DEFAULT);
				}
				break;
			case SCE_JSON5_ESCAPESEQUENCE:
				escapeSeq.digitsLeft--;
				if (!escapeSeq.atEscapeEnd()) {
					if (escapeSeq.isInvalidChar(context.ch)) {
						context.SetState(SCE_JSON5_ERROR);
					}
					break;
				}
				{
					const bool atCloseDouble = (context.ch == '"' && doubleQuotCntx);
					const bool atCloseSingle = (context.ch == '\'' && singleQuotCntx);
					if (atCloseDouble || atCloseSingle) {
						if (atCloseDouble) { doubleQuotCntx = false; } else { singleQuotCntx = false; }
						context.SetState(stringStyleBefore);
						context.ForwardSetState(SCE_JSON5_DEFAULT);
					} else if (context.ch == '\\') {
						if (!escapeSeq.newSequence(context.chNext)) {
							context.SetState(SCE_JSON5_ERROR);
						}
						context.Forward();
					} else {
						context.SetState(stringStyleBefore);
						if (context.atLineEnd) {
							context.ChangeState(SCE_JSON5_STRINGEOL);
						}
					}
				}
				break;
			case SCE_JSON5_PROPERTYNAME:
				[[fallthrough]];
			case SCE_JSON5_STRING:
				if (context.ch == ':' && !(doubleQuotCntx || singleQuotCntx)) {
					context.SetState(SCE_JSON5_OPERATOR);
				} else if (context.ch == '"' && doubleQuotCntx) {
					if (compactIRI.shouldHighlight()) {
						context.ChangeState(SCE_JSON5_COMPACTIRI);
						context.ForwardSetState(SCE_JSON5_DEFAULT);
						compactIRI.resetState();
					} else {
						context.ForwardSetState(SCE_JSON5_DEFAULT);
					}
					doubleQuotCntx = false;
				}
				else if (context.ch == '\'' && singleQuotCntx) {
					if (compactIRI.shouldHighlight()) {
						context.ChangeState(SCE_JSON5_COMPACTIRI);
						context.ForwardSetState(SCE_JSON5_DEFAULT);
						compactIRI.resetState();
					}
					else {
						context.ForwardSetState(SCE_JSON5_DEFAULT);
					}
					singleQuotCntx = false;
				} else if (context.ch == '\\') {
					// line continuation: JSON5 LineTerminator = LF | CR | LS (U+2028) | PS (U+2029)
					if (context.Match("\\\n")) {
						context.Forward();
						context.ForwardSetState(context.state);
						continue;
					}
					else if (context.Match("\\\r\n")) {
						context.Forward();
						context.Forward();
						context.ForwardSetState(context.state);
						continue;
					}
					else if (context.Match("\\\r")) {
						context.Forward();
						context.ForwardSetState(context.state);
						continue;
					}
					else if (context.Match("\\\xE2\x80\xA8") || context.Match("\\\xE2\x80\xA9")) {
						context.Forward();
						context.Forward();
						context.Forward();
						context.ForwardSetState(context.state);
						continue;
					}
					else {
						stringStyleBefore = context.state;
						if (options.escapeSequence) {
							context.SetState(SCE_JSON5_ESCAPESEQUENCE);
							if (!escapeSeq.newSequence(context.chNext)) {
								context.SetState(SCE_JSON5_ERROR);
							}
						}
						context.Forward();
					}
				} else if (context.atLineEnd) {
					context.ChangeState(SCE_JSON5_STRINGEOL);
				} else if (context.Match("https://") ||
						   context.Match("http://") ||
						   context.Match("ssh://") ||
						   context.Match("git://") ||
						   context.Match("svn://") ||
						   context.Match("ftp://") ||
						   context.Match("mailto:")) {
					// Handle most common URI schemes only
					stringStyleBefore = context.state;
					context.SetState(SCE_JSON5_URI);
				} else if (context.ch == '@') {
					// https://www.w3.org/TR/json-ld/#dfn-keyword
					if (IsNextWordInList(keywordsJSON5_LD, setKeywordJSON5_LD, context, styler)) {
						stringStyleBefore = context.state;
						context.SetState(SCE_JSON5_LDKEYWORD);
					}
				}
				else if (IsPropChar(context.ch)) {
					if (!AtPropertyName(styler, context.currentPos, (doubleQuotCntx || singleQuotCntx))) {
						if (context.state == SCE_JSON5_PROPERTYNAME) {
							context.SetState(SCE_JSON5_ERROR);
						}
					}
				}
				else {
					compactIRI.checkChar(context.ch);
				}
				break;
			case SCE_JSON5_LDKEYWORD:
			case SCE_JSON5_URI:
				if ((!setKeywordJSON5_LD.Contains(context.ch) &&
					 (context.state == SCE_JSON5_LDKEYWORD)) ||
					(!setURL.Contains(context.ch))) {
					context.SetState(stringStyleBefore);
				}
				if ((context.ch == '"' && doubleQuotCntx) || (context.ch == '\'' && singleQuotCntx)) {
					if (context.ch == '"') { doubleQuotCntx = false; } else { singleQuotCntx = false; }
					context.ForwardSetState(SCE_JSON5_DEFAULT);
				} else if (context.ch == '\\') {
					if (options.escapeSequence) {
						context.SetState(SCE_JSON5_ESCAPESEQUENCE);
						if (!escapeSeq.newSequence(context.chNext)) {
							context.SetState(SCE_JSON5_ERROR);
						}
					}
					context.Forward();
				} else if (context.atLineEnd) {
					context.ChangeState(SCE_JSON5_STRINGEOL);
				}
				break;
			case SCE_JSON5_OPERATOR:
			case SCE_JSON5_NUMBER:
				context.SetState(SCE_JSON5_DEFAULT);
				break;
			case SCE_JSON5_ERROR:
				if (context.MatchLineEnd()) {
					context.SetState(SCE_JSON5_DEFAULT);
				}
				break;
			case SCE_JSON5_KEYWORD:
				if (!setKeywordJSON5.Contains(context.ch)) {
					context.SetState(SCE_JSON5_DEFAULT);
				}
				break;
		}

		if (context.state == SCE_JSON5_DEFAULT) {
			if (context.ch == '"') {
				compactIRI.resetState();
				context.SetState(SCE_JSON5_STRING);
				doubleQuotCntx = !singleQuotCntx;
				if (AtPropertyName(styler, context.currentPos, true)) {
					context.SetState(SCE_JSON5_PROPERTYNAME);
				}
			} else if (context.ch == '\'') {
				compactIRI.resetState();
				context.SetState(SCE_JSON5_STRING);
				singleQuotCntx = !doubleQuotCntx;
				if (AtPropertyName(styler, context.currentPos, true)) {
					context.SetState(SCE_JSON5_PROPERTYNAME);
				}
			} else if (setKeywordJSON5.Contains(context.ch)) {
				if (IsNextWordInList(keywordsJSON5, setKeywordJSON5, context, styler)) {
					context.SetState(SCE_JSON5_KEYWORD);
				}
			} else if (setOperators.Contains(context.ch)) {
				context.SetState(SCE_JSON5_OPERATOR);
			} else if (options.allowComments && context.Match("/*")) {
				context.SetState(SCE_JSON5_BLOCKCOMMENT);
				context.Forward();
			} else if (options.allowComments && context.Match("//")) {
				context.SetState(SCE_JSON5_LINECOMMENT);
			}

			if (IsJSONNumber(context, setOperators.Contains(context.chPrev))) {
				context.SetState(SCE_JSON5_NUMBER);
			}
			else if (context.state == SCE_JSON5_DEFAULT) {
				if (IsPropChar(context.ch)) {
					if (AtPropertyName(styler, context.currentPos, (doubleQuotCntx || singleQuotCntx))) {
						context.SetState(SCE_JSON5_PROPERTYNAME);
					}
				}
				else if (context.state == SCE_JSON5_DEFAULT && !IsASpace(context.ch)) {
					context.SetState(SCE_JSON5_ERROR);
				}
			}
		}
		context.Forward();
	}
	context.Complete();
}

void SCI_METHOD LexerJSON5::Fold(Sci_PositionU startPos,
								Sci_Position length,
								int,
								IDocument *pAccess) {
	if (!options.fold) {
		return;
	}
	LexAccessor styler(pAccess);
	Sci_PositionU currLine = styler.GetLine(startPos);
	Sci_PositionU endPos = startPos + length;
	int currLevel = SC_FOLDLEVELBASE;
	if (currLine > 0)
		currLevel = styler.LevelAt(currLine - 1) >> 16;
	int nextLevel = currLevel;
	int visibleChars = 0;
	for (Sci_PositionU i = startPos; i < endPos; i++) {
		char curr = styler.SafeGetCharAt(i);
		char next = styler.SafeGetCharAt(i+1);
		bool atEOL = (curr == '\r' && next != '\n') || (curr == '\n');
		if (styler.StyleAt(i) == SCE_JSON5_OPERATOR) {
			if (curr == '{' || curr == '[') {
				nextLevel++;
			} else if (curr == '}' || curr == ']') {
				nextLevel--;
			}
		}
		if (atEOL || i == (endPos-1)) {
			int level = currLevel | nextLevel << 16;
			if (!visibleChars && options.foldCompact) {
				level |= SC_FOLDLEVELWHITEFLAG;
			} else if (nextLevel > currLevel) {
				level |= SC_FOLDLEVELHEADERFLAG;
			}
			if (level != styler.LevelAt(currLine)) {
				styler.SetLevel(currLine, level);
			}
			currLine++;
			currLevel = nextLevel;
			visibleChars = 0;
		}
		if (!isspacechar(curr)) {
			visibleChars++;
		}
	}
}

}

extern const LexerModule lmJSON5(SCLEX_JSON5, LexerJSON5::LexerFactoryJSON5, "json5", JSON5WordListDesc);
