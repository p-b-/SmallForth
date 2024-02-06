#include "ForthDefs.h"
#include <iostream>
#include <sstream>
using namespace std;
#include "InputProcessor.h"
#include "ForthDict.h"
#include "ForthWord.h"
#include "DataStack.h"
#include "ReturnStack.h"
#include "ExecState.h"
#include "TypeSystem.h"
#include "ForthString.h"
#include "ForthFile.h"
#include "Vector3.h"
#include "PreBuiltWords.h"
#include "CompileHelper.h"
#include "ForthArray.h"

void initialiseDict(ForthDict* pDict,ExecState* pExecState);
bool interpretForth(ExecState* pExecState, const string& toExecute);
bool compileWordIntoWord(ForthDict* pDict, ForthWord* pForthWord, const string& wordName);
void createTypeWords(ExecState* pExecState);
void createFileTypes(ExecState* pExecState);
void initialiseWord(ForthDict* pDict, const string& wordName, XT wordCode);
void initialiseImmediateWord(ForthDict* pDict, const string& wordName, XT wordCode);
void initialiseTypeSystem(ExecState* pExecState);

int main()
{
    ForthDict* pDict = new ForthDict();
    pDict->IncReference();
    
    InputProcessor* pProcessor = new InputProcessor();
    DataStack* pDataStack = new DataStack(40);
    ReturnStack* pReturnStack = new ReturnStack(40);
    CompileHelper* pCompiler = new CompileHelper();
    ExecState* pExecState = new ExecState(pDataStack, pDict, pProcessor, pReturnStack, pCompiler);

    initialiseTypeSystem(pExecState);

    initialiseDict(pDict, pExecState);

    pProcessor->Interpret(pExecState);

    delete pProcessor;
    delete pDataStack;
    delete pExecState;

    pDict->DecReference();
}

void initialiseTypeSystem(ExecState* pExecState) {
    TypeSystem* ts = TypeSystem::GetTypeSystem();
    ts->RegisterValueType(pExecState, "undefined");
    ts->RegisterValueType(pExecState, "char");
    ts->RegisterValueType(pExecState, "int");
    ts->RegisterValueType(pExecState, "float");
    ts->RegisterValueType(pExecState, "bool");
    ts->RegisterValueType(pExecState, "type");
    ts->RegisterValueType(pExecState, "xt");
    ts->RegisterValueType(pExecState, "binaryopstype");
    ts->RegisterValueType(pExecState, "ptertocfa");

    ts->RegisterObjectType(pExecState, "word", nullptr);
    ts->RegisterObjectType(pExecState, "dict", nullptr);
    ts->RegisterObjectType(pExecState, "string", ForthString::Construct);
    ts->RegisterObjectType(pExecState, "readfile", ForthFile::ConstructReadFile);
    ts->RegisterObjectType(pExecState, "writefile", ForthFile::ConstructWriteFile);
    ts->RegisterObjectType(pExecState, "readwritefile", ForthFile::ConstructReadWriteFile);
    ts->RegisterObjectType(pExecState, "array", ForthArray::Construct);
    ts->RegisterObjectType(pExecState, "vector3", Vector3::Construct, Vector3::BinaryOps);
}

void initialiseDict(ForthDict* pDict, ExecState* pExecState) {
    // TODO Create INTERPRET word

    initialiseImmediateWord(pDict, "(", ForthWord::BuiltIn_ParenthesisCommentStart);
    initialiseImmediateWord(pDict, ")", ForthWord::BuiltIn_ParenthesisCommentEnd);
    initialiseImmediateWord(pDict, "\\", ForthWord::BuiltIn_LineCommentStart);
    initialiseWord(pDict, "docol", ForthWord::BuiltIn_DoCol);
    initialiseWord(pDict, "[docol]", ForthWord::BuiltIn_IndirectDoCol);
    initialiseWord(pDict, "immediate", ForthWord::BuiltIn_Immediate);
    initialiseWord(pDict, "execute", ForthWord::BuiltIn_Execute);
    initialiseWord(pDict, "executeonobject", ForthWord::BuiltIn_ExecuteOnObject);
    initialiseWord(pDict, "exit", ForthWord::BuiltIn_Exit);
    initialiseWord(pDict, "jump", ForthWord::BuiltIn_Jump); // ( addr -- )
    initialiseWord(pDict, "jumpontrue", ForthWord::BuiltIn_JumpOnTrue); // ( addr b -- )
    initialiseWord(pDict, "jumponfalse", ForthWord::BuiltIn_JumpOnFalse); // ( addr b -- )
    initialiseWord(pDict, "!inword", ForthWord::BuiltIn_PokeIntegerInWord); // (v addr -- )
    initialiseWord(pDict, "@", ForthWord::BuiltIn_Peek); // (
    initialiseWord(pDict, "!", ForthWord::BuiltIn_Poke);

    initialiseWord(pDict, "reveal", ForthWord::BuiltIn_Reveal);
    initialiseWord(pDict, "stackreveal", ForthWord::BuiltIn_RevealToStack);
    initialiseWord(pDict, "create", ForthWord::BuiltIn_Create);
    initialiseWord(pDict, "allot", PreBuiltWords::BuiltIn_Allot);
    initialiseWord(pDict, "word>object", ForthWord::BuiltIn_AddWordToObject);
    initialiseWord(pDict, "find", ForthWord::BuiltIn_Find);

    initialiseWord(pDict, "constant", ForthWord::BuiltIn_Constant);
    initialiseWord(pDict, "variable", ForthWord::BuiltIn_Variable);
    initialiseImmediateWord(pDict, "]", ForthWord::BuiltIn_StartCompilation);
    initialiseImmediateWord(pDict, "[", ForthWord::BuiltIn_EndCompilation);
    initialiseImmediateWord(pDict, "postpone", ForthWord::BuiltIn_Postpone);
    initialiseWord(pDict, "(postpone)", ForthWord::BuiltIn_Postpone);
    initialiseImmediateWord(pDict, "literal", ForthWord::BuiltIn_Literal);
    initialiseImmediateWord(pDict, ",", ForthWord::BuiltIn_Literal);
    initialiseImmediateWord(pDict, "[char]", ForthWord::BuiltIn_CharLiteral);

    initialiseWord(pDict, "pushliteral", ForthWord::BuiltIn_PushUpcomingLiteral);
    initialiseWord(pDict, "true", ForthWord::BuiltIn_True);
    initialiseWord(pDict, "false", ForthWord::BuiltIn_False);
    initialiseWord(pDict, "type", ForthWord::BuiltIn_PushType);

    initialiseWord(pDict, "dup", ForthWord::BuiltIn_Dup);
    initialiseWord(pDict, "swap", ForthWord::BuiltIn_Swap);
    initialiseWord(pDict, "drop", ForthWord::BuiltIn_Drop);
    initialiseWord(pDict, "over", ForthWord::BuiltIn_Over);
    initialiseWord(pDict, "rot", ForthWord::BuiltIn_Rot);
    initialiseWord(pDict, "-rot", ForthWord::BuiltIn_ReverseRot);
    initialiseWord(pDict, ">r", ForthWord::BuiltIn_PushDataStackToReturnStack);
    initialiseWord(pDict, "<r", ForthWord::BuiltIn_PushReturnStackToDataStack);
    initialiseWord(pDict, ">t", ForthWord::BuiltIn_PushDataStackToTempStack);
    initialiseWord(pDict, "<t", ForthWord::BuiltIn_PushTempStackToDataStack);

    initialiseWord(pDict, "+", ForthWord::BuiltIn_Add);
    initialiseWord(pDict, "-", ForthWord::BuiltIn_Subtract);
    initialiseWord(pDict, "*", ForthWord::BuiltIn_Multiply);
    initialiseWord(pDict, "/", ForthWord::BuiltIn_Divide);
    initialiseWord(pDict, "%", ForthWord::BuiltIn_Modulus);
    initialiseWord(pDict, "<", ForthWord::BuiltIn_LessThan);
    initialiseWord(pDict, "<=", ForthWord::BuiltIn_LessThanOrEquals);
    initialiseWord(pDict, "=", ForthWord::BuiltIn_Equals);
    initialiseWord(pDict, "!=", ForthWord::BuiltIn_NotEquals);
    initialiseWord(pDict, "<>", ForthWord::BuiltIn_NotEquals);
    initialiseWord(pDict, ">=", ForthWord::BuiltIn_GreaterThanOrEquals);
    initialiseWord(pDict, ">", ForthWord::BuiltIn_GreaterThan);

    initialiseWord(pDict, "not", ForthWord::BuiltIn_Not);
    initialiseWord(pDict, "and", ForthWord::BuiltIn_And);
    initialiseWord(pDict, "or", ForthWord::BuiltIn_Or);
    initialiseWord(pDict, "xor", ForthWord::BuiltIn_Xor);

    initialiseWord(pDict, "depth", ForthWord::BuiltIn_StackSize); // ( a1 .. an -- a1 .. an n )
    initialiseWord(pDict, "rdepth", ForthWord::BuiltIn_RStackSize); // ( R: a1 .. an -- R: a1 .. an n )
    initialiseWord(pDict, "tdepth", ForthWord::BuiltIn_TStackSize); // ( T: a1 .. an -- T: a1 .. an n )
    initialiseWord(pDict, ".", ForthWord::BuiltIn_PrintStackTop);
    initialiseWord(pDict, "emit", ForthWord::BuiltIn_Emit);
    initialiseWord(pDict, "`", ForthWord::BuiltIn_WordCFAFromInputStream);
    initialiseImmediateWord(pDict, "[`]", ForthWord::BuiltIn_WordCFAFromDefinition);
    initialiseImmediateWord(pDict, "here", ForthWord::BuiltIn_Here);
    initialiseWord(pDict, "(here)", ForthWord::BuiltIn_Here);
    initialiseWord(pDict, "forget", ForthWord::BuiltIn_Forget);
    initialiseWord(pDict, "see", ForthWord::BuiltIn_DescribeWord);

    initialiseImmediateWord(pDict, "begin", ForthWord::BuiltIn_Begin);
    initialiseImmediateWord(pDict, "until", ForthWord::BuiltIn_Until);
    initialiseImmediateWord(pDict, "again", ForthWord::BuiltIn_Again);
    initialiseImmediateWord(pDict, "while", ForthWord::BuiltIn_While);
    initialiseImmediateWord(pDict, "repeat", ForthWord::BuiltIn_Repeat);
    initialiseImmediateWord(pDict, "do", ForthWord::BuiltIn_Do);
    initialiseImmediateWord(pDict, "loop", ForthWord::BuiltIn_Loop);
    initialiseImmediateWord(pDict, "leave", ForthWord::BuiltIn_Leave);
    initialiseImmediateWord(pDict, "+loop", ForthWord::BuiltIn_PlusLoop);
    initialiseImmediateWord(pDict, "if", ForthWord::BuiltIn_If);
    initialiseImmediateWord(pDict, "then", ForthWord::BuiltIn_Then);
    initialiseImmediateWord(pDict, "else", ForthWord::BuiltIn_Else);

    initialiseImmediateWord(pDict, "does>", ForthWord::BuiltIn_Does);

    initialiseImmediateWord(pDict,"\"", ForthWord::BuiltIn_StringLiteral);
    initialiseWord(pDict, "call", ForthWord::BuiltIn_CallObject);
    initialiseWord(pDict, "construct", ForthWord::BuiltIn_Construct);
    initialiseWord(pDict, "define", ForthWord::BuiltIn_DefineObject);
    initialiseWord(pDict, "self", PreBuiltWords::PushSelf);

    initialiseWord(pDict, "#refcount", PreBuiltWords::PushRefCount);

    initialiseWord(pDict, "tochar", PreBuiltWords::ToChar);
    initialiseWord(pDict, "toint", PreBuiltWords::ToInt);
    initialiseWord(pDict, "tofloat", PreBuiltWords::ToFloat);
    initialiseWord(pDict, "wordtoint", PreBuiltWords::WordToInt);
    initialiseWord(pDict, "wordtofloat", PreBuiltWords::WordToFloat);
    initialiseWord(pDict, "isObject", PreBuiltWords::IsObject);
    initialiseWord(pDict, "isPter", PreBuiltWords::IsPter);
    initialiseWord(pDict, "#s", PreBuiltWords::ToString);

    ForthWord* pDefineWord = new ForthWord(":", ForthWord::BuiltIn_DoCol);
    compileWordIntoWord(pDict, pDefineWord, "create");
    compileWordIntoWord(pDict, pDefineWord, "]");
    compileWordIntoWord(pDict, pDefineWord, "exit");
    pDefineWord->SetWordVisibility(true);

    pDict->AddWord(pDefineWord);

    ForthWord* pEndWordDefinition = new ForthWord(";", ForthWord::BuiltIn_DoCol);
    compileWordIntoWord(pDict, pEndWordDefinition, "postpone");
    compileWordIntoWord(pDict, pEndWordDefinition, "exit");
    compileWordIntoWord(pDict, pEndWordDefinition, "[");
    compileWordIntoWord(pDict, pEndWordDefinition, "reveal");
    compileWordIntoWord(pDict, pEndWordDefinition, "does>"); // Enters runtime compile mode for the next word (puts the next word into the CFA)
    compileWordIntoWord(pDict, pEndWordDefinition, "docol"); // This has to be a built-in, not a second-level word
    compileWordIntoWord(pDict, pEndWordDefinition, "["); // does> entered compile state, this leaves it
    compileWordIntoWord(pDict, pEndWordDefinition, "exit");

    pEndWordDefinition->SetImmediate(true);
    pEndWordDefinition->SetWordVisibility(true);
    pDict->AddWord(pEndWordDefinition);
    pEndWordDefinition = nullptr;

    interpretForth(pExecState, "1 type type variable #compileForType");

    // Compiles word onto stack
    pEndWordDefinition = new ForthWord(";;", ForthWord::BuiltIn_DoCol);
    compileWordIntoWord(pDict, pEndWordDefinition, "postpone");
    compileWordIntoWord(pDict, pEndWordDefinition, "exit");
    compileWordIntoWord(pDict, pEndWordDefinition, "[");
    compileWordIntoWord(pDict, pEndWordDefinition, "stackreveal");
    compileWordIntoWord(pDict, pEndWordDefinition, "#compileForType");
    compileWordIntoWord(pDict, pEndWordDefinition, "@");
    compileWordIntoWord(pDict, pEndWordDefinition, "word>object");
    compileWordIntoWord(pDict, pEndWordDefinition, "does>"); // Enters runtime compile mode for the next word (puts the next word into the CFA)
    compileWordIntoWord(pDict, pEndWordDefinition, "docol"); // This has to be a built-in, not a second-level word
    compileWordIntoWord(pDict, pEndWordDefinition, "["); // does> entered compile state, this leaves it
    compileWordIntoWord(pDict, pEndWordDefinition, "exit");

    pEndWordDefinition->SetImmediate(true);
    pEndWordDefinition->SetWordVisibility(true);
    pDict->AddWord(pEndWordDefinition);

    ForthWord* pDefineWordForObject = new ForthWord("::", ForthWord::BuiltIn_DoCol);
    compileWordIntoWord(pDict, pDefineWordForObject, "#compileForType");
    compileWordIntoWord(pDict, pDefineWordForObject, "!");
    compileWordIntoWord(pDict, pEndWordDefinition, "postpone");
    compileWordIntoWord(pDict, pDefineWordForObject, "create");
    compileWordIntoWord(pDict, pEndWordDefinition, "postpone");
    compileWordIntoWord(pDict, pDefineWordForObject, "]");
    compileWordIntoWord(pDict, pDefineWordForObject, "exit");
    pDefineWordForObject->SetImmediate(true);
    pDefineWordForObject->SetWordVisibility(true);
    pDict->AddWord(pDefineWordForObject);


    interpretForth(pExecState, ": 1+ 1 + ;"); // ( m -- m+1 )
    interpretForth(pExecState, ": 1- 1 - ;"); // ( m -- m-1 )
    interpretForth(pExecState, ": 2+ 2 + ;"); // ( m -- m+2 )
    interpretForth(pExecState, ": 2- 2 - ;"); // ( m -- m-2 )

    interpretForth(pExecState, ": .\" postpone \" . ;"); // ( -- obj($) )

    interpretForth(pExecState, ": cr ( -- ) 10 emit ;"); // (  --  )
    interpretForth(pExecState, ": '\10' 10 tochar ; ");
    interpretForth(pExecState, ": pushNewLine '\10' ;");

    interpretForth(pExecState, ": dup2 dup dup ;"); // ( m -- m m)
    interpretForth(pExecState, ": nip swap drop ;"); // ( m n -- n)
    interpretForth(pExecState, ": tuck swap over ;"); // ( m n -- n m n)
    interpretForth(pExecState, ": 2dup over over ;"); // ( m n -- m n m n)
    interpretForth(pExecState, ": 2drop drop drop ;"); // ( m n -- )
    interpretForth(pExecState, ": 3drop 2drop drop ;"); // ( m n p -- )
    interpretForth(pExecState, ": rdup <r dup >r >r ;"); // ( R: a -- R: a a)
    interpretForth(pExecState, ": r@ <r dup >r ;"); // (R : a -- a R: a)
    interpretForth(pExecState, ": 2>r swap >r >r ;"); // ( a b -- R a b)
    interpretForth(pExecState, ": 2<r <r <r swap ;"); // ( R a b -- a b)
    interpretForth(pExecState, ": 2r@ 2<r 2dup 2>r ;"); // ( R a b -- a b R a b)

    interpretForth(pExecState, ": 2swap rot >t rot <t ;"); // ( a b c d -- c d a b)
    interpretForth(pExecState, ": 2over >t >t 2dup <t -rot <t -rot ;"); // ( a b c d -- a b c d a b)
    interpretForth(pExecState, ": 2nip rot drop rot drop ;"); // (a b c d -- c d)
    interpretForth(pExecState, ": 2tuck 2dup >t >t >t -rot <t -rot <t <t ;"); // (a b c d -- c d a b c d )
    interpretForth(pExecState, ": 2rot >t >t 2swap <t <t 2swap ;"); // ( a b c d e f -- c d e f a b )
    interpretForth(pExecState, ": -2rot 2swap >t >t 2swap <t <t ;");  // ( a b c d e f -- e f a b c d )

    interpretForth(pExecState, ": 3>r -rot 2>r >r ;");  // ( a b c  -- R a b c )
    interpretForth(pExecState, ": 3<r <r 2<r rot ;");  // ( a b c  -- R a b c )

    interpretForth(pExecState, ": space ( -- ) 32 emit ;"); // (  --  )
    interpretForth(pExecState, ": spaces ( n -- ) dup 0 > if 0 do 32 emit loop then ;"); // ( n -- )

    interpretForth(pExecState, ": I <r <r dup -rot >r >r ;");
    interpretForth(pExecState, ": unloop 3<r 3drop ;");


    // Helper words
    interpretForth(pExecState, ": ptop dup . cr ;"); // ( n -- n )
    interpretForth(pExecState, ": pss depth . cr ;"); // ( n -- n )
    interpretForth(pExecState, ": rprint2 <r ptop <r ptop >r >r ;");
    interpretForth(pExecState, ": emitall ( ch0 ... chn -- ) depth 0 != if depth 0 do emit loop then ;");
    interpretForth(pExecState, ": psd [char] R emit [char] : emit space rdepth . cr [char] T emit [char] : emit space tdepth . cr [char] S emit [char] : emit space depth . cr ;");

    // commented version does not check for 0-length stack
    //createWord(pExecState, ": .s depth dup 0 do swap ptop >t loop 0 do <t loop ;"); // ( [s] -- [s] shows stack )
    interpretForth(pExecState, ": .s depth dup dup 0 != if 0 do swap ptop >t loop 0 do <t loop else 2drop then ;"); // ( [s] -- [s] shows stack )
    interpretForth(pExecState, ": .s depth dup dup 0 != if 0 do swap dup #s . cr >t loop 0 do <t loop else 2drop then ;"); // ( [s] -- [s] shows stack )
    interpretForth(pExecState, ": .ts tdepth dup dup 0 != if >r 0 do <t ptop loop <r 0 do >t loop else 2drop then ;");
    interpretForth(pExecState, ": .rs rdepth dup dup 0 != if 0 do 3<r <r ptop >t 3>r loop 0 do 3<r <t >r 3>r loop else 2drop then ;");
    interpretForth(pExecState, ": clear depth dup 0 != if 0 do drop loop else drop then ; "); // ( [s] -- )
    interpretForth(pExecState, ": rclear begin rdepth 1 > while <r <r drop >r repeat ;");

    // fi : function index
    interpretForth(pExecState, "0 constant fi_getsize");
    interpretForth(pExecState, "1 constant fi_hash");
    interpretForth(pExecState, "2 constant fi_elementatindex");
    interpretForth(pExecState, "3 constant fi_setelementindex");
    interpretForth(pExecState, "4 constant fi_contains");
    interpretForth(pExecState, "5 constant fi_indexof");
    interpretForth(pExecState, "6 constant fi_subrange");
    interpretForth(pExecState, "7 constant fi_deconstruct");
    interpretForth(pExecState, "8 constant fi_append");
    interpretForth(pExecState, "9 constant fi_implements");
    interpretForth(pExecState, "10 constant fi_close");
    interpretForth(pExecState, "11 constant fi_read");
    interpretForth(pExecState, "12 constant fi_readLine");
    interpretForth(pExecState, "13 constant fi_readChar");
    interpretForth(pExecState, "14 constant fi_eof");

    interpretForth(pExecState, ": len ( object -- length ) fi_getsize swap call ;");
    interpretForth(pExecState, ": [n] ( n object -- nth element ) fi_elementatindex swap call ;"); 
    interpretForth(pExecState, ": [n]= ( element n object -- ) fi_setelementindex swap call ;");
    interpretForth(pExecState, ": appendelement ( element object -- ) dup len swap [n]= ;");
    interpretForth(pExecState, ": prependelement ( element object -- ) -1 swap [n]= ;");
    interpretForth(pExecState, ": contains ( element object -- b ) fi_contains swap call ;");
    interpretForth(pExecState, ": indexof ( element object -- n ) 0 -rot fi_indexof swap call ;");
    interpretForth(pExecState, ": indexof_from ( n element object -- n ) fi_indexof swap call ;");
    interpretForth(pExecState, ": substring ( n m object : objectn...objectm ) fi_subrange swap call ;");
    interpretForth(pExecState, ": deconstruct ( object -- [object state values] ) fi_deconstruct swap call ;");
    interpretForth(pExecState, ": append ( element object -- ) fi_append swap call ;");
    interpretForth(pExecState, ": close ( element object -- ) fi_close swap call ;");
    interpretForth(pExecState, ": read ( object -- $ ) fi_read swap call ; ");
    interpretForth(pExecState, ": readline ( object -- $ ) fi_readline swap call ; ");
    interpretForth(pExecState, ": readChar ( object -- $ ) fi_readchar swap call ; ");
    interpretForth(pExecState, ": eof ( object -- $ ) fi_eof swap call ; ");

    createTypeWords(pExecState);
    createFileTypes(pExecState);

    // Postpone is used before a word that needs to be compiled into the defining word, that is immediate
    // 
    // : do (postpone) 2>r (here) ; immediate
    // : loop postpone , (postpone) 2<r (postpone) 1+ (postpone) 2dup (postpone) != (postpone) -rot (postpone) 2>r (postpone) jumpontrue (postpone) 2<r (postpone) 2drop ; immediate

    // : do (postpone) 2>r (here) ; immediate
    // : do                                                 -- create a dictionary entry for create, start compile mode
    //      (postpone)                                      -- compile postpone into new word (that is getting the loop)
    //                 2>r                                  -- compile 2>r into word getting loop
    //                                                         (move integer pair from stack to return stack  (max iteration -- R:max iteration) when running new word)
    //                     (here)                           -- compile word here into do (equivalent to postpone here).  This will execute when do is being execute in newly defined word
    //                            ; immediate               -- end compilation, reveal work to dictionary, and mark the word as immeidate

    // : loop postpone , (postpone) 2<r +1 2dup != -rot (postpone) 2>r (postpone) jumpontrue 2<r 2drop ; immediate
    //
    // : loop postpone , (postpone) 2<r  
    // : loop                                               -- create a dictionary entry for loop
    //        postpone ,                                    -- compile , into word being created (takes top of stack and compiles it into word)
    //                                                         this should be here address pushed to TOS when whatever loop adds to word is ran
    //                   (postpone)                         -- compile postpone into new word (that is getting the loop)
    //                              2<r                     -- compile 2<r into new word (that is getting the loop).  When the word is run, the pair
    //                                                         on the return stack will be moved onto the data stack
    // 
    // 1+ (postpone) 2dup (postpone) != (postpone) -rot
    // 
    // 1+                                                   -- compile inc1 into the loop word
    //    (postpone) 2dup                                   -- compile postpone and 2dup into loop, which when run will compile 2dup into the new word
    //                    (postpone) !=                     -- compile postpone and != into the loop, which when run will compile != into the new word
    //                                  (postpone) -rot     -- compile postpone and -rot into the loop, which, when run will compile -rot into the new word
    //                                                         rotate the stack ( addr max n flag : addr flag max n)
    //
    // (postpone) 2>r (postpone) jumpontrue (postpone) 2<r
    // 
    // (postpone) 2>r                                       -- compile postpone and 2>r into the loop word, which when run will compile 2>r into the new word
    //                                                         ( addr flag max n -- addr flag R: max n)
    //                (postpone) jumpontrue                 -- compile postpone and jumpontrue into the loop word, which when run will compile jumpontrue into the new word
    //                                                         if the flag at the tos is true, jump back to the address second in the stack (addr f -- )
    //                                      (postpone) 2<r  -- compile postpone and 2<r into the loop word, which when run will compile 2<r into the new word
    //                                                         remove the max:iter flags from the return stack and drop them
    // 
    // (postpone) 2drop ; immediate
    // 
    // (postpone) 2drop                                     -- compile postpone and 2drop into the loop word, which when run will compile 2drop into the new word
    //                  ; immediate                         -- end compilation, reveal word to dictionary, and mark it as immediate




    // Definition of if, then
    // : if postpone here dup >r postpone , (postpone) swap (postpone) postpone jumponfalse ; immediate
    // : then < r + 1 postpone here !inword; immediate
    // : else postpone here dup <r swap >r > r postpone tos > literal(postpone) jump postpone then; immediate

    // 'then' updates a literal for a branch that is currently on the return stack when 'then' is executed, so it can be executed
    //  by else to update whatever literal is pointed at by the top element of the return stack

    // Define:
    // showstack changes prompt to show the top items of the stack between prommpts
    // pick ( a0 .. an n -- a0 .. an a0 )
    // roll (a0 ..an n -- a1 ..an a0)
}

bool interpretForth(ExecState* pExecState, const string &toExecute) {
    pExecState->pInputProcessor->SetInputString(toExecute);
    return pExecState->pInputProcessor->Interpret(pExecState);
}

bool compileWordIntoWord(ForthDict* pDict, ForthWord* pForthWord, const string& wordName) {
    ForthWord* pAdd = pDict->FindWord(wordName);
    if (pAdd == nullptr) {
        return false;
    }
    pForthWord->CompileCFAPterIntoWord(pAdd->GetPterToBody());

    return true;
}

void createTypeWords(ExecState* pExecState) {
    initialiseWord(pExecState->pDict, "typefromint", ForthWord::BuiltIn_TypeFromInt); // (v addr -- )
    stringstream ss;
    ss << ": word_type " << ObjectType_Word << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": dict_type " << ObjectType_Dict<< " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": string_type " << ObjectType_String << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": readfile_type " << ObjectType_ReadFile << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": writefile_type " << ObjectType_WriteFile << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": readwritefile_type " << ObjectType_ReadWriteFile << " typefromint ; ";
    interpretForth(pExecState, ss.str());
    ss.str(string());

    ss << ": array_type " << ObjectType_Array << " typefromint ; ";
    interpretForth(pExecState, ss.str());

    ss << ": vector3_type " << ObjectType_Vector3 << " typefromint ; ";
    interpretForth(pExecState, ss.str());
}

void createFileTypes(ExecState* pExecState) {
    stringstream ss;
    ss << ": #stdout " << forth_stdout << " ; ";
    interpretForth(pExecState, ss.str());

    ss.str(string());
    ss << ": #stderr " << forth_stderr << " ; ";
    interpretForth(pExecState, ss.str());

    ss.str(string());
    ss << ": #stdin " << forth_stdin << " ; ";
    interpretForth(pExecState, ss.str());

    ss.str(string());

    interpretForth(pExecState, "#stdout writefile_type construct constant stdout");
    interpretForth(pExecState, "#stderr writefile_type construct constant stderr");
    interpretForth(pExecState, "#stdin readfile_type construct constant stdin");
}

void initialiseWord(ForthDict* pDict, const string& wordName, XT wordCode) {
    ForthWord* pForthWord = new ForthWord(wordName, wordCode);
    pForthWord->SetWordVisibility(true);
    pDict->AddWord(pForthWord);
}

void initialiseImmediateWord(ForthDict* pDict, const string& wordName, XT wordCode) {
    ForthWord* pForthWord = new ForthWord(wordName, wordCode);
    pForthWord->SetImmediate(true);
    pForthWord->SetWordVisibility(true);
    pDict->AddWord(pForthWord);
}