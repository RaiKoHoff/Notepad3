#include "TestEditSupport.h"
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { printf("Failed at line %d: %s\n", __LINE__, #expr); return 1; } } while (0)

static char events[32];
static size_t eventCount;
static bool selectionEmpty;
static DocPos selectedStart, selectedEnd;
static void Event(char c) { events[eventCount++] = c; events[eventCount] = '\0'; }
LONG BeginUndoActionSelection(void) { Event('B'); return 42; }
void EndUndoActionSelection(LONG token) { if (token != 42) abort(); Event('E'); }
bool TestSelectionEmpty(void) { Event('I'); return selectionEmpty; }
DocPos TestSelectionStart(void) { Event('S'); return 7; }
void TestReplaceSel(const char* text) { if (strcmp(text, "abc") != 0) abort(); Event('R'); }
DocPos TestCurrentPos(void) { Event('P'); return 10; }
void TestSetSel(DocPos start, DocPos end) { selectedStart = start; selectedEnd = end; Event('T'); }

static int TestEOL(void)
{
    EditFileIOStatus st = { 0 };
    st.iEOLMode = SC_EOL_LF;
    st.eolCount[0] = 9;
    EditDetectEOLMode(NULL, 3, &st);
    CHECK(st.iEOLMode == SC_EOL_LF && st.eolCount[0] == 9);
    EditDetectEOLMode("", 0, &st);
    CHECK(st.eolCount[0] == 9);

    const char mixed[] = { 'a', '\r', '\n', 'b', '\r', 'c', '\n', '\n' };
    EditDetectEOLMode(mixed, sizeof(mixed), &st);
    CHECK(st.iEOLMode == SC_EOL_LF && st.bInconsistentEOLs);
    CHECK(st.eolCount[SC_EOL_CRLF] == 1 && st.eolCount[SC_EOL_CR] == 1 && st.eolCount[SC_EOL_LF] == 2);
    st.iEOLMode = SC_EOL_LF;
    EditDetectEOLMode("\r\n\r", 3, &st);
    CHECK(st.iEOLMode == SC_EOL_CRLF && st.bInconsistentEOLs);
    CHECK(st.eolCount[SC_EOL_CRLF] == 1 && st.eolCount[SC_EOL_CR] == 1 && st.eolCount[SC_EOL_LF] == 0);
    st.iEOLMode = SC_EOL_CR;
    EditDetectEOLMode("\r\n\r", 3, &st);
    CHECK(st.iEOLMode == SC_EOL_CR);
    EditDetectEOLMode("\n", 1, &st);
    CHECK(st.iEOLMode == SC_EOL_LF && !st.bInconsistentEOLs && st.eolCount[SC_EOL_LF] == 1);
    EditDetectEOLMode("abc", 3, &st);
    CHECK(st.iEOLMode == SC_EOL_LF && !st.bInconsistentEOLs && st.eolCount[SC_EOL_LF] == 0);
    return 0;
}

static int TestNumbers(void)
{
    char out[128];
    CHECK(TestModifyDec("009", true, out, sizeof(out)) && strcmp(out, "010") == 0);
    CHECK(TestModifyDec("0", false, out, sizeof(out)) && strcmp(out, "0") == 0);
    CHECK(TestModifyDec("4294967295", true, out, sizeof(out)) && strcmp(out, "4294967295") == 0);
    CHECK(!TestModifyDec("4294967296", true, out, sizeof(out)));
    CHECK(!TestModifyDec("12x", true, out, sizeof(out)));
    CHECK(TestModifyHex("0X00fF", true, out, sizeof(out)) && strcmp(out, "0X0100") == 0);
    CHECK(TestModifyHex("0x00A", true, out, sizeof(out)) && strcmp(out, "0x00B") == 0);
    CHECK(!TestModifyHex("0x", true, out, sizeof(out)));
    CHECK(!TestModifyHex("0x100000000", true, out, sizeof(out)));
    CHECK(TestModifyBin("0B0011", true, out, sizeof(out)) && strcmp(out, "0B0100") == 0);
    CHECK(TestModifyBin("0b11", true, out, sizeof(out)) && strcmp(out, "0b100") == 0);
    CHECK(!TestModifyBin("0b2", true, out, sizeof(out)));
    CHECK(!TestModifyBin("0b11", true, out, 5));
    return 0;
}

static int TestSelection(void)
{
    selectionEmpty = true;
    EditReplaceSelection("abc", false);
    CHECK(strcmp(events, "BISRE") == 0);
    eventCount = 0;
    EditReplaceSelection("abc", true);
    CHECK(strcmp(events, "BISRPTE") == 0 && selectedStart == 7 && selectedEnd == 10);
    eventCount = 0;
    selectionEmpty = false;
    EditReplaceSelection("abc", false);
    CHECK(strcmp(events, "BISRPTE") == 0 && selectedStart == 7 && selectedEnd == 10);
    return 0;
}

static int TestLogicalSort(void)
{
    WCHAR* aligned = (WCHAR*)VirtualAlloc(NULL, 65536, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    CHECK(aligned != NULL && ((uintptr_t)aligned & 0xFFFFu) == 0);
    wcscpy_s(aligned, 32, L"item2");
    WCHAR item10[] = L"item10";
    CHECK(TestLogicalCompare(aligned, aligned, item10, item10, false) < 0);
    CHECK(TestLogicalCompare(aligned, aligned, item10, item10, true) > 0);
    CHECK(TestLogicalCompare(NULL, NULL, aligned, aligned, false) < 0);
    CHECK(TestLogicalCompare(aligned, aligned, NULL, NULL, false) > 0);
    CHECK(TestLogicalCompare(NULL, NULL, aligned, aligned, true) > 0);
    CHECK(TestLogicalCompare(NULL, NULL, NULL, NULL, false) == 0);
    CHECK(TestLogicalCompare(L"", L"", aligned, aligned, false) < 0);
    CHECK(TestLogicalCompare(aligned, L"", item10, L"", false) < 0);
    CHECK(TestLogicalCompare(aligned, L"", item10, L"", true) > 0);
    CHECK(TestLogicalCompare(aligned, L"key10", item10, L"key2", false) > 0);
    CHECK(TestLogicalCompare(aligned, aligned, aligned, aligned, false) == 0);
    CHECK(VirtualFree(aligned, 0, MEM_RELEASE));
    return 0;
}

int main(void)
{
    if (TestEOL() || TestNumbers() || TestSelection() || TestLogicalSort()) return 1;
    puts("Edit helper checks passed");
    return 0;
}
