# Created by DepGen.py. To recreate, run DepGen.py.
$(DIR_O)/Lexilla.obj: \
	../src/Lexilla.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../lexlib/LexerModule.h \
	../lexlib/CatalogueModules.h
$(DIR_O)/Accessor.obj: \
	../lexlib/Accessor.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h
$(DIR_O)/CharacterCategory.obj: \
	../lexlib/CharacterCategory.cxx \
	../lexlib/CharacterCategory.h
$(DIR_O)/CharacterSet.obj: \
	../lexlib/CharacterSet.cxx \
	../lexlib/CharacterSet.h
$(DIR_O)/DefaultLexer.obj: \
	../lexlib/DefaultLexer.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/LexerModule.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/InList.obj: \
	../lexlib/InList.cxx \
	../lexlib/InList.h \
	../lexlib/CharacterSet.h
$(DIR_O)/LexAccessor.obj: \
	../lexlib/LexAccessor.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../lexlib/LexAccessor.h \
	../lexlib/CharacterSet.h
$(DIR_O)/LexerBase.obj: \
	../lexlib/LexerBase.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/LexerModule.h \
	../lexlib/LexerBase.h
$(DIR_O)/LexerModule.obj: \
	../lexlib/LexerModule.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/LexerModule.h \
	../lexlib/LexerBase.h \
	../lexlib/LexerSimple.h
$(DIR_O)/LexerSimple.obj: \
	../lexlib/LexerSimple.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/LexerModule.h \
	../lexlib/LexerBase.h \
	../lexlib/LexerSimple.h
$(DIR_O)/PropSetSimple.obj: \
	../lexlib/PropSetSimple.cxx \
	../lexlib/PropSetSimple.h
$(DIR_O)/StyleContext.obj: \
	../lexlib/StyleContext.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h
$(DIR_O)/WordList.obj: \
	../lexlib/WordList.cxx \
	../lexlib/WordList.h \
	../lexlib/CharacterSet.h
$(DIR_O)/LexAsm.obj: \
	../lexers/LexAsm.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexAU3.obj: \
	../lexers/LexAU3.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexAVS.obj: \
	../lexers/LexAVS.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexBash.obj: \
	../lexers/LexBash.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/StringCopy.h \
	../lexlib/InList.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/SubStyles.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexBatch.obj: \
	../lexers/LexBatch.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/InList.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexCmake.obj: \
	../lexers/LexCmake.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexCoffeeScript.obj: \
	../lexers/LexCoffeeScript.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexConf.obj: \
	../lexers/LexConf.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexCPP.obj: \
	../lexers/LexCPP.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/StringCopy.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/SparseState.h \
	../lexlib/SubStyles.h
$(DIR_O)/LexCSS.obj: \
	../lexers/LexCSS.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexD.obj: \
	../lexers/LexD.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexDart.obj: \
	../lexers/LexDart.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexDiff.obj: \
	../lexers/LexDiff.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexFortran.obj: \
	../lexers/LexFortran.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexHTML.obj: \
	../lexers/LexHTML.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/InList.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/SubStyles.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexInno.obj: \
	../lexers/LexInno.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexJulia.obj: \
	../lexers/LexJulia.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/StringCopy.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/CharacterCategory.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexKix.obj: \
	../lexers/LexKix.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexLaTeX.obj: \
	../lexers/LexLaTeX.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/DefaultLexer.h \
	../lexlib/LexerBase.h
$(DIR_O)/LexLua.obj: \
	../lexers/LexLua.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/SubStyles.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexMake.obj: \
	../lexers/LexMake.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexMarkdown.obj: \
	../lexers/LexMarkdown.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexMatlab.obj: \
	../lexers/LexMatlab.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexNim.obj: \
	../lexers/LexNim.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/StringCopy.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexNsis.obj: \
	../lexers/LexNsis.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexNull.obj: \
	../lexers/LexNull.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexPascal.obj: \
	../lexers/LexPascal.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexPerl.obj: \
	../lexers/LexPerl.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexPowerShell.obj: \
	../lexers/LexPowerShell.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexProps.obj: \
	../lexers/LexProps.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexPython.obj: \
	../lexers/LexPython.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/CharacterCategory.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/SubStyles.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexR.obj: \
	../lexers/LexR.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexRegistry.obj: \
	../lexers/LexRegistry.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexRuby.obj: \
	../lexers/LexRuby.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexRust.obj: \
	../lexers/LexRust.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/PropSetSimple.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexSQL.obj: \
	../lexers/LexSQL.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/SparseState.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexTCL.obj: \
	../lexers/LexTCL.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexTOML.obj: \
	../lexers/LexTOML.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexVB.obj: \
	../lexers/LexVB.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h
$(DIR_O)/LexVHDL.obj: \
	../lexers/LexVHDL.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexYAML.obj: \
	../lexers/LexYAML.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h
$(DIR_O)/LexAHK.obj: \
	../lexers_x/LexAHK.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../lexlib/StringCopy.h \
	../lexlib/PropSetSimple.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/LexerModule.h \
	../lexlib/DefaultLexer.h \
	../lexlib/OptionSet.h \
	../lexlib/WordList.h \
	../lexers_x/CharSetX.h \
	../lexlib/CharacterSet.h \
	../lexers_x/SciXLexer.h \
	../include/SciLexer.h
$(DIR_O)/LexCSV.obj: \
	../lexers_x/LexCSV.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../lexlib/StringCopy.h \
	../lexlib/PropSetSimple.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/LexerModule.h \
	../lexlib/DefaultLexer.h \
	../lexlib/OptionSet.h \
	../lexlib/WordList.h \
	../lexers_x/CharSetX.h \
	../lexlib/CharacterSet.h \
	../lexers_x/SciXLexer.h \
	../include/SciLexer.h
$(DIR_O)/LexerUtils.obj: \
	../lexers_x/LexerUtils.cxx \
	../../scintilla/include/Scintilla.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/ILexer.h \
	../lexlib/LexAccessor.h \
	../lexers_x/CharSetX.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexers_x/StringUtils.h \
	../lexers_x/LexerUtils.h
$(DIR_O)/LexJSON.obj: \
	../lexers_x/LexJSON.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/StyleContext.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/DefaultLexer.h \
	../lexers_x/CharSetX.h \
	../lexlib/CharacterSet.h \
	../lexers_x/SciXLexer.h \
	../include/SciLexer.h
$(DIR_O)/LexKotlin.obj: \
	../lexers_x/LexKotlin.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../lexlib/StringCopy.h \
	../lexlib/PropSetSimple.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/LexerModule.h \
	../lexlib/DefaultLexer.h \
	../lexlib/OptionSet.h \
	../lexlib/WordList.h \
	../lexers_x/CharSetX.h \
	../lexlib/CharacterSet.h \
	../lexers_x/SciXLexer.h \
	../include/SciLexer.h \
	../lexers_x/StringUtils.h \
	../lexers_x/LexerUtils.h
$(DIR_O)/LexVerilog.obj: \
	../lexers_x/LexVerilog.cxx \
	../../scintilla/include/ILexer.h \
	../../scintilla/include/Sci_Position.h \
	../../scintilla/include/Scintilla.h \
	../include/SciLexer.h \
	../lexers_x/SciXLexer.h \
	../lexlib/WordList.h \
	../lexlib/LexAccessor.h \
	../lexlib/Accessor.h \
	../lexlib/StyleContext.h \
	../lexlib/CharacterSet.h \
	../lexlib/LexerModule.h \
	../lexlib/OptionSet.h \
	../lexlib/SubStyles.h \
	../lexlib/DefaultLexer.h
