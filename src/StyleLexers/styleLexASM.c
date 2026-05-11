#include "StyleLexers.h"

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_ASM =
{
    "aaa aad aam aas adc add and arpl bound bsf bsr bswap bt btc btr bts call cbw cdq cflush clc cld cli clts "
    "cmc cmova cmovae cmovb cmovbe cmovc cmove cmovg cmovge cmovl cmovle cmovna cmovnae cmovnb cmovnbe cmovnc "
    "cmovne cmovng cmovnge cmovnl cmovnle cmovno cmovnp cmovns cmovnz cmovo cmovp cmovpe cmovpo cmovs cmovz "
    "cmp cmps cmpsb cmpsd cmpsq cmpsw cmpxchg cmpxchg486 cmpxchg8b cpuid cwd cwde daa das dec div emms enter "
    "esc femms hlt ibts icebp idiv imul in inc ins insb insd insw int int01 int03 int1 int3 into invd invlpg "
    "iret iretd iretdf iretf iretw ja jae jb jbe jc jcxz je jecxz jg jge jl jle jmp jna jnae jnb jnbe jnc jne "
    "jng jnge jnl jnle jno jnp jns jnz jo jp jpe jpo js jz lahf lar lds lea leave les lfs lgdt lgs lidt lldt "
    "lmsw loadall loadall286 lock lods lodsb lodsd lodsq lodsw loop loopd loope looped loopew loopne loopned "
    "loopnew loopnz loopnzd loopnzw loopw loopz loopzd loopzw lsl lss ltr mov movs movsb movsd movsq movsw "
    "movsx movsxd movzx mul neg nop not or out outs outsb outsd outsw pop popa popad popaw popf popfd popfw "
    "push pusha pushad pushaw pushd pushf pushfd pushfw pushw rcl rcr rdmsr rdpmc rdshr rdtsc rep repe repne "
    "repnz repz ret retf retn rol ror rsdc rsldt rsm rsts sahf sal salc sar sbb scas scasb scasd scasq scasw "
    "seta setae setb setbe setc sete setg setge setl setle setna setnae setnb setnbe setnc setne setng setnge "
    "setnl setnle setno setnp setns setnz seto setp setpe setpo sets setz sgdt shl shld shr shrd sidt sldt smi "
    "smint smintold smsw stc std sti stos stosb stosd stosq stosw str sub svdc svldt svts syscall sysenter "
    "sysexit sysret test ud0 ud1 ud2 umov verr verw wait wbinvd wrmsr wrshr xadd xbts xchg xlat xlatb xor"
    " andn bextr blsi blsmk blsr bzhi cdqe cmpxchg16b cqo jrcxz llwpcb lwpins lwpval lzcnt movbe mulx pclmulqdq pdep pext popfq pushfq rdrand rdtscp rorx sarx shlx shrx slwpcb tzcnt xgetbv",
    "f2xm1 fabs fadd faddp fbld fbstp fchs fclex fcmovb fcmovbe fcmove fcmovnb fcmovnbe fcmovne fcmovnu fcmovu "
    "fcom fcomi fcomip fcomp fcompp fcos fdecstp fdisi fdiv fdivp fdivr fdivrp feni ffree ffreep fiadd ficom "
    "ficomp fidiv fidivr fild fimul fincstp finit fist fistp fisub fisubr fld fld1 fldcw fldenv fldenvd "
    "fldenvw fldl2e fldl2t fldlg2 fldln2 fldpi fldz fmul fmulp fnclex fndisi fneni fninit fnop fnsave fnsaved "
    "fnsavew fnstcw fnstenv fnstenvd fnstenvw fnstsw fpatan fprem fprem1 fptan frndint frstor frstord frstorw "
    "fsave fsaved fsavew fscale fsetpm fsin fsincos fsqrt fst fstcw fstenv fstenvd fstenvw fstp fstsw fsub "
    "fsubp fsubr fsubrp ftst fucom fucomp fucompp fwait fxam fxch fxtract fyl2x fyl2xp1"
    " fucomi fucomip",
    "ah al ax bh bl bp bx ch cl cr0 cr2 cr3 cr4 cs cx dh di dl dr0 dr1 dr2 dr3 dr6 dr7 ds dx eax ebp ebx ecx edi "
    "edx eip es esi esp fs gs mm0 mm1 mm2 mm3 mm4 mm5 mm6 mm7 r10 r10b r10d r10w r11 r11b r11d r11w r12 r12b "
    "r12d r12w r13 r13b r13d r13w r14 r14b r14d r14w r15 r15b r15d r15w r8 r8b r8d r8w r9 r9b r9d r9w rax rbp "
    "rbx rcx rdi rdx rip rsi rsp si sp ss st st0 st1 st2 st3 st4 st5 st6 st7 tr3 tr4 tr5 tr6 tr7 xmm0 xmm1 "
    "xmm10 xmm11 xmm12 xmm13 xmm14 xmm15 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7 xmm8 xmm9 ymm0 ymm1 ymm10 ymm11 ymm12 "
    "ymm13 ymm14 ymm15 ymm2 ymm3 ymm4 ymm5 ymm6 ymm7 ymm8 ymm9"
    " bpl dil sil",
    "%arg %assign %define %elif %elifctk %elifdef %elifid %elifidn %elifidni %elifmacro %elifnctk %elifndef "
    "%elifnid %elifnidn %elifnidni %elifnmacro %elifnnum %elifnstr %elifnum %elifstr %else %endif %endmacro "
    "%endrep %error %exitrep %iassign %idefine %if %ifctk %ifdef %ifid %ifidn %ifidni %ifmacro %ifnctk %ifndef "
    "%ifnid %ifnidn %ifnidni %ifnmacro %ifnnum %ifnstr %ifnum %ifstr %imacro %include %line %local %macro %out "
    "%pop %push %rep %repl %rotate %stacksize %strlen %substr %undef %xdefine %xidefine .186 .286 .286c .286p "
    ".287 .386 .386c .386p .387 .486 .486p .8086 .8087 .alpha .break .code .const .continue .cref .data .data? "
    ".dosseg .else .elseif .endif .endw .err .err1 .err2 .errb .errdef .errdif .errdifi .erre .erridn .erridni "
    ".errnb .errndef .errnz .exit .fardata .fardata? .if .lall .lfcond .list .listall .listif .listmacro "
    ".listmacroall .model .msfloat .no87 .nocref .nolist .nolistif .nolistmacro .radix .repeat .sall .seq "
    ".sfcond .stack .startup .tfcond .type .until .untilcxz .while .xall .xcref .xlist absolute alias align "
    "alignb assume at bits catstr comm comment common cpu db dd df dosseg dq dt dup dw echo else elseif "
    "elseif1 elseif2 elseifb elseifdef elseifdif elseifdifi elseife elseifidn elseifidni elseifnb elseifndef "
    "end endif endm endp ends endstruc eq equ even exitm export extern externdef extrn for forc ge global goto "
    "group gt high highword iend if if1 if2 ifb ifdef ifdif ifdifi ife ifidn ifidni ifnb ifndef import incbin "
    "include includelib instr invoke irp irpc istruc label le length lengthof local low lowword lroffset lt "
    "macro mask mod name ne offset opattr option org page popcontext proc proto ptr public purge pushcontext "
    "record repeat rept resb resd resq rest resw section seg segment short size sizeof sizestr struc struct "
    "substr subtitle subttl textequ this times title type typedef union use16 use32 while width"
    " %+ %00 %clear %defstr %deftok %depend %elifctx %elifempty %elifenv %elifn %elifnctx %elifnempty %elifnenv %elifntoken %eliftoken %endwhile %exitmacro %exitwhile %fatal %final %idefstr %ideftok %ifctx %ifempty %ifenv %ifn %ifnctx %ifnempty %ifnenv %ifntoken %iftoken %ixdefine %pathsearch %rmacro %strcat %unmacro %use %warning %while ___nasm_patchlevel__ __alignmode__ __bits__ __date__ __date_num__ __file__ __float128h__ __float128l__ __float16__ __float32__ __float64__ __float80e__ __float80m__ __float8__ __float__ __float_daz__ __float_round__ __infinity__ __line__ __nan__ __nasm_major__ __nasm_minor__ __nasm_subminor__ __nasm_ver__ __nasm_version_id__ __output_format__ __pass__ __posix_time__ __qnan__ __sect__ __snan__ __time__ __time_num__ __use_altreg__ __use_fp__ __use_smartalign__ __utc_date__ __utc_date_num__ __utc_time__ __utc_time_num__ __utf16__ __utf32__ alloc altreg bss class code data default do dy exec flat64 float imagebase noalloc nobits noexec nowrite osabi overlay progbits rdata reso resy safeseh sectalign smartalign stack start tls use64 write",
    "$ $$ %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 .bss .data .text ? @b @f a16 a32 abs addr all assumes at basic byte c "
    "carry? casemap common compact cpu dotname dword emulator epilogue error export expr16 expr32 far far16 "
    "far32 farstack flat forceframe fortran fword huge language large listing ljmp loadds m510 medium memory "
    "near near16 near32 nearstack nodotname noemulator nokeyword noljmp nom510 none nonunique nooldmacros "
    "nooldstructs noreadonly noscoped nosignextend nosplit nothing notpublic o16 o32 oldmacros oldstructs "
    "os_dos overflow? para parity? pascal private prologue qword radix readonly real10 real4 real8 req sbyte "
    "scoped sdword seq setif2 sign? small smallstack stdcall sword syscall tbyte tiny use16 use32 uses vararg "
    "word wrt zero?"
    " a64 o64 oword rel strict tword yword",
    "addpd addps addsd addss andnpd andnps andpd andps blendpd blendps blendvpd blendvps cmpeqpd cmpeqps cmpeqsd "
    "cmpeqss cmplepd cmpleps cmplesd cmpless cmpltpd cmpltps cmpltsd cmpltss cmpnepd cmpneps cmpnesd cmpness "
    "cmpnlepd cmpnleps cmpnlesd cmpnless cmpnltpd cmpnltps cmpnltsd cmpnltss cmpordpd cmpordps cmpordsd "
    "cmpordss cmpunordpd cmpunordps cmpunordsd cmpunordss comisd comiss crc32 cvtdq2pd cvtdq2ps cvtpd2dq "
    "cvtpd2pi cvtpd2ps cvtpi2pd cvtpi2ps cvtps2dq cvtps2pd cvtps2pi cvtsd2si cvtsd2ss cvtsi2sd cvtsi2ss "
    "cvtss2sd cvtss2si cvttpd2dq cvttpd2pi cvttps2dq cvttps2pi cvttsd2si cvttss2si divpd divps divsd divss "
    "dppd dpps extractps fxrstor fxsave insertps ldmxscr lfence maskmovdq maskmovdqu maxpd maxps maxss mfence "
    "minpd minps minsd minss movapd movaps movd movdq2q movdqa movdqu movhlps movhpd movhps movlhps movlpd "
    "movlps movmskpd movmskps movntdq movntdqa movnti movntpd movntps movntq movq movq2dq movsd movss movupd "
    "movups mpsadbw mulpd mulps mulsd mulss orpd orps packssdw packsswb packusdw packuswb paddb paddd paddq "
    "paddsb paddsiw paddsw paddusb paddusw paddw pand pandn pause paveb pavgb pavgusb pavgw paxsd pblendvb "
    "pblendw pcmpeqb pcmpeqd pcmpeqq pcmpeqw pcmpestri pcmpestrm pcmpgtb pcmpgtd pcmpgtq pcmpgtw pcmpistri "
    "pcmpistrm pdistib pextrb pextrd pextrq pextrw pf2id pf2iw pfacc pfadd pfcmpeq pfcmpge pfcmpgt pfmax pfmin "
    "pfmul pfnacc pfpnacc pfrcp pfrcpit1 pfrcpit2 pfrsqit1 pfrsqrt pfsub pfsubr phminposuw pi2fd pinsrb pinsrd "
    "pinsrq pinsrw pmachriw pmaddwd pmagw pmaxsb pmaxsd pmaxsw pmaxub pmaxud pmaxuw pminsb pminsd pminsw "
    "pminub pminud pminuw pmovmskb pmovsxbd pmovsxbq pmovsxbw pmovsxdq pmovsxwd pmovsxwq pmovzxbd pmovzxbq "
    "pmovzxbw pmovzxdq pmovzxwd pmovzxwq pmuldq pmulhriw pmulhrwa pmulhrwc pmulhuw pmulhw pmulld pmullw "
    "pmuludq pmvgezb pmvlzb pmvnzb pmvzb popcnt por prefetch prefetchnta prefetcht0 prefetcht1 prefetcht2 "
    "prefetchw psadbw pshufd pshufhw pshuflw pshufw pslld pslldq psllq psllw psrad psraw psrld psrldq psrlq "
    "psrlw psubb psubd psubq psubsb psubsiw psubsw psubusb psubusw psubw pswapd ptest punpckhbw punpckhdq "
    "punpckhqdq punpckhwd punpcklbw punpckldq punpcklqdq punpcklwd pxor rcpps rcpss roundpd roundps roundsd "
    "roundss rsqrtps rsqrtss sfence shufpd shufps sqrtpd sqrtps sqrtsd sqrtss stmxcsr subpd subps subsd subss "
    "ucomisd ucomiss unpckhpd unpckhps unpcklpd unpcklps xorpd xorps"
    " addsubpd addsubps aesdec aesdeclast aesenc aesenclast aesimc aeskeygenassist cmpneqpd cmpneqps cmpneqsd cmpneqss cmppd cmpps cmpss extrq fisttp haddpd haddps hsubpd hsubps insertq lddqu ldmxcsr maskmovq maxsd montmul movddup movntsd movntss movshdup movsldup pabsb pabsd pabsw palignr pfrcpv pfrsqrtv phaddd phaddsw phaddw phsubd phsubsw phsubw pi2fw pmaddubsw pmulhrsw pmulhrw pshufb psignb psignd psignw vaddpd vaddps vaddsd vaddss vaddsubpd vaddsubps vaesdec vaesdeclast vaesenc vaesenclast vaesimc vaeskeygenassist vandnpd vandnps vandpd vandps vblendpd vblendps vblendvpd vblendvps vbroadcastf128 vbroadcasti128 vbroadcastsd vbroadcastss vcmpeq_ospd vcmpeq_osps vcmpeq_ossd vcmpeq_osss vcmpeq_uqpd vcmpeq_uqps vcmpeq_uqsd vcmpeq_uqss vcmpeq_uspd vcmpeq_usps vcmpeq_ussd vcmpeq_usss vcmpeqpd vcmpeqps vcmpeqsd vcmpeqss vcmpfalse_ospd vcmpfalse_osps vcmpfalse_ossd vcmpfalse_osss vcmpfalsepd vcmpfalseps vcmpfalsesd vcmpfalsess vcmpge_oqpd vcmpge_oqps vcmpge_oqsd vcmpge_oqss vcmpgepd vcmpgeps vcmpgesd vcmpgess vcmpgt_oqpd vcmpgt_oqps vcmpgt_oqsd vcmpgt_oqss vcmpgtpd vcmpgtps vcmpgtsd vcmpgtss vcmple_oqpd vcmple_oqps vcmple_oqsd vcmple_oqss vcmplepd vcmpleps vcmplesd vcmpless vcmplt_oqpd vcmplt_oqps vcmplt_oqsd vcmplt_oqss vcmpltpd vcmpltps vcmpltsd vcmpltss vcmpneq_oqpd vcmpneq_oqps vcmpneq_oqsd vcmpneq_oqss vcmpneq_ospd vcmpneq_osps vcmpneq_ossd vcmpneq_osss vcmpneq_uspd vcmpneq_usps vcmpneq_ussd vcmpneq_usss vcmpneqpd vcmpneqps vcmpneqsd vcmpneqss vcmpnge_uqpd vcmpnge_uqps vcmpnge_uqsd vcmpnge_uqss vcmpngepd vcmpngeps vcmpngesd vcmpngess vcmpngt_uqpd vcmpngt_uqps vcmpngt_uqsd vcmpngt_uqss vcmpngtpd vcmpngtps vcmpngtsd vcmpngtss vcmpnle_uqpd vcmpnle_uqps vcmpnle_uqsd vcmpnle_uqss vcmpnlepd vcmpnleps vcmpnlesd vcmpnless vcmpnlt_uqpd vcmpnlt_uqps vcmpnlt_uqsd vcmpnlt_uqss vcmpnltpd vcmpnltps vcmpnltsd vcmpnltss vcmpord_spd vcmpord_sps vcmpord_ssd vcmpord_sss vcmpordpd vcmpordps vcmpordsd vcmpordss vcmppd vcmpps vcmpsd vcmpss vcmptrue_uspd vcmptrue_usps vcmptrue_ussd vcmptrue_usss vcmptruepd vcmptrueps vcmptruesd vcmptruess vcmpunord_spd vcmpunord_sps vcmpunord_ssd vcmpunord_sss vcmpunordpd vcmpunordps vcmpunordsd vcmpunordss vcomisd vcomiss vcvtdq2pd vcvtdq2ps vcvtpd2dq vcvtpd2pi vcvtpd2ps vcvtph2ps vcvtpi2pd vcvtpi2ps vcvtps2dq vcvtps2pd vcvtps2ph vcvtps2pi vcvtsd2si vcvtsd2ss vcvtsi2sd vcvtsi2ss vcvtss2sd vcvtss2si vcvttpd2dq vcvttpd2pi vcvttps2dq vcvttps2pi vcvttsd2si vcvttss2si vdivpd vdivps vdivsd vdivss vdppd vdpps vextractf128 vextracti128 vextractps vfmadd132pd vfmadd132ps vfmadd132sd vfmadd132ss vfmadd213pd vfmadd213ps vfmadd213sd vfmadd213ss vfmadd231pd vfmadd231ps vfmadd231sd vfmadd231ss vfmaddpd vfmaddps vfmaddsd vfmaddss vfmaddsub132pd vfmaddsub132ps vfmaddsub213pd vfmaddsub213ps vfmaddsub231pd vfmaddsub231ps vfmaddsubpd vfmaddsubps vfmsub132pd vfmsub132ps vfmsub132sd vfmsub132ss vfmsub213pd vfmsub213ps vfmsub213sd vfmsub213ss vfmsub231pd vfmsub231ps vfmsub231sd vfmsub231ss vfmsubadd132pd vfmsubadd132ps vfmsubadd213pd vfmsubadd213ps vfmsubadd231pd vfmsubadd231ps vfmsubaddpd vfmsubaddps vfmsubpd vfmsubps vfmsubsd vfmsubss vfnmadd132pd vfnmadd132ps vfnmadd132sd vfnmadd132ss vfnmadd213pd vfnmadd213ps vfnmadd213sd vfnmadd213ss vfnmadd231pd vfnmadd231ps vfnmadd231sd vfnmadd231ss vfnmsub132pd vfnmsub132ps vfnmsub132sd vfnmsub132ss vfnmsub213pd vfnmsub213ps vfnmsub213sd vfnmsub213ss vfnmsub231pd vfnmsub231ps vfnmsub231sd vfnmsub231ss vfrczpd vfrczps vfrczsd vfrczss vgatherdpd vgatherdps vgatherdq vgatherqpd vgatherqps vgatherqq vhaddpd vhaddps vhsubpd vhsubps vinsertf128 vinserti128 vinsertps vlddqu vldmxcsr vmaskmovdqu vmaskmovpd vmaskmovps vmaxpd vmaxps vmaxsd vmaxss vminpd vminps vminsd vminss vmovapd vmovaps vmovd vmovddup vmovdqa vmovdqu vmovhlps vmovhpd vmovhps vmovlhps vmovlpd vmovlps vmovmskpd vmovmskps vmovntdq vmovntdqa vmovntpd vmovntps vmovq vmovsd vmovshdup vmovsldup vmovss vmovupd vmovups vmpsadbw vmulpd vmulps vmulsd vmulss vnfmaddpd vnfmaddps vnfmaddsd vnfmaddss vnfmsubpd vnfmsubps vnfmsubsd vnfmsubss vorpd vorps vpabsb vpabsd vpabsw vpackssdw vpacksswb vpackusdw vpackuswb vpaddb vpaddd vpaddq vpaddsb vpaddsw vpaddusb vpaddusw vpaddw vpalignr vpand vpandn vpavgb vpavgw vpblendd vpblendw vpblendwb vpbroadcastb vpbroadcastd vpbroadcastq vpbroadcastw vpclmulqdq vpcmov vpcmpeqb vpcmpeqd vpcmpeqq vpcmpeqw vpcmpestri vpcmpestrm vpcmpgtb vpcmpgtd vpcmpgtq vpcmpgtw vpcmpistri vpcmpistrm vpcomb vpcomd vpcomq vpcomub vpcomud vpcomuq vpcomuw vpcomw vperlil2pd vperm2f128 vperm2i128 vpermd vpermil2ps vpermilpd vpermilps vpermpd vpermps vpermq vpextrb vpextrd vpextrq vpextrw vpgatherdd vpgatherqd vphaddbd vphaddbq vphaddbw vphaddd vphadddq vphaddsw vphaddubd vphaddubq vphaddubw vphaddudq vphadduwd vphadduwq vphaddw vphaddwd vphaddwq vphminposuw vphsubbw vphsubd vphsubdq vphsubsw vphsubw vphsubwd vpinsrb vpinsrd vpinsrq vpinsrw vpmacsdd vpmacsdqh vpmacsdql vpmacssdd vpmacssdqh vpmacssdql vpmacsswd vpmacssww vpmacswd vpmacsww vpmadcsswd vpmadcswd vpmaddubsw vpmaddwd vpmaskmovd vpmaskmovq vpmaxsb vpmaxsd vpmaxsw vpmaxub vpmaxud vpmaxuw vpminsb vpminsd vpminsw vpminub vpminud vpminuw vpmovmskb vpmovsxbd vpmovsxbq vpmovsxbw vpmovsxdq vpmovsxwd vpmovsxwq vpmovzxbd vpmovzxbq vpmovzxbw vpmovzxdq vpmovzxwd vpmovzxwq vpmuldq vpmulhrsw vpmulhuw vpmulhw vpmulld vpmullw vpmuludq vpor vpperm vprotb vprotd vprotq vprotw vpsadbw vpshab vpshad vpshaq vpshaw vpshlb vpshld vpshlq vpshlw vpshufb vpshufd vpshufhw vpshuflw vpsignb vpsignd vpsignw vpslld vpslldq vpsllq vpsllvd vpsllvq vpsllw vpsrad vpsravd vpsraw vpsrld vpsrldq vpsrlq vpsrlvd vpsrlw vpsubb vpsubd vpsubq vpsubsb vpsubsw vpsubusb vpsubusw vpsubw vptest vpunpckhbw vpunpckhdq vpunpckhqdq vpunpckhwd vpunpcklbw vpunpckldq vpunpcklqdq vpunpcklwd vpxor vrcpps vrcpss vroundpd vroundps vroundsd vroundss vrsqrtps vrsqrtss vshufpd vshufps vsqrtpd vsqrtps vsqrtsd vsqrtss vstmxcsr vsubpd vsubps vsubsd vsubss vtestpd vtestps vucomisd vucomiss vunpckhpd vunpckhps vunpcklpd vunpcklps vxorpd vxorps vzeroall vzeroupper xcryptcbc xcryptcfb xcryptctr xcryptecb xcryptofb xsha1 xsha256 xstore",
    // User slot 6
    "",
    // User slot 7
    "",
    NULL,
};


EDITLEXER lexASM =
{
    SCLEX_ASM, "asm", IDS_LEX_ASM_SCR, L"Assembly Script", L"asm; s; sx; inc; a51", L"",
    &KeyWords_ASM, {
        { {STYLE_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        //{ {SCE_ASM_DEFAULT}, IDS_LEX_STR_Default, L"Default", L"", L"" },
        { {MULTI_STYLE(SCE_ASM_COMMENT,SCE_ASM_COMMENTBLOCK,0,0)}, IDS_LEX_STR_Comment, L"Comment", L"fore:#008000", L"" },
        { {SCE_ASM_IDENTIFIER}, IDS_LEX_STR_Identifier, L"Identifier", L"", L"" },
        { {MULTI_STYLE(SCE_ASM_STRING,SCE_ASM_CHARACTER,SCE_ASM_STRINGEOL,0)}, IDS_LEX_STR_String, L"String", L"fore:#008000", L"" },
        { {SCE_ASM_NUMBER}, IDS_LEX_STR_Number, L"Number", L"fore:#FF0000", L"" },
        { {SCE_ASM_OPERATOR}, IDS_LEX_STR_Operator, L"Operator", L"fore:#0A246A", L"" },
        { {SCE_ASM_CPUINSTRUCTION}, IDS_LEX_STR_63206, L"CPU Instruction", L"fore:#0A246A", L"" },
        { {SCE_ASM_MATHINSTRUCTION}, IDS_LEX_STR_63207, L"FPU Instruction", L"fore:#0A246A", L"" },
        { {SCE_ASM_EXTINSTRUCTION}, IDS_LEX_STR_63210, L"Extended Instruction", L"fore:#0A246A", L"" },
        { {SCE_ASM_DIRECTIVE}, IDS_LEX_STR_63203, L"Directive", L"fore:#0A246A", L"" },
        { {SCE_ASM_DIRECTIVEOPERAND}, IDS_LEX_STR_63209, L"Directive Operand", L"fore:#0A246A", L"" },
        { {SCE_ASM_REGISTER}, IDS_LEX_STR_Register, L"Register", L"fore:#FF8000", L"" },
        EDITLEXER_SENTINEL
    }
};

