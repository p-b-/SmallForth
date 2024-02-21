#include <iostream>
#include "PreBuiltWords.h"
#include "ExecState.h"
#include "ForthString.h"
#include "CompileHelper.h"
#include "ForthWord.h"
#include "ForthDict.h"
#include "ReturnStack.h"
#include "InputProcessor.h"

using std::ostream;

void PreBuiltWords::RegisterWords(ForthDict* pDict) {
	InitialiseImmediateWord(pDict, "(", PreBuiltWords::BuiltIn_ParenthesisCommentStart);
	InitialiseImmediateWord(pDict, ")", PreBuiltWords::BuiltIn_ParenthesisCommentEnd);
	InitialiseImmediateWord(pDict, "\\", PreBuiltWords::BuiltIn_LineCommentStart);

	InitialiseWord(pDict, "#boolvar", PreBuiltWords::BuiltIn_ThreadSafeBoolVariable);
	InitialiseWord(pDict, "#intvar", PreBuiltWords::BuiltIn_ThreadSafeIntVariable);
	InitialiseWord(pDict, "exit", PreBuiltWords::BuiltIn_Exit);
	InitialiseWord(pDict, "pushliteral", PreBuiltWords::BuiltIn_PushUpcomingLiteral);

	InitialiseWord(pDict, "exception", PreBuiltWords::BuiltIn_ThrowException);

	// cannot yet execute either of these:
	//  "0 variable_intstate compileState "
	//  ": compileState 0 #intvar ;"
	// as : not yet defined, and neither is variable_intstate
	// ":" needs compileState to work correctly, so manually create it here.  Note, as this is a threadlocal state variable, we start the 
	//  the variable definition of #nextIntVarIndex at 1 not 0.

	//  Note, #compileState has to be first thread-local int, as ExecState::c_compileStateIndex is set to 0
	// Cannot create these as leve-two words, as DOCOL (that executes level 2 words) needs to be able to execute them
	InitialiseWord(pDict, "#compileState", PreBuiltWords::BuiltIn_CompileState);
	//  Note, #postponeState has to be first thread-local bool, as ExecState::c_postponedExecIndex is set to 0
	// Creating this means #nextBoolVarIndex starts at 1, not 0
	InitialiseWord(pDict, "#postponeState", PreBuiltWords::BuiltIn_PostponeState);

	InitialiseWord(pDict, "#insideComment", PreBuiltWords::BuiltIn_InsideCommentState);
	InitialiseWord(pDict, "#insideCommentLine", PreBuiltWords::BuiltIn_InsideCommentLineState);
	InitialiseWord(pDict, "#debugState", PreBuiltWords::BuiltIn_DebugState);

	// TODO Delete this
	//ForthWord* pCompileStateWord = new ForthWord("#compileState", PreBuiltWords::BuiltIn_DoCol);
	//CompileWordIntoWord(pDict, pCompileStateWord, "pushliteral");
	//CompileTypeIntoWord(pCompileStateWord, StackElement_Int);
	//CompileLiteralValueIntoWord(pCompileStateWord, 0);
	//CompileWordIntoWord(pDict, pCompileStateWord, "#intvar");
	//CompileWordIntoWord(pDict, pCompileStateWord, "exit");
	//pCompileStateWord->SetWordVisibility(true);
	//pDict->AddWord(pCompileStateWord);

	// TODO Delete this
	//// Creating this means #nextBoolVarIndex starts at 1, not 0
	//ForthWord* pPostponeStateWord = new ForthWord("#postponeState", PreBuiltWords::BuiltIn_DoCol);
	//CompileWordIntoWord(pDict, pPostponeStateWord, "pushliteral");
	//CompileTypeIntoWord(pPostponeStateWord, StackElement_Int);
	//CompileLiteralValueIntoWord(pPostponeStateWord, 1);
	//CompileWordIntoWord(pDict, pPostponeStateWord, "#boolvar");
	//CompileWordIntoWord(pDict, pPostponeStateWord, "exit");
	//pPostponeStateWord->SetWordVisibility(true);
	//pDict->AddWord(pPostponeStateWord);
	
	InitialiseWord(pDict, "docol", PreBuiltWords::BuiltIn_DoCol);
	InitialiseWord(pDict, "[docol]", PreBuiltWords::BuiltIn_IndirectDoCol);
	InitialiseWord(pDict, "immediate", PreBuiltWords::BuiltIn_Immediate);
	InitialiseWord(pDict, "execute", PreBuiltWords::BuiltIn_Execute);
	InitialiseWord(pDict, "executeonobject", PreBuiltWords::BuiltIn_ExecuteOnObject);
	InitialiseWord(pDict, "jump", PreBuiltWords::BuiltIn_Jump); // ( addr -- )
	InitialiseWord(pDict, "jumpontrue", PreBuiltWords::BuiltIn_JumpOnTrue); // ( addr b -- )
	InitialiseWord(pDict, "jumponfalse", PreBuiltWords::BuiltIn_JumpOnFalse); // ( addr b -- )
	InitialiseWord(pDict, "!inword", PreBuiltWords::BuiltIn_PokeIntegerInWord); // (v addr -- )
	InitialiseWord(pDict, "@", PreBuiltWords::BuiltIn_Peek); // (
	InitialiseWord(pDict, "!", PreBuiltWords::BuiltIn_Poke);
	InitialiseWord(pDict, "fetchliteral", PreBuiltWords::BuiltIn_FetchLiteral);
	InitialiseImmediateWord(pDict, "updateForwardJump", PreBuiltWords::BuiltIn_UpdateForwardJump);

	InitialiseWord(pDict, "reveal", PreBuiltWords::BuiltIn_Reveal);
	InitialiseWord(pDict, "stackreveal", PreBuiltWords::BuiltIn_RevealToStack);
	InitialiseWord(pDict, "create", PreBuiltWords::BuiltIn_Create);
	InitialiseWord(pDict, "word>object", PreBuiltWords::BuiltIn_AddWordToObject);
	InitialiseWord(pDict, "find", PreBuiltWords::BuiltIn_Find);

	// Although these change TLS variables, they cannot be create via forth because the definition for ":" needs them, and we need ":" to define things in forth
	InitialiseImmediateWord(pDict, "]", PreBuiltWords::BuiltIn_StartCompilation);
	InitialiseImmediateWord(pDict, "[", PreBuiltWords::BuiltIn_EndCompilation);
	InitialiseImmediateWord(pDict, "postpone", PreBuiltWords::BuiltIn_Postpone);
	InitialiseWord(pDict, "(postpone)", PreBuiltWords::BuiltIn_PostponePostpone);
	InitialiseImmediateWord(pDict, "postpone=0", PreBuiltWords::BuiltIn_ResetPostponeState);

	InitialiseImmediateWord(pDict, "literal", PreBuiltWords::BuiltIn_Literal);
	InitialiseImmediateWord(pDict, "#literal", PreBuiltWords::BuiltIn_LiteralNoPush);
	InitialiseImmediateWord(pDict, ",", PreBuiltWords::BuiltIn_Literal);
	InitialiseImmediateWord(pDict, "[char]", PreBuiltWords::BuiltIn_CharLiteral);

	InitialiseWord(pDict, "true", PreBuiltWords::BuiltIn_True);
	InitialiseWord(pDict, "false", PreBuiltWords::BuiltIn_False);
	InitialiseWord(pDict, "type", PreBuiltWords::BuiltIn_PushType);

	InitialiseWord(pDict, "dup", PreBuiltWords::BuiltIn_Dup);
	InitialiseWord(pDict, "swap", PreBuiltWords::BuiltIn_Swap);
	InitialiseWord(pDict, "drop", PreBuiltWords::BuiltIn_Drop);
	InitialiseWord(pDict, "over", PreBuiltWords::BuiltIn_Over);
	InitialiseWord(pDict, "rot", PreBuiltWords::BuiltIn_Rot);
	InitialiseWord(pDict, "-rot", PreBuiltWords::BuiltIn_ReverseRot);
	InitialiseWord(pDict, ">r", PreBuiltWords::BuiltIn_PushDataStackToReturnStack);
	InitialiseWord(pDict, "<r", PreBuiltWords::BuiltIn_PushReturnStackToDataStack);
	InitialiseWord(pDict, ">t", PreBuiltWords::BuiltIn_PushDataStackToTempStack);
	InitialiseWord(pDict, "<t", PreBuiltWords::BuiltIn_PushTempStackToDataStack);

	InitialiseWord(pDict, "+", PreBuiltWords::BuiltIn_Add);
	InitialiseWord(pDict, "-", PreBuiltWords::BuiltIn_Subtract);
	InitialiseWord(pDict, "*", PreBuiltWords::BuiltIn_Multiply);
	InitialiseWord(pDict, "/", PreBuiltWords::BuiltIn_Divide);
	InitialiseWord(pDict, "%", PreBuiltWords::BuiltIn_Modulus);
	InitialiseWord(pDict, "<", PreBuiltWords::BuiltIn_LessThan);
	InitialiseWord(pDict, "<=", PreBuiltWords::BuiltIn_LessThanOrEquals);
	InitialiseWord(pDict, "=", PreBuiltWords::BuiltIn_Equals);
	InitialiseWord(pDict, "!=", PreBuiltWords::BuiltIn_NotEquals);
	InitialiseWord(pDict, "<>", PreBuiltWords::BuiltIn_NotEquals);
	InitialiseWord(pDict, ">=", PreBuiltWords::BuiltIn_GreaterThanOrEquals);
	InitialiseWord(pDict, ">", PreBuiltWords::BuiltIn_GreaterThan);

	InitialiseWord(pDict, "not", PreBuiltWords::BuiltIn_Not);
	InitialiseWord(pDict, "and", PreBuiltWords::BuiltIn_And);
	InitialiseWord(pDict, "or", PreBuiltWords::BuiltIn_Or);
	InitialiseWord(pDict, "xor", PreBuiltWords::BuiltIn_Xor);

	InitialiseWord(pDict, "^", PreBuiltWords::BuiltIn_Power);
	InitialiseWord(pDict, "sin", PreBuiltWords::BuiltIn_Sin);
	InitialiseWord(pDict, "cos", PreBuiltWords::BuiltIn_Cos);
	InitialiseWord(pDict, "tan", PreBuiltWords::BuiltIn_Tan);
	InitialiseWord(pDict, "sin-1", PreBuiltWords::BuiltIn_Arcsine);
	InitialiseWord(pDict, "cos-1", PreBuiltWords::BuiltIn_Arccosine);
	InitialiseWord(pDict, "tan-1", PreBuiltWords::BuiltIn_Arctan);
	InitialiseWord(pDict, "sqrt", PreBuiltWords::BuiltIn_Sqrt);

	InitialiseWord(pDict, "depth", PreBuiltWords::BuiltIn_StackSize); // ( a1 .. an -- a1 .. an n )
	InitialiseWord(pDict, "rdepth", PreBuiltWords::BuiltIn_RStackSize); // ( R: a1 .. an -- R: a1 .. an n )
	InitialiseWord(pDict, "tdepth", PreBuiltWords::BuiltIn_TStackSize); // ( T: a1 .. an -- T: a1 .. an n )
	InitialiseWord(pDict, ".", PreBuiltWords::BuiltIn_PrintStackTop);
	InitialiseWord(pDict, "emit", PreBuiltWords::BuiltIn_Emit);
	InitialiseWord(pDict, "`", PreBuiltWords::BuiltIn_WordCFAFromInputStream);
	InitialiseImmediateWord(pDict, "[`]", PreBuiltWords::BuiltIn_WordCFAFromDefinition);
	InitialiseImmediateWord(pDict, "here", PreBuiltWords::BuiltIn_Here);
	InitialiseWord(pDict, "(here)", PreBuiltWords::BuiltIn_Here);
	InitialiseWord(pDict, "forget", PreBuiltWords::BuiltIn_Forget);
	InitialiseWord(pDict, "see", ForthWord::BuiltIn_DescribeWord);

	InitialiseImmediateWord(pDict, "does>", PreBuiltWords::BuiltIn_Does);

	InitialiseImmediateWord(pDict, "\"", PreBuiltWords::BuiltIn_StringLiteral);
	InitialiseWord(pDict, "call", PreBuiltWords::BuiltIn_CallObject);
	InitialiseWord(pDict, "construct", PreBuiltWords::BuiltIn_Construct);
	InitialiseWord(pDict, "define", PreBuiltWords::BuiltIn_DefineObject);

	// Low level words
	InitialiseWord(pDict, "self", PreBuiltWords::PushSelf);
	InitialiseWord(pDict, "#refcount", PreBuiltWords::PushRefCount);
	InitialiseWord(pDict, "quit", PreBuiltWords::Quit);

	// Definitions 
	InitialiseWord(pDict, "allot", PreBuiltWords::BuiltIn_Allot);

	// Strings and string output
	InitialiseWord(pDict, "#s", PreBuiltWords::ToString);

	// Value type manipulation
	InitialiseWord(pDict, "tochar", PreBuiltWords::ToChar);
	InitialiseWord(pDict, "toint", PreBuiltWords::ToInt);
	InitialiseWord(pDict, "tofloat", PreBuiltWords::ToFloat);
	InitialiseWord(pDict, "wordtoint", PreBuiltWords::WordToInt);
	InitialiseWord(pDict, "wordtofloat", PreBuiltWords::WordToFloat);

	InitialiseWord(pDict, "isObject", PreBuiltWords::IsObject);
	InitialiseWord(pDict, "isPter", PreBuiltWords::IsPter);

	// Timer and time
	InitialiseWord(pDict, "elapsedSeconds", PreBuiltWords::BuiltIn_GetHighResolutionTime);
}

void PreBuiltWords::CreateSecondLevelWords(ExecState* pExecState) {
	ForthDict* pDict = pExecState->pDict;

	ForthWord* pDefineWord = new ForthWord(":", PreBuiltWords::BuiltIn_DoCol);
	CompileWordIntoWord(pDict, pDefineWord, "create");
	CompileWordIntoWord(pDict, pDefineWord, "]");
	CompileWordIntoWord(pDict, pDefineWord, "exit");
	pDefineWord->SetWordVisibility(true);
	pDict->AddWord(pDefineWord);

	ForthWord* pEndWordDefinition = new ForthWord(";", PreBuiltWords::BuiltIn_DoCol);
	CompileWordIntoWord(pDict, pEndWordDefinition, "postpone");
	CompileWordIntoWord(pDict, pEndWordDefinition, "exit");
	CompileWordIntoWord(pDict, pEndWordDefinition, "[");
	CompileWordIntoWord(pDict, pEndWordDefinition, "reveal");
	CompileWordIntoWord(pDict, pEndWordDefinition, "does>"); // Enters runtime compile mode for the next word (puts the next word into the CFA)
	CompileWordIntoWord(pDict, pEndWordDefinition, "docol"); // This has to be a built-in, not a second-level word
	CompileWordIntoWord(pDict, pEndWordDefinition, "["); // does> entered compile state, this leaves it
	CompileWordIntoWord(pDict, pEndWordDefinition, "exit");

	pEndWordDefinition->SetImmediate(true);
	pEndWordDefinition->SetWordVisibility(true);
	pDict->AddWord(pEndWordDefinition);
	pEndWordDefinition = nullptr;
	InterpretForth(pExecState, ": constant create postpone #literal reveal postpone does> fetchliteral postpone [ ;");
	InterpretForth(pExecState, ": variable create postpone #literal reveal ;");

	InterpretForth(pExecState, "1 type type variable #compileForType");

	// Set to 2 and 3, as already defined compileState, debugState (int) and postponeState, insideComment, insideCommentLine (bool) manually
	InterpretForth(pExecState, "2 variable #nextIntVarIndex");
	InterpretForth(pExecState, "3 variable #nextBoolVarIndex");

	InterpretForth(pExecState, ": #getNextIntVarIndex #nextIntVarIndex dup @ dup 1 + rot ! ; immediate");
	InterpretForth(pExecState, ": #getNextBoolVarIndex #nextBoolVarIndex dup @ dup 1 + rot ! ; immediate");
	InterpretForth(pExecState, ": variable_intstate \
        create \
          postpone ] \
          postpone #getNextIntVarIndex \
          postpone dup \
          postpone literal \
          #intvar ! \
          (postpone) #intvar \
          (postpone) postpone exit \
          reveal \
          postpone does> docol \
          postpone [ \
        ;");
	InterpretForth(pExecState, ": variable_boolstate \
        create \
          postpone ] \
          postpone #getNextBoolVarIndex \
          postpone dup \
          postpone literal \
          #boolvar ! \
          (postpone) #boolvar \
          (postpone) postpone exit \
          reveal \
          postpone does> docol \
          postpone [ \
        ;");

	InterpretForth(pExecState, "3.1415926535897932384626433832795 constant pi");
	InterpretForth(pExecState, ": radtodeg pi / 180 * ;");
	InterpretForth(pExecState, ": degtorad 180 / pi * ;");

	// Compiles word onto stack
	pEndWordDefinition = new ForthWord(";;", PreBuiltWords::BuiltIn_DoCol);
	CompileWordIntoWord(pDict, pEndWordDefinition, "postpone");
	CompileWordIntoWord(pDict, pEndWordDefinition, "exit");
	CompileWordIntoWord(pDict, pEndWordDefinition, "[");
	CompileWordIntoWord(pDict, pEndWordDefinition, "stackreveal");
	CompileWordIntoWord(pDict, pEndWordDefinition, "#compileForType");
	CompileWordIntoWord(pDict, pEndWordDefinition, "@");
	CompileWordIntoWord(pDict, pEndWordDefinition, "word>object");
	CompileWordIntoWord(pDict, pEndWordDefinition, "does>"); // Enters runtime compile mode for the next word (puts the next word into the CFA)
	CompileWordIntoWord(pDict, pEndWordDefinition, "docol"); // This has to be a built-in, not a second-level word
	CompileWordIntoWord(pDict, pEndWordDefinition, "["); // does> entered compile state, this leaves it
	CompileWordIntoWord(pDict, pEndWordDefinition, "exit");

	pEndWordDefinition->SetImmediate(true);
	pEndWordDefinition->SetWordVisibility(true);
	pDict->AddWord(pEndWordDefinition);

	ForthWord* pDefineWordForObject = new ForthWord("::", PreBuiltWords::BuiltIn_DoCol);
	CompileWordIntoWord(pDict, pDefineWordForObject, "#compileForType");
	CompileWordIntoWord(pDict, pDefineWordForObject, "!");
	CompileWordIntoWord(pDict, pEndWordDefinition, "postpone");
	CompileWordIntoWord(pDict, pDefineWordForObject, "create");
	CompileWordIntoWord(pDict, pEndWordDefinition, "postpone");
	CompileWordIntoWord(pDict, pDefineWordForObject, "]");
	CompileWordIntoWord(pDict, pDefineWordForObject, "exit");
	pDefineWordForObject->SetImmediate(true);
	pDefineWordForObject->SetWordVisibility(true);
	pDict->AddWord(pDefineWordForObject);

	InterpretForth(pExecState, ": startDebugging 1 #debugState ! ; immediate");
	InterpretForth(pExecState, ": stopDebugging 0 #debugState ! ; immediate");


	InterpretForth(pExecState, ": 1+ 1 + ;"); // ( m -- m+1 )
	InterpretForth(pExecState, ": 1- 1 - ;"); // ( m -- m-1 )
	InterpretForth(pExecState, ": .\" postpone \" . ;"); // ( -- ) outputs literal to stdout

	InterpretForth(pExecState, ": cr ( -- ) 10 emit ;"); // (  --  )
	InterpretForth(pExecState, ": '\10' 10 tochar ; ");
	InterpretForth(pExecState, ": pushNewLine '\10' ;");

	InterpretForth(pExecState, ": dup2 dup dup ;"); // ( m -- m m m )
	InterpretForth(pExecState, ": nip swap drop ;"); // ( m n -- n)
	InterpretForth(pExecState, ": tuck swap over ;"); // ( m n -- n m n)
	InterpretForth(pExecState, ": 2dup over over ;"); // ( m n -- m n m n)
	InterpretForth(pExecState, ": 2drop drop drop ;"); // ( m n -- )
	InterpretForth(pExecState, ": 3drop 2drop drop ;"); // ( m n p -- )
	InterpretForth(pExecState, ": rdup <r dup >r >r ;"); // ( R: a -- R: a a)
	InterpretForth(pExecState, ": r@ <r dup >r ;"); // (R : a -- a R: a)
	InterpretForth(pExecState, ": 2>r swap >r >r ;"); // ( a b -- R a b)
	InterpretForth(pExecState, ": 2<r <r <r swap ;"); // ( R a b -- a b)
	InterpretForth(pExecState, ": 2r@ 2<r 2dup 2>r ;"); // ( R a b -- a b R a b)

	InterpretForth(pExecState, ": 2swap rot >t rot <t ;"); // ( a b c d -- c d a b)
	InterpretForth(pExecState, ": 2over >t >t 2dup <t -rot <t -rot ;"); // ( a b c d -- a b c d a b)
	InterpretForth(pExecState, ": 2nip rot drop rot drop ;"); // (a b c d -- c d)
	InterpretForth(pExecState, ": 2tuck 2dup >t >t >t -rot <t -rot <t <t ;"); // (a b c d -- c d a b c d )
	InterpretForth(pExecState, ": 2rot >t >t 2swap <t <t 2swap ;"); // ( a b c d e f -- c d e f a b )
	InterpretForth(pExecState, ": -2rot 2swap >t >t 2swap <t <t ;");  // ( a b c d e f -- e f a b c d )

	InterpretForth(pExecState, ": 3>r -rot 2>r >r ;");  // ( a b c  -- R a b c )
	InterpretForth(pExecState, ": 3<r <r 2<r rot ;");  // ( a b c  -- R a b c )

	// Rely on stack operations above
	InterpretForth(pExecState, ": #if    postpone here dup >r postpone , (postpone) swap (postpone) jumponfalse ; immediate");
	InterpretForth(pExecState, ": #then  postpone updateforwardjump ; immediate");
	InterpretForth(pExecState, ": if     #compileState @ 0 = #if \" Cannot execute IF when not compiling \" exception #then \
								         postpone here dup >r postpone , (postpone) swap (postpone) jumponfalse ; immediate");
	InterpretForth(pExecState, ": then   #compileState @ 0 = #if \" Cannot execute THEN when not compiling \" exception updateforwardjump \
										 postpone updateforwardjump ; immediate");
	InterpretForth(pExecState, "forget #if forget #then");
	InterpretForth(pExecState, ": else   #compileState @ 0 = if \" Cannot execute ELSE when not compiling \" exception then \
                                         postpone here dup <r swap >r >r postpone , (postpone) jump postpone updateforwardjump ; immediate");
	// Note, #if, and #then, have to be defined without exception traps on compilation state, because the actual IF and THEN definitions rely on if and then for the trapping.
	// TODO Make forget work properaly

	InterpretForth(pExecState, ": leave #compileState @ 0 = if \" Cannot execute LEAVE when not compiling \" exception then (postpone) <r (postpone) dup (postpone) >r (postpone) jump ; immediate ");
	InterpretForth(pExecState, ": while #compileState @ 0 = if \" Cannot execute WHILE when not compiling \" exception then (postpone) not postpone if postpone leave postpone then ; immediate ");
	InterpretForth(pExecState, ": do #compileState @ 0 = if \" Cannot execute DO when not compiling \" exception then postpone here 0 postpone , (postpone) 3>r postpone here ; immediate ");
	InterpretForth(pExecState, ": loop #compileState @ 0 = if \" Cannot execute LOOP when not compiling \" exception then postpone , 2 + (postpone)  3<r (postpone)  >r (postpone) 1+ (postpone) 2dup (postpone) != (postpone) -rot (postpone) <r (postpone) 3>r (postpone) jumpontrue postpone here !inword (postpone) 3<r (postpone) 3drop ; immediate");
	InterpretForth(pExecState, ": +loop #compileState @ 0 = if \" Cannot execute +LOOP when not compiling \" exception then postpone , 2 + (postpone) swap (postpone) 3<r (postpone)  >r (postpone) rot (postpone) + (postpone) 2dup (postpone) != (postpone) -rot (postpone) <r (postpone) 3>r (postpone) jumpontrue postpone here !inword (postpone) 3<r (postpone) 3drop ; immediate");
	InterpretForth(pExecState, ": again #compileState @ 0 = if \" Cannot execute AGAIN when not compiling \" exception then postpone , 2 + (postpone) 3<r (postpone)  >r (postpone) 1+ (postpone) <r (postpone) 3>r (postpone) jump postpone here !inword (postpone) 3<r (postpone) 3drop ; immediate");
	InterpretForth(pExecState, ": begin #compileState @ 0 = if \" Cannot execute BEGIN when not compiling \" exception then 0 postpone , 0 postpone , postpone here 0 postpone , (postpone) 3>r postpone here ; immediate");
	InterpretForth(pExecState, ": until #compileState @ 0 = if \" Cannot execute UNTIL when not compiling \" exception then postpone , 2 + (postpone)  3<r (postpone)  >r (postpone) 1+ (postpone) <r (postpone) 3>r (postpone) swap (postpone) jumponfalse postpone here !inword (postpone) 3<r (postpone) 3drop ; immediate");
	InterpretForth(pExecState, ": repeat  #compileState @ 0 = if \" Cannot execute REPEAT when not compiling \" exception then postpone , 2 + (postpone) 3<r (postpone)  >r (postpone) 1+ (postpone) <r (postpone) 3>r (postpone) jump postpone here !inword (postpone) 3<r (postpone) 3drop ; immediate");

	// In the above:
	// postpone is placed before any immediate words that are to be compiled into the defining word (into the IF or the BEGIN words).  Otherwise they would be executed directly during the compilation
	//    : here , ; updateforwardjump leave if then     are all immediate words that needs to be postponed for them to make it into the words being defined
	// (postpone) is placed before any words that make it into the compiled word being defined, that have to be compiled into the ultimate word that will be created from these defining words
	//    <r dup > r swap 2dup                           etc are all preceded by (postpone), which means they make it into the final compiled word that if/repeat etc uses
	// If any immediate words were to be compiled into the final compiled word, then it would need to be preceeded by (postpone) postpone.

	InterpretForth(pExecState, ": space ( -- ) 32 emit ;"); // (  --  )
	InterpretForth(pExecState, ": spaces ( n -- ) dup 0 > if 0 do 32 emit loop then ;"); // ( n -- )

	InterpretForth(pExecState, ": I <r <r dup -rot >r >r ;");
	InterpretForth(pExecState, ": unloop 3<r 3drop ;");


	// Helper words
	InterpretForth(pExecState, ": ptop dup . cr ;"); // ( n -- n )
	InterpretForth(pExecState, ": pss depth . cr ;"); // ( n -- n )
	InterpretForth(pExecState, ": rprint2 <r ptop <r ptop >r >r ;");
	InterpretForth(pExecState, ": emitall ( ch0 ... chn -- ) depth 0 != if depth 0 do emit loop then ;");
	InterpretForth(pExecState, ": psd [char] R emit [char] : emit space rdepth . cr [char] T emit [char] : emit space tdepth . cr [char] S emit [char] : emit space depth . cr ;");

	// commented version does not check for 0-length stack
	InterpretForth(pExecState, ": .s depth dup dup 0 != if 0 do swap ptop >t loop 0 do <t loop else 2drop then ;"); // ( [s] -- [s] shows stack )
	InterpretForth(pExecState, ": .s depth dup dup 0 != if 0 do swap dup #s . cr >t loop 0 do <t loop else 2drop then ;"); // ( [s] -- [s] shows stack )
	InterpretForth(pExecState, ": .ts tdepth dup dup 0 != if >r 0 do <t ptop loop <r 0 do >t loop else 2drop then ;");
	InterpretForth(pExecState, ": .rs rdepth dup dup 0 != if 0 do 3<r <r ptop >t 3>r loop 0 do 3<r <t >r 3>r loop else 2drop then ;");
	InterpretForth(pExecState, ": clear depth dup 0 != if 0 do drop loop else drop then ; "); // ( [s] -- )
	InterpretForth(pExecState, ": rclear begin rdepth 1 > while <r <r drop >r repeat ;");

	// fi : function index
	InterpretForth(pExecState, "0 constant fi_getsize");
	InterpretForth(pExecState, "1 constant fi_hash");
	InterpretForth(pExecState, "2 constant fi_elementatindex");
	InterpretForth(pExecState, "3 constant fi_setelementindex");
	InterpretForth(pExecState, "4 constant fi_contains");
	InterpretForth(pExecState, "5 constant fi_indexof");
	InterpretForth(pExecState, "6 constant fi_subrange");
	InterpretForth(pExecState, "7 constant fi_deconstruct");
	InterpretForth(pExecState, "8 constant fi_append");
	InterpretForth(pExecState, "9 constant fi_implements");
	InterpretForth(pExecState, "10 constant fi_close");
	InterpretForth(pExecState, "11 constant fi_read");
	InterpretForth(pExecState, "12 constant fi_readLine");
	InterpretForth(pExecState, "13 constant fi_readChar");
	InterpretForth(pExecState, "14 constant fi_eof");

	InterpretForth(pExecState, ": len ( object -- length ) fi_getsize swap call ;");
	InterpretForth(pExecState, ": [n] ( n object -- nth element ) fi_elementatindex swap call ;");
	InterpretForth(pExecState, ": [n]= ( element n object -- ) fi_setelementindex swap call ;");
	InterpretForth(pExecState, ": appendelement ( element object -- ) dup len swap [n]= ;");
	InterpretForth(pExecState, ": prependelement ( element object -- ) -1 swap [n]= ;");
	InterpretForth(pExecState, ": contains ( element object -- b ) fi_contains swap call ;");
	InterpretForth(pExecState, ": indexof ( element object -- n ) 0 -rot fi_indexof swap call ;");
	InterpretForth(pExecState, ": indexof_from ( n element object -- n ) fi_indexof swap call ;");
	InterpretForth(pExecState, ": substring ( n m object : objectn...objectm ) fi_subrange swap call ;");
	InterpretForth(pExecState, ": deconstruct ( object -- [object state values] ) fi_deconstruct swap call ;");
	InterpretForth(pExecState, ": append ( element object -- ) fi_append swap call ;");
	InterpretForth(pExecState, ": close ( element object -- ) fi_close swap call ;");
	InterpretForth(pExecState, ": read ( object -- $ ) fi_read swap call ; ");
	InterpretForth(pExecState, ": readline ( object -- $ ) fi_readline swap call ; ");
	InterpretForth(pExecState, ": readChar ( object -- $ ) fi_readchar swap call ; ");
	InterpretForth(pExecState, ": eof ( object -- $ ) fi_eof swap call ; ");
}

void PreBuiltWords::InitialiseWord(ForthDict* pDict, const std::string& wordName, XT wordCode) {
	ForthWord* pForthWord = new ForthWord(wordName, wordCode);
	pForthWord->SetWordVisibility(true);
	pDict->AddWord(pForthWord);
}

void PreBuiltWords::InitialiseImmediateWord(ForthDict* pDict, const std::string& wordName, XT wordCode) {
	ForthWord* pForthWord = new ForthWord(wordName, wordCode);
	pForthWord->SetImmediate(true);
	pForthWord->SetWordVisibility(true);
	pDict->AddWord(pForthWord);
}

bool PreBuiltWords::InterpretForth(ExecState* pExecState, const std::string& toExecute) {
	pExecState->pInputProcessor->SetInputString(toExecute);
	return pExecState->pInputProcessor->Interpret(pExecState);
}

bool PreBuiltWords::CompileWordIntoWord(ForthDict* pDict, ForthWord* pForthWord, const std::string& wordName) {
	ForthWord* pAdd = pDict->FindWord(wordName);
	if (pAdd == nullptr) {
		return false;
	}
	pForthWord->CompileCFAPterIntoWord(pAdd->GetPterToBody());

	return true;
}

void PreBuiltWords::CompileTypeIntoWord(ForthWord* pForthWord, ForthType type) {
	pForthWord->CompileTypeIntoWord(type);
}

void PreBuiltWords::CompileLiteralValueIntoWord(ForthWord* pForthWord, int value) {
	pForthWord->CompileLiteralIntoWord((int64_t)value);
}

void PreBuiltWords::CompileLiteralValueIntoWord(ForthWord* pForthWord, bool value) {
	pForthWord->CompileLiteralIntoWord(value);
}

bool PreBuiltWords::BuiltIn_ThreadSafeBoolVariable(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElementIndex;
	bool incorrectType;
	std::tie(incorrectType, pElementIndex) = pExecState->pStack->PullType(StackElement_Int);
	if (incorrectType) {
		return pExecState->CreateException("Getting thread safe variable must be called with ( n -- ) - need an element index");
	}
	else if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException("whilst getting threadsafe bool");
	}
	int index = (int)pElementIndex->GetInt();
	delete pElementIndex;
	pElementIndex = nullptr;

	WordBodyElement** ppWBE = pExecState->GetPointerToBoolStateVariable(index);
	ForthType pterType = pTS->CreatePointerTypeTo(StackElement_Bool);
	StackElement* pElementVariable = new StackElement(pterType, ppWBE);
	if (!pExecState->pStack->Push(pElementVariable)) {
		return pExecState->CreateStackOverflowException("whilst pushing a state boolean variable");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_ThreadSafeIntVariable(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElementIndex;
	bool incorrectType;
	std::tie(incorrectType, pElementIndex) = pExecState->pStack->PullType(StackElement_Int);
	if (incorrectType) {
		return pExecState->CreateException("Getting thread safe variable must be called with ( n -- ) - need an element index");
	}
	else if (pElementIndex == nullptr) {
		return pExecState->CreateStackUnderflowException("whilst getting threadsafe int");
	}
	int index = (int)pElementIndex->GetInt();
	delete pElementIndex;
	pElementIndex = nullptr;

	WordBodyElement** ppWBE = pExecState->GetPointerToIntStateVariable(index);
	ForthType pterType = pTS->CreatePointerTypeTo(StackElement_Int);
	StackElement* pElementVariable = new StackElement(pterType, ppWBE);
	if (!pExecState->pStack->Push(pElementVariable)) {
		return pExecState->CreateStackOverflowException("whilst pushing a state int variable");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_CompileState(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	WordBodyElement** ppWBE = pExecState->GetPointerToIntStateVariable(ExecState::c_compileStateIndex);
	ForthType pterType = pTS->CreatePointerTypeTo(StackElement_Int);
	StackElement* pElementVariable = new StackElement(pterType, ppWBE);
	if (!pExecState->pStack->Push(pElementVariable)) {
		return pExecState->CreateStackOverflowException("whilst pushing a state int variable");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_PostponeState(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	WordBodyElement** ppWBE = pExecState->GetPointerToBoolStateVariable(ExecState::c_postponedExecIndex);
	ForthType pterType = pTS->CreatePointerTypeTo(StackElement_Bool);
	StackElement* pElementVariable = new StackElement(pterType, ppWBE);
	if (!pExecState->pStack->Push(pElementVariable)) {
		return pExecState->CreateStackOverflowException("whilst pushing a state boolean variable");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_InsideCommentState(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	WordBodyElement** ppWBE = pExecState->GetPointerToBoolStateVariable(ExecState::c_insideCommentIndex);
	ForthType pterType = pTS->CreatePointerTypeTo(StackElement_Bool);
	StackElement* pElementVariable = new StackElement(pterType, ppWBE);
	if (!pExecState->pStack->Push(pElementVariable)) {
		return pExecState->CreateStackOverflowException("whilst pushing a state boolean variable");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_InsideCommentLineState(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	WordBodyElement** ppWBE = pExecState->GetPointerToBoolStateVariable(ExecState::c_insideCommentLineIndex);
	ForthType pterType = pTS->CreatePointerTypeTo(StackElement_Bool);
	StackElement* pElementVariable = new StackElement(pterType, ppWBE);
	if (!pExecState->pStack->Push(pElementVariable)) {
		return pExecState->CreateStackOverflowException("whilst pushing a state boolean variable");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_DebugState(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	WordBodyElement** ppWBE = pExecState->GetPointerToIntStateVariable(ExecState::c_debugStateIndex);
	ForthType pterType = pTS->CreatePointerTypeTo(StackElement_Int);
	StackElement* pElementVariable = new StackElement(pterType, ppWBE);
	if (!pExecState->pStack->Push(pElementVariable)) {
		return pExecState->CreateStackOverflowException("whilst pushing a state int variable");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_IndirectDoCol(ExecState* pExecState) {
	WordBodyElement* pWbe = pExecState->pExecBody[pExecState->ip];
	pExecState->ip++;

	// Second element of indirect docol, is a pointer to the body which contains the elements to be executed
	// Used when creating words where the CFA needs to be point to a second level word, but the created word contains data
	WordBodyElement** secondayWordBody = pWbe->wordElement_BodyPter;
	// Push address of next element - where the data is stored
	pExecState->pStack->Push(pExecState->pExecBody + pExecState->ip);

	pExecState->NestAndSetCFA(secondayWordBody, 1);
	// The secondary word being called should know that there is a pointer on the stack to data
	bool success = BuiltIn_DoCol(pExecState);
	pExecState->UnnestCFA();
	return success;
}


bool PreBuiltWords::BuiltIn_DoCol(ExecState* pExecState) {
	bool exitFound = false;

	int64_t nDebugState = pExecState->GetIntTLSVariable(ExecState::c_debugStateIndex);

	if (nDebugState>0) {
		ostream* pStdoutStream = pExecState->GetStdout();
		(*pStdoutStream) << pExecState->pWordBeingInterpreted->GetName() << std::endl;
		return BuiltIn_DoCol_Debug(pExecState, pStdoutStream, 1);
	}

	while (!exitFound) {
		if (InputProcessor::ExecuteHaltRequested()) {
			InputProcessor::ResetExecutionHaltFlag();
			return pExecState->CreateException("Halted");
		}

		// Each execution state object contains a pointer to a word body.
		//  Traditionally in forth, this contain either machine, or in the case of a call to DoCol, it contains
		// pWbe[0]  :   address of docol (this method)
		// pWbe[1]  :   address of the first word's CFA to execute   <== pExecState->ip starts here, at 1
		// pWbe[2]  :   address of the second word's CFAto execute 
		// pWbe[3]  :   address of the third word's CFA to execute.  <== this may be the exit word, which is a function pointer that
		//                                                                 returns false but does not create an exception

		// A CFA is always a pointer to a pWbe[0], though, it is stored as a WordBodyElement union, and must be extracted from the .wordElement_BodyPter element
		// The first entry in pWbe, pWbe[0], is always of union element .wordElement_XT, and is therefore callable.

		WordBodyElement* pWbe = pExecState->pExecBody[pExecState->ip];
		// The WordBodyElement is a union that can contain pointers to CFAs (in this case) or other data (when the body is storing a constant for instance)
		WordBodyElement** pCFA = pWbe->wordElement_BodyPter;
		WordBodyElement* actualCFA = pCFA[0];
		XT exec = actualCFA->wordElement_XT;

		pExecState->ip++;

		// It would be easier, when creating defining words at least, if all words in a DOCOL were compiled, apart from immediate ones.
		//  A defining word definition has a lot of postpones in it:
		// : loop postpone , (postpone) postpone 2<r (postpone) postpone 1+ (postpone) 2dup (postpone) != (postpone) -rot (postpone) postpone 2>r (postpone) jumpontrue (postpone) 2<r (postpone) 2drop ; immediate

		// (note - (postpone) is equivalent to postpone postpone).
		// However, in DOCOL, we do not have the precedence bit - so do not know it's immediate.  This causes issues with every DOCOL called for compilation, as
		//  it would compile the EXIT and continue past into undefined behaviour.


		bool bExecutePostponed = pExecState->GetBoolTLSVariable(ExecState::c_postponedExecIndex);
		if (bExecutePostponed) {
			int64_t nCompileState = pExecState->GetIntTLSVariable(ExecState::c_compileStateIndex);

			if (!pExecState->SetVariable("#postponeState", false)) {
				return pExecState->CreateException("Could not reset postpone state flag");
			}

			if (nCompileState>0) {
				// Compile this word
				pExecState->pCompiler->CompileWord(pExecState, pCFA);
				if (!pExecState->SetVariable("#postponeState", false)) {
					return pExecState->CreateException("Could not reset postpone state flag");
				}
				continue;
			}
		}
		pExecState->NestAndSetCFA(pCFA, 1);
		try
		{
			if (!exec(pExecState)) {
				if (pExecState->exceptionThrown) {
					// TODO Fix this so it reports IP stack trace properly, and is guarded by an environment variable
					//int prevIP = pExecState->GetPreviousBodyIP() - 1;
					//pExecState->SetExeptionIP(prevIP);
					//cout << " Exception IP " << prevIP << endl;
					pExecState->UnnestCFA();
					return false;
				}
				exitFound = true;
			}
		}
		catch (...) {
			pExecState->UnnestCFA();
			throw;
		}
		pExecState->UnnestCFA();
	}

	return true;
}

bool PreBuiltWords::BuiltIn_DoCol_Debug(ExecState* pExecState, std::ostream* pStdoutStream, int indentation) {
	// All comments from BuiltIn_DoCol have been removed.
	// Extra code for debugging is highlighted
	bool exitFound = false;

	while (!exitFound) {
		if (InputProcessor::ExecuteHaltRequested()) {
			InputProcessor::ResetExecutionHaltFlag();
			return pExecState->CreateException("Halted");
		}
		// Added for debug code
		//
		int64_t nDebugState = pExecState->GetIntTLSVariable(ExecState::c_debugStateIndex);
		//
		////

		WordBodyElement* pWbe = pExecState->pExecBody[pExecState->ip];
		WordBodyElement** pCFA = pWbe->wordElement_BodyPter;
		WordBodyElement* actualCFA = pCFA[0];

		XT exec = actualCFA->wordElement_XT;

		pExecState->ip++;

		// Added for debug code
		//
		ForthWord* pWord = pExecState->pDict->FindWordFromCFAPter(pCFA);
		//
		////

		bool bExecutePostponed = pExecState->GetBoolTLSVariable(ExecState::c_postponedExecIndex);
		if (bExecutePostponed) {
			int64_t nCompileState = pExecState->GetIntTLSVariable(ExecState::c_compileStateIndex);

			if (!pExecState->SetVariable("#postponeState", false)) {
				return pExecState->CreateException("Could not reset postpone state flag");
			}

			if (nCompileState > 0) {
				// Compile this word

				// Added for debug code
				//
				if (pWord != nullptr) {
					(*pStdoutStream) << std::string(indentation, ' ');
					(*pStdoutStream) << "Compiling word: " << pWord->GetName() << std::endl;
				}
				//
				////

				pExecState->pCompiler->CompileWord(pExecState, pCFA);
				if (!pExecState->SetVariable("#postponeState", false)) {
					return pExecState->CreateException("Could not reset postpone state flag");
				}
				continue;
			}
			// Added for debug code
			//
			else {
				(*pStdoutStream) << std::string(indentation, ' ');
				(*pStdoutStream) << "Postpone state is set, when not compiling" << std::endl;
			}
			//
			////
		}
		pExecState->NestAndSetCFA(pCFA, 1);
		try
		{
			// Added for debug code
			//
			bool stepOver = false;
			if (nDebugState<3 && pWord != nullptr) {
				(*pStdoutStream) << std::string(indentation, ' ');
				bool allowStepInto = false;
				if (exec != PreBuiltWords::BuiltIn_DoCol) {
					(*pStdoutStream) << pWord->GetName();
					if (nDebugState == 2) {
						(*pStdoutStream) << std::endl;
					}
					else {
						(*pStdoutStream) << " (O,R,B) ";
					}
				}
				else {
					(*pStdoutStream) << pWord->GetName();
					if (nDebugState == 2) {
						(*pStdoutStream) << std::endl;
					}
					else {
						(*pStdoutStream) << " (I,O,R,B) ";
					}
					allowStepInto = true;
				}
				if (nDebugState == 1) {
					char c = pExecState->pInputProcessor->GetNextChar();
					c = std::tolower(c);
					if (c == 'r') {
						// RUN!
						nDebugState = 2;
						if (!pExecState->SetVariable("#debugState", (int64_t)2)) {
							return pExecState->CreateException("Could not set debugstate to RUN");
						}
					}
					else if (c == 'o') {
						// When control returns to this level of recursion, with this flag set the debug state will be set back to 1
						stepOver = true;
						nDebugState = 3;
						if (!pExecState->SetVariable("#debugState", (int64_t)3)) {
							return pExecState->CreateException("Could not set debugstate to DEBUGOVER");
						}
					}
					(*pStdoutStream) << std::endl;
				}

			}
			if (exec == PreBuiltWords::BuiltIn_DoCol) {
				if (!PreBuiltWords::BuiltIn_DoCol_Debug(pExecState, pStdoutStream, indentation + 1)) {
					if (pExecState->exceptionThrown) {
						pExecState->UnnestCFA();
						return false;
					}
					exitFound = true;
				}
			}
			//
			////  (else on next line is part of debug code)
			else if (!exec(pExecState)) {
				if (pExecState->exceptionThrown) {
					pExecState->UnnestCFA();
					return false;
				}
				exitFound = true;
			}
			// Added for debug code
			//
			if (stepOver && nDebugState == 3) {
				// Was stepping over, now continue to debug
				nDebugState = 1;
				if (!pExecState->SetVariable("#debugState", (int64_t)1)) {
					return pExecState->CreateException("Could not set debugstate to DEBUG");
				}
			}
			//
			////
		}
		catch (...) {
			pExecState->UnnestCFA();
			throw;
		}
		pExecState->UnnestCFA();
	}

	// Added for debug code
	//
	if (indentation == 1) {
		// Last return - ensure that anything executed next will debug and not just run through
		if (!pExecState->SetVariable("#debugState", (int64_t)1)) {
			return pExecState->CreateException("Could not set debugstate to DEBUG");
		}
	}
	//
	////
	return true;
}

bool PreBuiltWords::BuiltIn_Immediate(ExecState* pExecState) {
	return pExecState->pCompiler->SetLastWordCreatedToImmediate(pExecState);
}

bool PreBuiltWords::BuiltIn_Here(ExecState* pExecState) {
	// Pushes 'dictionary pointer' to parameter stack
	return pExecState->pCompiler->PushDPForCurrentlyCreatingWord(pExecState);
}

// Execute XT that is on the stack
bool PreBuiltWords::BuiltIn_Execute(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	WordBodyElement** pWBE = pElement->GetWordBodyElement();
	delete pElement;
	pElement = nullptr;

	pExecState->NestAndSetCFA(pWBE, 1);

	XT execFirst = pExecState->pExecBody[0]->wordElement_XT;
	bool returnValue = execFirst(pExecState);

	pExecState->UnnestCFA();

	return returnValue;
}

// Execute XT that is on the stack on object that next on the stack
bool PreBuiltWords::BuiltIn_ExecuteOnObject(ExecState* pExecState) {
	StackElement* pElementWordToExecute = pExecState->pStack->Pull();
	WordBodyElement** pWBE = pElementWordToExecute->GetWordBodyElement();
	delete pElementWordToExecute;
	pElementWordToExecute = nullptr;

	StackElement* pElementObjectToExecOn = pExecState->pStack->Pull();
	RefCountedObject* pObjToExecOn = pElementObjectToExecOn->GetObject();
	pExecState->NestSelfPointer(pObjToExecOn);
	delete pElementObjectToExecOn;
	pElementObjectToExecOn = nullptr;

	pExecState->NestAndSetCFA(pWBE, 1);

	XT execFirst = pExecState->pExecBody[0]->wordElement_XT;
	bool returnValue = execFirst(pExecState);

	pExecState->UnnestSelfPointer();

	pExecState->UnnestCFA();

	return returnValue;
}


bool PreBuiltWords::PushSelf(ExecState* pExecState) {
	// This call increments reference count on pter before returning, if it is valid.
	RefCountedObject* self = pExecState->GetCurrentSelfPter();

	if (self == nullptr) {
		return pExecState->CreateException("Not currently in object - self not defined");
	}

	bool success = true;
	if (!pExecState->pStack->Push(self)) {
		success = pExecState->CreateStackOverflowException();
	}
	self->DecReference();

	return success;
}

bool PreBuiltWords::BuiltIn_Exit(ExecState* pExecState) {
	// Ensure that exception state is reset, to let the calling method know this is the exit of the level-2 word.
	pExecState->exceptionThrown = false;
	return false;
}

bool PreBuiltWords::BuiltIn_Jump(ExecState* pExecState) {
	if (InputProcessor::ExecuteHaltRequested()) {
		InputProcessor::ResetExecutionHaltFlag();
		return pExecState->CreateException("Halted");
	}
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	int64_t newIp = pElement->GetInt();

	delete pElement;
	pElement = nullptr;

	if (newIp == 0) {
		// Initial CFA is generally DOCOL. However, it is an XT not a pointer to a body, which is what IP expects
		return pExecState->CreateException("Cannot jump to initial CFA in level-2 word");
	}
	// Jump the body that nested this jump call to a different IP
	pExecState->SetPreviousBodyIP((int)newIp);
	return true;
}

bool PreBuiltWords::BuiltIn_JumpOnTrue(ExecState* pExecState) {
	if (InputProcessor::ExecuteHaltRequested()) {
		InputProcessor::ResetExecutionHaltFlag();
		return pExecState->CreateException("Halted");
	}

	StackElement* pAddrElement = nullptr;
	StackElement* pBoolElement = nullptr;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pAddrElement, pBoolElement)) {
		return false;
	}
	int64_t newIp = pAddrElement->GetInt();
	bool flag = pBoolElement->GetBool();

	ForthWord::BuiltInHelper_DeleteOperands(pAddrElement, pBoolElement);

	if (newIp == 0) {
		return pExecState->CreateException("Cannot jump to initialise CFA in level-2 word");
	}
	if (flag) {
		// Jump the body that nested this jump call to a different IP
		pExecState->SetPreviousBodyIP((int)newIp);
	}
	return true;
}

bool PreBuiltWords::BuiltIn_JumpOnFalse(ExecState* pExecState) {
	if (InputProcessor::ExecuteHaltRequested()) {
		InputProcessor::ResetExecutionHaltFlag();
		return pExecState->CreateException("Halted");
	}

	StackElement* pAddrElement = nullptr;
	StackElement* pBoolElement = nullptr;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pAddrElement, pBoolElement)) {
		return false;
	}
	int64_t newIp = pAddrElement->GetInt();
	bool flag = pBoolElement->GetBool();

	ForthWord::BuiltInHelper_DeleteOperands(pAddrElement, pBoolElement);

	if (newIp == 0) {
		return pExecState->CreateException("Cannot jump to initialise CFA in level-2 word");
	}
	if (!flag) {
		// Jump the body that nested this jump call to a different IP
		pExecState->SetPreviousBodyIP((int)newIp);
	}
	return true;
}

// ( addr:value -- ) *addr=value
bool PreBuiltWords::BuiltIn_PokeIntegerInWord(ExecState* pExecState) {
	int64_t nCompileState = pExecState->GetIntTLSVariable(ExecState::c_compileStateIndex);
	if (nCompileState==0) {
		return pExecState->CreateException("Cannot execute !INWORD when not compiling");
	}
	StackElement* pAddressElement = nullptr;
	StackElement* pValueElement = nullptr;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pAddressElement, pValueElement)) {
		return false;
	}
	if (pValueElement->GetType() != StackElement_Int) {
		ForthWord::BuiltInHelper_DeleteOperands(pValueElement, pAddressElement);
		return pExecState->CreateException("Need an integer value to poke into word");
	}
	if (pAddressElement->GetType() != StackElement_Int) {
		ForthWord::BuiltInHelper_DeleteOperands(pValueElement, pAddressElement);
		return pExecState->CreateException("Need an integer address to poke into word");
	}
	int v = (int)pValueElement->GetInt();
	int addr = (int)pAddressElement->GetInt();
	ForthWord::BuiltInHelper_DeleteOperands(pValueElement, pAddressElement);

	WordBodyElement* pLiteral = new WordBodyElement();
	pLiteral->wordElement_int = v;
	return pExecState->pCompiler->AlterElementInWordUnderCreation(pExecState, addr, pLiteral);
}

bool PreBuiltWords::PushRefCount(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	StackElement* pElement = pExecState->pStack->TopElement();
	if (pElement == nullptr) {
		return pExecState->CreateException("No TOS to out reference count for");
	}
	if (pTS->TypeIsObjectOrObjectPter(pElement->GetType())) {
		int refCount = pTS->GetReferenceCount(pElement->GetType(), pElement->GetContainedPter());
		ostream* pStdoutStream = pExecState->GetStdout();
		(*pStdoutStream) << "Reference count is " << refCount << std::endl;
	}
	else {
		return pExecState->CreateException("Only object types have reference counts");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_WordCFAFromInputStream(ExecState* pExecState) {
	InputWord iw = pExecState->GetNextWordFromInput();
	std::string word = iw.word;
	ForthWord* pWord = pExecState->pDict->FindWord(word);
	if (pWord == nullptr) {
		return pExecState->CreateException("Cannot find word in dictionary");
	}
	WordBodyElement** pCFA = pWord->GetPterToBody();
	StackElement* pNewElement = new StackElement(pCFA);
	pExecState->pStack->Push(pNewElement);
	return true;
}

bool PreBuiltWords::BuiltIn_WordCFAFromDefinition(ExecState* pExecState) {
	WordBodyElement* pWBE = pExecState->GetNextWordFromCurrentBodyAndIncIP();

	if (pWBE == nullptr) {
		return pExecState->CreateException("Expecting a word pointer in definition");
	}
	if (!pExecState->pStack->Push(new StackElement(pWBE->wordElement_BodyPter))) {
		return pExecState->CreateStackUnderflowException();
	}
	return true;
}

bool PreBuiltWords::Quit(ExecState* pExecState) {
	return true;
}

// TODO Do something with found word.
// Get next word from input stream, and find it in the dictionary
bool PreBuiltWords::BuiltIn_Find(ExecState* pExecState) {
	InputWord iw = pExecState->GetNextWordFromInput();
	std::string word = iw.word;
	return true;
}


bool PreBuiltWords::BuiltIn_ParenthesisCommentStart(ExecState* pExecState) {
	if (!pExecState->SetVariable("#insideComment", true)) {
		return pExecState->CreateException("Could not set #insideComment flag");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_ParenthesisCommentEnd(ExecState* pExecState) {
	if (!pExecState->SetVariable("#insideComment", false)) {
		return pExecState->CreateException("Could not reset #insideComment flag");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_LineCommentStart(ExecState* pExecState) {
	if (!pExecState->SetVariable("#insideCommentLine", true)) {
		return pExecState->CreateException("Could not set #insideCommentLine flag");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Allot(ExecState* pExecState) {
	StackElement* pElementAllotBy = pExecState->pStack->Pull();
	if (pElementAllotBy == nullptr) {
		return pExecState->CreateStackUnderflowException("whilst attempting to allot cells to a word");
	}
	else if (pElementAllotBy->GetType() != StackElement_Int) {
		delete pElementAllotBy;
		pElementAllotBy = nullptr;
		return pExecState->CreateException("ALLOT requires an integer");
	}
	int64_t n = pElementAllotBy->GetInt();
	delete pElementAllotBy;
	pElementAllotBy = nullptr;//

	bool success = pExecState->pCompiler->ExpandLastWordCompiledBy(pExecState, (int)n);

	return success;
}

bool PreBuiltWords::BuiltIn_Reveal(ExecState* pExecState) {
	return pExecState->pCompiler->RevealWord(pExecState, true);
}

bool PreBuiltWords::BuiltIn_RevealToStack(ExecState* pExecState) {
	return pExecState->pCompiler->RevealWord(pExecState, false);
}

bool PreBuiltWords::BuiltIn_Does(ExecState* pExecState) {
	if (!pExecState->pCompiler->HasValidLastWordCreated()) {
		return pExecState->CreateException("Cannot execute does> when not compiling");
	}
	if (!pExecState->SetVariable("#compileState", (int64_t)2)) {
		return pExecState->CreateException("Could not set #compileState to 2");
	}
	if (!pExecState->SetVariable("#postponeState", true)) {
		return pExecState->CreateException("Could not set postpone state flag");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Create(ExecState* pExecState) {
	pExecState->pCompiler->AbandonWordUnderCreation();
	// Get next word from input stream

	InputWord iw = pExecState->GetNextWordFromInput();
	std::string word = iw.word;

	pExecState->pCompiler->StartWordCreation(word);
	pExecState->pCompiler->CompileDoesXT(pExecState, PreBuiltWords::BuiltIn_PushPter);

	return true;
}

bool PreBuiltWords::BuiltIn_AddWordToObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();

	// ( word object -- )
	StackElement* pElementType = pExecState->pStack->Pull();
	if (pElementType == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (pElementType->GetType() != StackElement_Type ||
		!pTS->TypeIsUserObject(pElementType->GetValueType())) {
		delete pElementType;
		pElementType = nullptr;
		return pExecState->CreateException("Adding a word to an object requires ( word type -- ), where type is a type of user object.  No such type found");
	}

	StackElement* pElementWord = pExecState->pStack->Pull();
	if (pElementWord == nullptr) {
		delete pElementType;
		pElementType = nullptr;
		return pExecState->CreateStackUnderflowException();
	}
	if (pElementWord->GetType() != ObjectType_Word) {
		delete pElementType;
		pElementType = nullptr;
		delete pElementWord;
		pElementWord = nullptr;
		return pExecState->CreateException("Adding a word to an object requires ( word object -- ), no word found on stack");
	}

	ForthType objectType = pElementType->GetValueType();
	ForthWord* pWord = (ForthWord*)pElementWord->GetObject();

	bool success = true;
	success = pTS->AddWordToObject(pExecState, objectType, pWord);

	// Cannot delete these before adding the word to the object, as it could bring the ref count to 0.  (Although the word
	//  has likely just been created, it will still have a reference for the last word created.  However, the object may
	//  only exist on the stack).
	delete pElementWord;
	pElementWord = nullptr;
	delete pElementType;
	pElementType = nullptr;

	return success;
}

// TODO Once variables are working, remove compile state from ExecState and implement this word in Forth
bool PreBuiltWords::BuiltIn_StartCompilation(ExecState* pExecState) {
	if (!pExecState->SetVariable("#compileState", (int64_t)1)) {
		return pExecState->CreateException("Could not set compilation state to 1");
	}
	return true;
}

// TODO Once variables are working, remove compile state from ExecState and implement this word in Forth
bool PreBuiltWords::BuiltIn_EndCompilation(ExecState* pExecState) {
	if (!pExecState->SetVariable("#compileState", (int64_t)0)) {
		return pExecState->CreateException("Could not reset compilation state");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Forget(ExecState* pExecState) {
	InputWord iw = pExecState->GetNextWordFromInput();
	std::string word = iw.word;

	return pExecState->pDict->ForgetWord(word);;
}

bool PreBuiltWords::BuiltIn_Postpone(ExecState* pExecState) {
	if (!pExecState->SetVariable("#postponeState", true)) {
		return pExecState->CreateException("Could not set postpone state flag");
	}
	return true;
}

// None-immediate version of postpone, used to compile postpone into a defining word
bool PreBuiltWords::BuiltIn_PostponePostpone(ExecState* pExecState) {
	if (!pExecState->SetVariable("#postponeState", true)) {
		return pExecState->CreateException("Could not set postpone state flag");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_ResetPostponeState(ExecState* pExecState) {
	if (!pExecState->SetVariable("#postponeState", false)) {
		return pExecState->CreateException("Could not reset postpone state flag");
	}
	return true;
}


// ( e -- $) formats element e and converts to string $
bool PreBuiltWords::ToString(ExecState* pExecState) {
	StackElement* pElementToStringify = pExecState->pStack->Pull();
	if (pElementToStringify == nullptr) {
		return pExecState->CreateReturnStackUnderflowException();
	}
	bool success = pElementToStringify->ToString(pExecState);
	delete pElementToStringify;
	pElementToStringify = nullptr;

	

	return success;
}

// ( n -- c)
bool PreBuiltWords::ToChar(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != StackElement_Char && elementType != StackElement_Int) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Can only convert ints and chars, to chars");
	}
	if (elementType == StackElement_Char) {
		if (!pExecState->pStack->Push(pElement)) {
			return pExecState->CreateStackOverflowException();
		}
	}
	else {
		int64_t n = pElement->GetInt();
		delete pElement;
		pElement = nullptr;
		// Only supporting ascii at the moment
		char c = (char)(n & 0xff);
		if (!pExecState->pStack->Push(c)) {
			return pExecState->CreateStackOverflowException();
		}
	}
	return true;
}

// ( $/c/f -- n)
bool PreBuiltWords::ToInt(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType!= StackElement_Int && elementType != StackElement_Char && elementType != StackElement_Float && elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Can only convert floats, ints, chars and strings, to ints");
	}
	if (elementType == StackElement_Int) {
		if (!pExecState->pStack->Push(pElement)) {
			return pExecState->CreateStackOverflowException("whilst converting an int to an int");
		}
		return true;
	}
	int64_t n;
	if (elementType == StackElement_Char) {
		char c = pElement->GetChar();

		n = (int64_t)c;
	}
	else if (elementType == StackElement_Float) {
		double f = pElement->GetFloat();
		n = (int64_t)f;
	}
	else if (elementType == ObjectType_String) {
		ForthString* pString = (ForthString*)pElement->GetObject();
		std::string s = pString->GetContainedString();

		char* end;
		n = strtoll(s.c_str(), &end, 10);
		if (end == s.c_str()) {
			delete pElement;
			pElement = nullptr;
			return pExecState->CreateException("Could not convert string to integer");
		}
	}

	delete pElement;
	pElement = nullptr;

	if (!pExecState->pStack->Push(n)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS to an integer");
	}
	return true;
}

// ( $/n -- f)
bool PreBuiltWords::ToFloat(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != StackElement_Int && elementType != StackElement_Float && elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Can only convert floats, chars and strings, to ints");
	}
	if (elementType == StackElement_Float) {
		if (!pExecState->pStack->Push(pElement)) {
			return pExecState->CreateStackOverflowException("whilst converting a float to a float");
		}
		return true;
	}
	double f;

	if (elementType == StackElement_Int) {
		int64_t n = pElement->GetInt();
		f = (double)n;
	}
	else if (elementType == ObjectType_String) {
		ForthString* pString = (ForthString*)pElement->GetObject();
		std::string s = pString->GetContainedString();

		std::string convertWord = s;
		/*if (s[s.length() - 1] == 'f') {
			convertWord = s.substr(0, s.length() - 1);
		}*/
		char* end;
		f = strtod(convertWord.c_str(), &end);
		if (end == convertWord.c_str()) {
			delete pElement;
			pElement = nullptr;
			return pExecState->CreateException("Could not convert string to float");
		}
	}

	delete pElement;
	pElement = nullptr;

	if (!pExecState->pStack->Push(f)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS to float");
	}
	return true;
}

// ( $ -- n)
bool PreBuiltWords::WordToInt(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("WordToInt requires a string");
	}

	int64_t n;
	ForthString* pString = (ForthString*)pElement->GetObject();
	std::string s = pString->GetContainedString();
	delete pElement;
	pElement = nullptr;
	if (s.find(".") != std::string::npos || s[s.length() - 1] == 'f') {
		return pExecState->CreateException("Could not convert float string to integer");
	}

	char* end;
	n = strtoll(s.c_str(), &end, 10);
	if (end == s.c_str() || *end != '\0') {
		return pExecState->CreateException("Could not convert entire string to integer");
	}
	else if (!pExecState->pStack->Push(n)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS word to an integer");
	}
	return true;
}


// ( $ -- f)
bool PreBuiltWords::WordToFloat(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType elementType = pElement->GetType();
	if (elementType != ObjectType_String) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("WordToFloat requires a string");
	}

	double f;

	ForthString* pString = (ForthString*)pElement->GetObject();
	std::string s = pString->GetContainedString();
	delete pElement;
	pElement = nullptr;

	std::string convertWord = s;
	if (s[s.length() - 1] == 'f') {
		convertWord = s.substr(0, s.length() - 1);
	}
	char* end;
	f = strtod(convertWord.c_str(), &end);
	if (end == convertWord.c_str() || *end != '\0') {
		return pExecState->CreateException("Could not convert entire word to float");
	}
	else if (!pExecState->pStack->Push(f)) {
		return pExecState->CreateStackOverflowException("whilst converting TOS word to float");
	}
	return true;
}

// ( [e] -- b ) true if [e] is object, false if [e] is pter to anything or value
bool PreBuiltWords::IsObject(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType t = pElement->GetType();
	delete pElement;
	pElement = nullptr;
	StackElement* pReturnElement = new StackElement(pTS->TypeIsObject(t));
	if (!pExecState->pStack->Push(pReturnElement)) {
		return pExecState->CreateStackOverflowException("whilst determining if a type is an object");
	}
	return true;
};

// ( [e] -- b ) true if [e] is pter to object or value, false otherwise
bool PreBuiltWords::IsPter(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	ForthType t = pElement->GetType();
	delete pElement;
	pElement = nullptr;
	StackElement* pReturnElement = new StackElement(TypeSystem::IsPter(t));
	if (!pExecState->pStack->Push(pReturnElement)) {
		return pExecState->CreateStackOverflowException("whilst determining if a type is a pter");
	}
	return true;
};

// ( n -- n n )
bool PreBuiltWords::BuiltIn_Dup(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();

	if (pElement == nullptr) {
		return pExecState->CreateException("Stack underflow");
	}

	StackElement* pClonedElement = new StackElement(*pElement);

	pExecState->pStack->Push(pElement);
	pExecState->pStack->Push(pClonedElement);

	return true;
}

// ( n m -- m n )
bool PreBuiltWords::BuiltIn_Swap(ExecState* pExecState) {
	StackElement* pElement1 = nullptr;
	StackElement* pElement2 = nullptr;
	// Element1 is further in the stack than element2
	// ( Element1 Element2 -- Element2 Element1)
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}

	pExecState->pStack->Push(pElement2);
	pExecState->pStack->Push(pElement1);

	return true;
}

// ( n -- )
bool PreBuiltWords::BuiltIn_Drop(ExecState* pExecState) {
	StackElement* pElement1 = nullptr;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement1)) {
		return false;
	}
	delete pElement1;
	pElement1 = nullptr;
	return true;
}

// ( n m -- n m n)
bool PreBuiltWords::BuiltIn_Over(ExecState* pExecState) {
	StackElement* pElement1 = nullptr;
	StackElement* pElement2 = nullptr;
	// Element1 is further in the stack than element2
	// ( Element1 Element2 -- Element2 Element1)
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	pExecState->pStack->Push(pElement1);
	pExecState->pStack->Push(pElement2);
	StackElement* pElement3 = new StackElement(*pElement1);
	pExecState->pStack->Push(pElement3);
	return true;
}

// ( m n p -- n p m)
bool PreBuiltWords::BuiltIn_Rot(ExecState* pExecState) {
	StackElement* pElementm = nullptr;
	StackElement* pElementn = nullptr;
	StackElement* pElementp = nullptr;
	if (!ForthWord::BuiltInHelper_GetThreeStackElements(pExecState, pElementm, pElementn, pElementp)) {
		return false;
	}

	pExecState->pStack->Push(pElementn);
	pExecState->pStack->Push(pElementp);
	pExecState->pStack->Push(pElementm);
	return true;
}

// ( m n p -- p m n)
bool PreBuiltWords::BuiltIn_ReverseRot(ExecState* pExecState) {
	StackElement* pElementm = nullptr;
	StackElement* pElementn = nullptr;
	StackElement* pElementp = nullptr;
	if (!ForthWord::BuiltInHelper_GetThreeStackElements(pExecState, pElementm, pElementn, pElementp)) {
		return false;
	}

	pExecState->pStack->Push(pElementp);
	pExecState->pStack->Push(pElementm);
	pExecState->pStack->Push(pElementn);
	return true;
}

// >R ( a -- R: a )
bool PreBuiltWords::BuiltIn_PushDataStackToReturnStack(ExecState* pExecState) {
	StackElement* pElement = nullptr;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (pElement->GetType() != StackElement_Int) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Cannot push a non-integer to the return stack");
	}
	int nToPush = (int)pElement->GetInt();
	delete pElement;
	pElement = nullptr;
	if (!pExecState->pReturnStack->Push(nToPush)) {
		return pExecState->CreateReturnStackOverflowException();
	}
	return true;
}

// R> ( R: a -- a )
bool PreBuiltWords::BuiltIn_PushReturnStackToDataStack(ExecState* pExecState) {
	int nFromReturnStack;
	if (!pExecState->pReturnStack->Pull(nFromReturnStack)) {
		return pExecState->CreateReturnStackUnderflowException();
	}
	if (!pExecState->pStack->Push((int64_t)nFromReturnStack)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

// >T ( a -- T: a)
bool PreBuiltWords::BuiltIn_PushDataStackToTempStack(ExecState* pExecState) {
	StackElement* pElement = nullptr;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pExecState->pTempStack->Push(pElement)) {
		return pExecState->CreateTempStackOverflowException();
	}
	return true;
}

// <T ( T:a -- a)
bool PreBuiltWords::BuiltIn_PushTempStackToDataStack(ExecState* pExecState) {
	StackElement* pElement = nullptr;
	if (!ForthWord::BuiltInHelper_GetOneTempStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pExecState->pStack->Push(pElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_StackSize(ExecState* pExecState) {
	int64_t stackSize = pExecState->pStack->Count();
	StackElement* pNewElement = new StackElement(stackSize);
	// If push fails, it deletes the element
	if (!pExecState->pStack->Push(pNewElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_RStackSize(ExecState* pExecState) {
	int64_t stackSize = pExecState->pReturnStack->Count();
	StackElement* pNewElement = new StackElement(stackSize);
	// If push fails, it deletes the element
	if (!pExecState->pStack->Push(pNewElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_TStackSize(ExecState* pExecState) {
	int64_t stackSize = pExecState->pTempStack->Count();
	StackElement* pNewElement = new StackElement(stackSize);
	// If push fails, it deletes the element
	if (!pExecState->pStack->Push(pNewElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_PrintStackTop(ExecState* pExecState) {
	bool success = true;

	StackElement* pTop = pExecState->pStack->Pull();
	if (pTop == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (!pTop->ToString(pExecState)) {
		delete pTop;
		pTop = nullptr;
		return false;
	}
	delete pTop;
	pTop = nullptr;
	std::string str;
	tie(success, str) = pExecState->pStack->PullAsString();
	if (success) {
		ostream* pStdoutStream = pExecState->GetStdout();
		(*pStdoutStream) << str;
		return true;
	}
	else {
		return pExecState->CreateException(str.c_str());
	}
}

bool PreBuiltWords::BuiltIn_Emit(ExecState* pExecState) {
	StackElement* pTop = pExecState->pStack->Pull();
	if (pTop == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	ForthType t = pTop->GetType();

	ostream* pStdoutStream = pExecState->GetStdout();
	bool success = true;
	if (t == StackElement_Int) {
		char c = (pTop->GetInt() & 0xff);

		(*pStdoutStream) << c;
	}
	else if (t == StackElement_Char) {
		char c = pTop->GetChar();
		(*pStdoutStream) << c;
	}
	else {
		success = pExecState->CreateException("Could not convert top to char");
	}
	delete pTop;
	pTop = nullptr;
	return success;
}

bool PreBuiltWords::BuiltIn_Add(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Add);
}

bool PreBuiltWords::BuiltIn_Subtract(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Subtract);
}

bool PreBuiltWords::BuiltIn_Multiply(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Multiply);
}

bool PreBuiltWords::BuiltIn_Divide(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Divide);
}

bool PreBuiltWords::BuiltIn_Modulus(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Modulus);
}

bool PreBuiltWords::BuiltIn_LessThan(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_LessThan);
}

bool PreBuiltWords::BuiltIn_LessThanOrEquals(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_LessThanOrEquals);
}

bool PreBuiltWords::BuiltIn_Equals(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_Equals);
}

bool PreBuiltWords::BuiltIn_NotEquals(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_NotEquals);
}

bool PreBuiltWords::BuiltIn_GreaterThanOrEquals(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_GreaterThanOrEquals);
}

bool PreBuiltWords::BuiltIn_GreaterThan(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_BinaryOperation(pExecState, BinaryOp_GreaterThan);
}

bool PreBuiltWords::BuiltIn_Not(ExecState* pExecState) {
	StackElement* pElement = pExecState->pStack->Pull();
	if (pElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (pElement->GetType() != StackElement_Bool) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Cannot invert non-boolean type");
	}
	bool value = pElement->GetBool();
	delete pElement;
	pElement = nullptr;

	pElement = new StackElement(!value);
	if (!pExecState->pStack->Push(pElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Or(ExecState* pExecState) {
	StackElement* pElement1;
	StackElement* pElement2;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	if (pElement1->GetType() != StackElement_Bool ||
		pElement2->GetType() != StackElement_Bool) {
		ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("OR operation needs two booleans");
	}
	bool value1 = pElement1->GetBool();
	bool value2 = pElement2->GetBool();
	ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);

	StackElement* pElementResult = new StackElement(value1 || value2);
	if (!pExecState->pStack->Push(pElementResult)) {
		return pExecState->CreateStackUnderflowException();
	}

	return true;
}

bool PreBuiltWords::BuiltIn_And(ExecState* pExecState) {
	StackElement* pElement1;
	StackElement* pElement2;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	if (pElement1->GetType() != StackElement_Bool ||
		pElement2->GetType() != StackElement_Bool) {
		ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("AMD operation needs two booleans");
	}
	bool value1 = pElement1->GetBool();
	bool value2 = pElement2->GetBool();
	ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);

	StackElement* pElementResult = new StackElement(value1 && value2);
	if (!pExecState->pStack->Push(pElementResult)) {
		return pExecState->CreateStackUnderflowException();
	}

	return true;
}

bool PreBuiltWords::BuiltIn_Xor(ExecState* pExecState) {
	StackElement* pElement1;
	StackElement* pElement2;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	if (pElement1->GetType() != StackElement_Bool ||
		pElement2->GetType() != StackElement_Bool) {
		ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("XOR operation needs two booleans");
	}
	bool value1 = pElement1->GetBool();
	bool value2 = pElement2->GetBool();
	ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);

	StackElement* pElementResult = new StackElement(value1 != value2);
	if (!pExecState->pStack->Push(pElementResult)) {
		return pExecState->CreateStackUnderflowException();
	}

	return true;
}

bool PreBuiltWords::BuiltIn_Power(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement1;
	StackElement* pElement2;
	if (!ForthWord::BuiltInHelper_GetTwoStackElements(pExecState, pElement1, pElement2)) {
		return false;
	}
	ForthType type1 = pElement1->GetType();
	ForthType type2 = pElement2->GetType();
	if (!pTS->IsNumeric(type1) || !pTS->IsNumeric(type2)) {
		ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);
		return pExecState->CreateException("Power function needs to numeric elements to operate on");
	}
	// Return x ^ y
	double x = pElement1->GetFloat();
	double y = pElement2->GetFloat();
	ForthWord::BuiltInHelper_DeleteOperands(pElement1, pElement2);

	double toReturn = pow(x, y);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of power function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Cos(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pTS->IsNumeric(pElement->GetType())) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Cos function needs a numeric to operate on");
	}

	double theta = pElement->GetFloat();
	delete pElement;
	pElement = nullptr;
	double toReturn = cos(theta);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of cosine function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Sin(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pTS->IsNumeric(pElement->GetType())) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Sin function needs a numeric to operate on");
	}

	double theta = pElement->GetFloat();
	delete pElement;
	pElement = nullptr;
	double toReturn = sin(theta);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of sine function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Tan(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pTS->IsNumeric(pElement->GetType())) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Tan function needs a numeric to operate on");
	}
	double theta = pElement->GetFloat();
	delete pElement;
	pElement = nullptr;
	double toReturn = tan(theta);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of tan function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Arccosine(ExecState* pExecState) { 
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pTS->IsNumeric(pElement->GetType())) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Cos-1 function needs a numeric to operate on");
	}

	double theta = pElement->GetFloat();
	delete pElement;
	pElement = nullptr;
	double toReturn = acos(theta);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of arccosine function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Arcsine(ExecState* pExecState) { 
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pTS->IsNumeric(pElement->GetType())) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Sin-1 function needs a numeric to operate on");
	}

	double theta = pElement->GetFloat();
	delete pElement;
	pElement = nullptr;
	double toReturn = asin(theta);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of a4rcsine function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Arctan(ExecState* pExecState) { 
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pTS->IsNumeric(pElement->GetType())) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Tan-1 function needs a numeric to operate on");
	}
	double theta = pElement->GetFloat();
	delete pElement;
	pElement = nullptr;
	double toReturn = atan(theta);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of arctan function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Sqrt(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (!pTS->IsNumeric(pElement->GetType())) {
		delete pElement;
		pElement = nullptr;
		return pExecState->CreateException("Sqrt function needs a numeric to operate on");
	}
	double x = pElement->GetFloat();
	delete pElement;
	pElement = nullptr;
	double toReturn = sqrt(x);
	if (!pExecState->pStack->Push(toReturn)) {
		return pExecState->CreateStackOverflowException("whilst pushing result of sqrt function");
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Peek(ExecState* pExecState) {
	StackElement* pElementAddress = pExecState->pStack->TopElement();

	if (pElementAddress == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (!pElementAddress->IsPter()) {
		return pExecState->CreateException("Cannot load pter value, as top of stack is not a pointer");
	}
	pExecState->pStack->Pull();

	bool returnValue = true;
	StackElement* pValueElement = pElementAddress->GetDerefedPterValueAsStackElement();
	if (pValueElement == nullptr) {
		returnValue = pExecState->CreateException("Could not read value of pter on stack");
	}
	else {
		pExecState->pStack->Push(pValueElement);
	}

	delete pElementAddress;
	pElementAddress = nullptr;

	return returnValue;
}

bool PreBuiltWords::BuiltIn_Poke(ExecState* pExecState) {
	StackElement* pAddressElement = pExecState->pStack->TopElement();

	if (pAddressElement == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	if (!pAddressElement->IsPter()) {
		return pExecState->CreateException("Cannot store into pter value, as top of stack is not a pointer");
	}
	pExecState->pStack->Pull();
	StackElement* pValueElement = pExecState->pStack->Pull();
	if (pValueElement == nullptr) {
		delete pAddressElement;
		pAddressElement = nullptr;
		return pExecState->CreateException("Cannot get value to set pter to, as stack empty.  Stack underflow");
	}
	bool returnValue = true;

	bool success;
	success = pAddressElement->PokeIntoContainedPter(pExecState, pValueElement);
	delete pAddressElement;
	pAddressElement = nullptr;
	delete pValueElement;
	pValueElement = nullptr;
	return success;
}

bool PreBuiltWords::BuiltIn_PushPter(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	WordBodyElement** ppWBE_Type = pExecState->GetWordPterAtOffsetFromCurrentBody(1);
	WordBodyElement** ppWBE_Literal = pExecState->GetWordPterAtOffsetFromCurrentBody(2);
	if (pExecState->CurrentBodyIsInLastCompiledWord()) {
		// 
		pExecState->pCompiler->ForgetLastCompiledWord();
	}

	if (ppWBE_Type == nullptr || ppWBE_Literal == nullptr) {
		return pExecState->CreateException("Cannot push a pointer a word-contained literal, as has to have both a type and a literal value");
	}
	ForthType currentType = (*ppWBE_Type)->forthType;
	ForthType pointerType = pTS->CreatePointerTypeTo(currentType);
	//void* pter = reinterpret_cast<void*>(ppWBE_Literal[0]);
	void* pter = static_cast<void*>(ppWBE_Literal);
	StackElement* pNewStackElement = new StackElement(pointerType, pter);
	if (!pExecState->pStack->Push(pNewStackElement)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_Variable(ExecState* pExecState) {
	TypeSystem* pTS = TypeSystem::GetTypeSystem();
	if (!PreBuiltWords::BuiltIn_Create(pExecState)) {
		return false;
	}

	if (!ForthWord::BuiltInHelper_CompileTOSLiteral(pExecState, false)) return false;
	pExecState->pCompiler->CompileDoesXT(pExecState, PreBuiltWords::BuiltIn_PushPter);
	pExecState->pCompiler->RevealWord(pExecState, true);

	return true;
}

bool PreBuiltWords::BuiltIn_True(ExecState* pExecState) {
	if (!pExecState->pStack->Push(true)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_False(ExecState* pExecState) {
	if (!pExecState->pStack->Push(false)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_PushType(ExecState* pExecState) {
	StackElement* pSE = pExecState->pStack->Pull();
	if (pSE == nullptr) {
		return pExecState->CreateStackUnderflowException();
	}
	//ValueType t = pSE->GetTypeAsTypeValue();
	ForthType t = pSE->GetType();
	delete pSE;
	pSE = nullptr;

	if (!pExecState->pStack->Push(t)) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_TypeFromInt(ExecState* pExecState) {
	StackElement* pElement;
	if (!ForthWord::BuiltInHelper_GetOneStackElement(pExecState, pElement)) {
		return false;
	}
	if (pElement->GetType() != ValueType_Int) {
		return pExecState->CreateException("Only ints can be cast to types");
	}
	int64_t typeAsInt = pElement->GetInt();
	ForthWord::BuiltInHelper_DeleteStackElement(pElement);

	ForthType typeAsType = (ForthType)typeAsInt;
	pExecState->pStack->Push(typeAsType);

	return true;
}

bool PreBuiltWords::BuiltIn_Literal(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_CompileTOSLiteral(pExecState, true);
}

bool PreBuiltWords::BuiltIn_LiteralNoPush(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_CompileTOSLiteral(pExecState, false);
}


bool PreBuiltWords::BuiltIn_CharLiteral(ExecState* pExecState) {
	int64_t nCompileState = pExecState->GetIntTLSVariable(ExecState::c_compileStateIndex);
	if (nCompileState==0) {
		return true;
	}
	pExecState->nextWordIsCharLiteral = true;
	return true;
}

bool PreBuiltWords::BuiltIn_FetchLiteral(ExecState* pExecState) {
	return ForthWord::BuiltInHelper_FetchLiteralWithOffset(pExecState, 0);
}

bool PreBuiltWords::BuiltIn_PushUpcomingLiteral(ExecState* pExecState) {
	WordBodyElement** ppWBE_Type = pExecState->GetNextWordFromPreviousNestedBodyAndIncIP();
	if (ppWBE_Type == nullptr) {
		return pExecState->CreateException("Push literal cannot find a literal type in word body");
	}
	WordBodyElement** ppWBE_Literal = pExecState->GetNextWordFromPreviousNestedBodyAndIncIP();
	if (ppWBE_Literal == nullptr) {
		return pExecState->CreateException("Push literal cannot find a literal in word body");
	}
	ForthType forthType = (*ppWBE_Type)->forthType;

	if (!pExecState->pStack->Push(new StackElement(forthType, ppWBE_Literal))) {
		return pExecState->CreateStackOverflowException();
	}
	return true;
}

bool PreBuiltWords::BuiltIn_UpdateForwardJump(ExecState* pExecState) {
	int64_t nCompileState = pExecState->GetIntTLSVariable(ExecState::c_compileStateIndex);
	
	if (nCompileState == 0) {
		return pExecState->CreateException("Cannot execute UPDATEFORWARDJUMP when not compiling");
	}
	return ForthWord::BuiltInHelper_UpdateForwardJump(pExecState);
}

bool PreBuiltWords::BuiltIn_ThrowException(ExecState* pExecState) {
	std::string str;
	bool success;
	tie(success, str) = pExecState->pStack->PullAsString();
	if (success) {
		return pExecState->CreateException(str.c_str());
	}
	else {
		return pExecState->CreateException("Could not retrieve string to make exception of");
	}
}

bool PreBuiltWords::BuiltIn_GetHighResolutionTime(ExecState* pExecState) {
	int64_t freq = _Query_perf_frequency();
	int64_t time = _Query_perf_counter();

	double elapsedSeconds = (double)time / (double)freq;
	if (!pExecState->pStack->Push(elapsedSeconds)) {
		return pExecState->CreateStackOverflowException("whilst getting high resolution time");
	}
	return true;
}