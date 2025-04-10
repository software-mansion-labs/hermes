/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IRGEN_ESTREEIRGEN_H
#define HERMES_IRGEN_ESTREEIRGEN_H

#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Sema/FlowContext.h"
#include "hermes/Sema/Keywords.h"
#include "hermes/Sema/SemContext.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/Debug.h"

// Use this value to enable debug logging from the command line.
#define DEBUG_TYPE "irgen"

using llvh::dyn_cast_or_null;

namespace hermes {
namespace irgen {

// Forward declarations
class SurroundingTry;
class ESTreeIRGen;

//===----------------------------------------------------------------------===//
// Free standing helpers.

/// Return the name field from ID nodes.
inline Identifier getNameFieldFromID(const ESTree::Node *ID) {
  return Identifier::getFromPointer(cast<ESTree::IdentifierNode>(ID)->_name);
}

/// \returns true if \p node is known to be a constant expression.
bool isConstantExpr(ESTree::Node *node);

//===----------------------------------------------------------------------===//
// FunctionContext

/// Holds the target basic block for a break and continue label.
/// It has a 1-to-1 correspondence to SemInfoFunction::GotoLabel.
/// Labels can be context dependent - one label can be initialized multiple
/// times if it is defined inside a finally block, because the body of the
/// finally block will be visited and compiled multiple times from different
/// contexts (on normal exit, on exceptional exit, on break/return/etc).
struct GotoLabel {
  BasicBlock *breakTarget = nullptr;
  BasicBlock *continueTarget = nullptr;
  /// A record on the stack defining the closest surrounding try/catch
  /// statement. We need this so we can call the finally handlers.
  SurroundingTry *surroundingTry = nullptr;
};

/// This struct holds information which is passed down through certain capturing
/// functions, such as arrow functions.
struct CapturedState {
  /// Parents of arrow functions need to capture their "this" parameter so the
  /// arrow function can use it. Normal functions and base class constructors
  /// store their "this" in a variable and record it here, if they contain at
  /// least one arrow function. Derived class constructors maintain a 'checked
  /// this' which is only initialized after a completed call to super(). Arrow
  /// functions always copy their parent's value.
  Variable *thisVal;
  /// Captured value of new target. In ES5 functions and ES6 constructors it is
  /// a Variable with the result of GetNewTargetInst executed at the start of
  /// the function. In ES6 methods it will be initialized to LiteralUndefined.
  /// In arrow functions it is a copy of the parent's capturedState.newTarget.
  Value *newTarget;
  /// Optionally captured value of the eagerly created Arguments object. Used
  /// when arrow functions need to access it.
  Variable *arguments;
  /// 10.2 ECMAScript Function Objects [[HomeObject]]: This is the object whose
  /// [[GetPrototypeOf]] provides the object where super property lookups begin.
  Variable *homeObject = nullptr;
};

// A context structure (pushed/popped within class declarations) for
// conveying info about the current typed class, for use in compiling
// constructors and related implicit methods.
struct TypedClassContext {
  /// The declaration node of the innermost typed class we are compiliing.
  ESTree::ClassDeclarationNode *node = nullptr;

  /// The type of the innermost typed class we are compiliing.
  flow::ClassType *type = nullptr;
};

/// Holds class state.
struct LegacyClassContext {
  /// Variable containing the class constructor of the enclosing class of the
  /// method/constructor being generated, nullptr if none available.
  Variable *constructor;

  /// Variable containing the instance elements initializer closure.
  Variable *instElemInitFuncVar;

  /// Map from a ClassPropertyNode with a computed key to that key's value.
  llvh::DenseMap<ESTree::ClassPropertyNode *, Variable *>
      classComputedFieldKeys{};

  explicit LegacyClassContext(Variable *cons, Variable *funcVar)
      : constructor(cons), instElemInitFuncVar(funcVar) {}
};

/// Holds per-function state, specifically label tables. Should be constructed
/// on the stack. Upon destruction it automatically restores the previous
/// function context.
class FunctionContext {
  /// Pointer to the "outer" object this is associated with.
  ESTreeIRGen *const irGen_;

  /// Semantic info of the function we are emitting.
  sema::FunctionInfo *const semInfo_;

  /// The old value which we save and will restore on destruction.
  FunctionContext *oldContext_;

  /// As we descend into a new function, we save the state of the builder
  /// here. It is automatically restored once we are done with the function.
  IRBuilder::SaveRestore builderSaveState_;

  /// A vector of labels corresponding 1-to-1 to the labels defined in
  /// \c semInfo_.
  llvh::SmallVector<GotoLabel, 2> labels_;

  /// A map from an AST node to the VariableScope associated with it. This is
  /// used to ensure that if the node is emitted multiple times (due to
  /// unrolling or finally blocks), we don't create multiple scopes. Note that
  /// this is not needed for the function scope, since that is only emitted
  /// once.
  llvh::DenseMap<ESTree::Node *, VariableScope *> innerScopes_;

 public:
  /// This is the actual function associated with this context.
  Function *const function;

  /// The innermost surrounding try/catch node at any point.
  SurroundingTry *surroundingTry = nullptr;

  /// Superclass of a method being generated, nullptr if none available.
  ESTree::Node *superClassNode_ = nullptr;

  /// Whether this function has already called super() constructor.
  /// TODO: Allow branches with super() calls.
  bool calledSuperConstructor_ = false;

  /// The number of nested contexts in which we are compiling a given AST for
  /// after it has already been compiled. We compile an AST twice when it is
  /// the body of a "finally" handler, or when it is a loop pre-condition.
  ///
  /// This counter is used only for debugging and is checked like a flag. If
  /// it is non-zero, then we will encounter declarations that are already
  /// bound to an IR value.
#ifndef NDEBUG
  unsigned debugAllowRecompileCounter = 0;

  /// Increment \c debugAllowRecompileCounter on entry and restore it on exit.
  class AllowRecompileRAII {
    FunctionContext &outer_;

   public:
    explicit AllowRecompileRAII(FunctionContext &outer) : outer_(outer) {
      ++outer_.debugAllowRecompileCounter;
    }
    ~AllowRecompileRAII() {
      --outer_.debugAllowRecompileCounter;
    }
  };
#else
  class AllowRecompileRAII {
   public:
    explicit AllowRecompileRAII(FunctionContext &) {}
  };
#endif

  /// Stack Register that will hold the return value of the global scope.
  AllocStackInst *globalReturnRegister{nullptr};

  /// A running counter for anonymous closure. We append this number to some
  /// label to create a unique number;
  size_t anonymousLabelCounter{0};

  /// This holds the CreateArguments instruction. We always insert it in the
  /// prologue and delete it in the epilogue if it wasn't used.
  CreateArgumentsInst *createArgumentsInst{};

  /// The instruction loading every parameter. Index 0 is "this", coerced to
  /// object if necessary.
  /// We always generate them in the prologue. The load of "this" is deleted
  /// when emitting the epilogue if not used.
  llvh::SmallVector<Instruction *, 4> jsParams{};

  /// Holds state that needs to be persisted between arrow functions.
  CapturedState capturedState{};

  /// The set of TDZ variables whose declaration initialization has completed.
  /// If a TDZ variable is accessed in its own function before it is
  /// initialized (not present in the set), we can directly throw. If a variable
  /// is accessed in its own function after it is initialized (present in the
  /// set), there is no need for a check.
  ///
  /// In short, this set eliminates all TDZ checks when accessing variables in
  /// their declaring function.
  llvh::DenseSet<Variable *> initializedTDZVars{};

  /// The CreateScopeInst that creates the scope for this function.
  CreateScopeInst *curScope{};

  /// Information about the current enclosing typed class.
  TypedClassContext typedClassContext{};

  /// Information about the current enclosing legacy class. We make this a
  /// shared_ptr so that we don't copy it by value when we need to enqueue
  /// contained methods for compilation. It is initialized to nullptr here, and
  /// is only set to some heap allocated value at the beginning of generating IR
  /// for a class node. As we generate the IR for the methods of the class, the
  /// shared_ptr is handed out to all of them when enqueuing them for
  /// compilation. Thus, the memory can only be freed once we exit the IR
  /// generation for the top-level class node, and once we finish generating IR
  /// for all of the functions contained within the class node.
  std::shared_ptr<LegacyClassContext> legacyClassContext{nullptr};

  /// Initialize a new function context, while preserving the previous one.
  /// \param irGen the associated ESTreeIRGen object.
  /// \param function the newly created Function IR node.
  /// \param semInfo semantic info obtained from the AST node.
  FunctionContext(
      ESTreeIRGen *irGen,
      Function *function,
      sema::FunctionInfo *semInfo);

  ~FunctionContext();

  /// \return the associated semantic information, asserting that it is
  /// present.
  sema::FunctionInfo *getSemInfo() {
    assert(semInfo_ && "semInfo is not set");
    return semInfo_;
  }

  /// \return true if there is an active typed class context held in this
  /// function context.
  bool hasTypedClassContext() const {
    // The class node is always set when a typed class is encountered, so we
    // can reliably only test this field.
    return typedClassContext.node;
  }

  /// \return true if there is an active legacy class context held in this
  /// function context.
  bool hasLegacyClassContext() const {
    return legacyClassContext != nullptr;
  }

  /// Generate a unique string that represents a temporary value. The string
  /// \p hint appears in the name.
  Identifier genAnonymousLabelName(llvh::StringRef const hint);

  /// Initialize an empty goto label. All labels in JavaScript a structured, so
  /// a label is guaranteed to be visited and initialized before it is used.
  /// Perhaps surprisingly, a label will be initialized more than once if it is
  /// defined in a finally block, since the statements inside the finally block
  /// will be visited (and compiled) more than once.
  void initLabel(
      ESTree::LabelDecorationBase *LDB,
      BasicBlock *breakTarget,
      BasicBlock *continueTarget) {
    auto &label = labels_[LDB->getLabelIndex()];
    label.breakTarget = breakTarget;
    label.continueTarget = continueTarget;
    label.surroundingTry = surroundingTry;
  }

  /// Get the VariableScope associated with the given \p node, or create one if
  /// it does not exist. \c curScope is used as the parent scope.
  VariableScope *getOrCreateInnerVariableScope(ESTree::Node *node);

  /// Access an already initialized label.
  const GotoLabel &label(ESTree::LabelDecorationBase *LDB) {
    auto &label = labels_[LDB->getLabelIndex()];
    assert(
        (label.breakTarget || label.continueTarget) &&
        "accessing an uninitialized label");
    return label;
  }
};

enum class ControlFlowChange { Break, Continue };

/// A link in the stack of surrounding try/catch statements we are maintaining
/// at any time.
class SurroundingTry {
  FunctionContext *const functionContext_;

 public:
  /// The record of the outer (closest surrounding) try/catch.
  SurroundingTry *const outer;
  /// The surrounding scope of the try/catch statement. This is used to ensure
  /// that finally blocks execute in the same scope.
  CreateScopeInst *const scope;
  /// The ESTree node defining the try/catch. This is usually a
  /// TryStatementNode, but in some cases that require internal try/catch
  /// statements, it can be something different.
  ESTree::Node *const node;
  /// The catch block of the try statement.
  BasicBlock *catchBlock;
  /// Optional debug location to be used for the TryEnd instruction.
  SMLoc const tryEndLoc{};

  /// \param continueTarget if cfc is Continue, then the target of that
  /// continue, which is used to determine whether to emit 'finally' code in
  /// certain cases such as for-of.
  using GenFinalizerCB = std::function<
      void(ESTree::Node *, ControlFlowChange cfc, BasicBlock *continueTarget)>;

  /// An optional callback that will be invoked to generate the code for the
  /// finalizer.
  GenFinalizerCB genFinalizer{};

  SurroundingTry(
      FunctionContext *functionContext,
      ESTree::Node *node,
      BasicBlock *catchBlock,
      SMLoc tryEndLoc = {},
      GenFinalizerCB genFinalizer = {})
      : functionContext_(functionContext),
        outer(functionContext->surroundingTry),
        scope(functionContext->curScope),
        node(node),
        catchBlock(catchBlock),
        tryEndLoc(tryEndLoc),
        genFinalizer(std::move(genFinalizer)) {
    // Push.
    functionContext->surroundingTry = this;
  }

  ~SurroundingTry() {
    // Pop.
    functionContext_->surroundingTry = outer;
  }
};

//===----------------------------------------------------------------------===//
// LReference

/// This is a utility class that's related to the ES5 Reference type without
/// the runtime semantic parts - https://es5.github.io/#x8.7.
class LReference {
 public:
  enum class Kind {
    /// Elision in destructuring pattern.
    Empty,
    /// Reference to a member expression.
    Member,
    /// Reference to a variable or a global property.
    VarOrGlobal,
    /// Invalid reference. Error has been reported, so it does nothing.
    Error,
    /// Destructuring assignment target.
    Destructuring,
  };

  LReference(
      Kind kind,
      ESTreeIRGen *irgen,
      bool declInit,
      ESTree::Node *ast,
      Value *base,
      Value *property,
      SMLoc loadLoc,
      Value *thisVal = nullptr)
      : kind_(kind), irgen_(irgen), declInit_(declInit) {
    ast_ = ast;
    base_ = base;
    property_ = property;
    thisValue_ = thisVal;
    loadLoc_ = loadLoc;
  }

  LReference(ESTreeIRGen *irgen, bool declInit, ESTree::PatternNode *target)
      : kind_(Kind::Destructuring), irgen_(irgen), declInit_(declInit) {
    destructuringTarget_ = target;
  }

  bool isEmpty() const {
    return kind_ == Kind::Empty;
  }

  Value *emitLoad();
  void emitStore(Value *value);

  /// \return true if it is known that \c emitStore() will not have any side
  ///   effects, including exceptions. This is not a sophisticated analysis,
  ///   it only checks for a local variable.
  bool canStoreWithoutSideEffects() const;

  Variable *castAsVariable() const;
  GlobalObjectProperty *castAsGlobalObjectProperty() const;

 private:
  /// Self explanatory.
  Kind kind_;

  /// The associated instance of ESTreeIRGen.
  ESTreeIRGen *irgen_;

  /// This is a declaration initialization, so the TDZ check should be skipped
  /// when storing.
  bool declInit_;

  union {
    struct {
      /// The base AST node.
      ESTree::Node *ast_;

      /// The base of the object, or the variable we load from.
      Value *base_;
      /// The name/value of the field this reference accesses, or null if this
      /// is a variable access.
      Value *property_;

      /// Corresponds to [[ThisValue]] in ES15 6.2.5 Reference Record. Only
      /// populated for super references.
      Value *thisValue_;
    };

    /// Destructuring assignment target.
    ESTree::PatternNode *destructuringTarget_;
  };

  /// Debug position for loads. Must be outside of the union because it has a
  /// constructor.
  SMLoc loadLoc_;

  /// \return a reference to IRGen's builder.
  IRBuilder &getBuilder();
};

//===----------------------------------------------------------------------===//
// ESTreeIRGen

/// Performs lowering of the JSON ESTree down to Hermes IR.
class ESTreeIRGen {
  friend class FunctionContext;
  friend class LReference;

  using BasicBlockListType = llvh::SmallVector<BasicBlock *, 4>;

  /// The module we are constructing.
  Module *Mod;
  /// Semantic resolution tables.
  sema::SemContext &semCtx_;
  /// Keywords to avoid string content comparisons.
  sema::Keywords &kw_;
  /// Flow types context.
  flow::FlowContext &flowContext_;
  /// The root of the ESTree.
  ESTree::Node *Root;
  /// The IRBuilder we use to construct the module.
  IRBuilder Builder;
  /// This points to the current function's context. It is saved and restored
  /// whenever we enter a nested function.
  FunctionContext *functionContext_{};

  /// The type of the key stored in \c compiledEntities_. It is a pair of an
  /// AST node pointer and an extra key, which is a small integer value.
  /// The purpose of the extra key is to enable us to distinguish different
  /// "compiled entities" associated with the same AST node. For example,
  /// "outer" and "inner" generator function.
  using CompiledMapKey = llvh::PointerIntPair<ESTree::Node *, 2>;

  /// An "additional key" when mapping from an AST node to a compiled Value,
  /// used to distinguish between different values that may be associated with
  /// the same node.
  enum class ExtraKey : unsigned {
    // These apply for function-like nodes.
    Normal = 0,
    GeneratorOuter = 1,
    AsyncOuter = 2,
    // These apply for class-like nodes.
    ImplicitClassConstructor = 0,
    ImplicitFieldInitializer = 1,
    ImplicitStaticElementsInitializer = 2,
  };

  /// Map from an AST node to a "compiled entity", which is usually a Function.
  /// This prevents multiple compilation of the same function expression when
  /// we compile the surrounding AST more than once.
  ///
  /// With generators and asyc functions, more than one function can be
  /// associated with the same AST node. We use the "extra key" to distinguish
  /// between the different cases.
  ///
  /// Initially we create an empty (forward-declared) Function and associate
  /// it with the AST in this map. It is also added to the compilation queue,
  /// so it is eventually compiled.
  llvh::DenseMap<CompiledMapKey, Value *> compiledEntities_{};

  /// Data about the IR representation of a class type.
  struct ClassConstructorInfo {
    /// Constructor function for the class.
    /// Used for populating Construct targets
    Function *constructorFunc;

    /// The variable containing the ".prototype" object, used as the vtable.
    Variable *homeObjectVar;

    ClassConstructorInfo(Function *constructorFunc, Variable *homeObjectVar)
        : constructorFunc(constructorFunc), homeObjectVar(homeObjectVar) {}
  };

  /// Map from a class type to IR information about the class.
  llvh::DenseMap<flow::ClassType *, ClassConstructorInfo> classConstructors_{};

  struct ClassFieldInitInfo {
    // For classes with field initializations:
    //  * The Function for the implicit field init function.
    Function *fieldInitFunction{};
    //  * The variable that holds the closure value of the implicit field init
    //    function.
    Variable *fieldInitFunctionVar{};

    ClassFieldInitInfo() = default;
  };

  /// Map from a class type to information related to field initializations for
  /// the class.
  llvh::DenseMap<flow::ClassType *, ClassFieldInitInfo> classFieldInitInfo_{};

  /// Map from a field on the home object to the IR function that it is
  /// guaranteed to be, if any.
  llvh::DenseMap<const flow::ClassType::Field *, Function *> finalMethods_{};

  /// A queue of "entities" that have been forward declared and mapped in
  /// \c compiledEntities_, but need to be actually compiled. This makes the
  /// compiler non-recursive.
  std::deque<std::function<void()>> compilationQueue_{};

  /// Identifier representing the string "?default".
  const Identifier identDefaultExport_;

  /// Generate a unique string that represents a temporary value. The string \p
  /// hint appears in the name.
  Identifier genAnonymousLabelName(llvh::StringRef hint) {
    return curFunction()->genAnonymousLabelName(hint);
  }

  /// Generate a unique string that represents a temporary value. The string \p
  /// hint appears in the name.
  Identifier genAnonymousLabelName(Identifier hint) {
    return genAnonymousLabelName(hint.isValid() ? hint.str() : "anonymous");
  }

  /// Convert a property key node to its JavaScript string representation.
  static llvh::StringRef propertyKeyAsString(
      llvh::SmallVectorImpl<char> &storage,
      ESTree::Node *Key);

 public:
  explicit ESTreeIRGen(
      Module *M,
      sema::SemContext &semCtx,
      flow::FlowContext &flowContext,
      ESTree::Node *root);

  /// Perform IRGeneration for the whole module.
  /// \param topLevelFunctionName the name of the top-level function.
  void doIt(llvh::StringRef topLevelFunctionName);

  /// Perform IRGeneration for the whole module.
  /// \param evalDataInst must not contain a null varScope.
  Function *doItInScope(EvalCompilationDataInst *evalDataInst);

  /// Perform IR generation for a given CJS module.
  void doCJSModule(
      sema::SemContext &semContext,
      uint32_t segmentID,
      uint32_t id,
      llvh::StringRef filename);

  /// Perform IR generation for a single lazy function, potentially adding more
  /// Functions to the Module.
  /// \param lazyFunc the lazy function for which to generate new IR.
  /// \return a newly created IR Function to replace the lazy function.
  Function *doLazyFunction(Function *lazyFunc);

  /// Generate a function which immediately throws the specified SyntaxError
  /// message.
  Function *genSyntaxErrorFunction(
      Module *M,
      Identifier originalName,
      SMRange sourceRange,
      llvh::StringRef error);

 private:
  /// @name statements
  /// @{

  /// \brief Perform IRGen for \p body, which can be the body of a function or
  /// the top-level of a program.
  void genBody(ESTree::NodeList &Body);

  /// Generate code for the statement \p Stmt.
  void genStatement(ESTree::Node *stmt);

  /// Wrapper of genExpression. If curFunction()->globalReturnRegister is
  /// set, stores the expression value into it.
  void genExpressionWrapper(ESTree::Node *expr);

  void genVariableDeclaration(ESTree::VariableDeclarationNode *declaration);
  void genVariableDeclarator(
      ESTree::NodeLabel kind,
      ESTree::VariableDeclaratorNode *declarator);

  void genIfStatement(ESTree::IfStatementNode *IfStmt);
  void genReturnStatement(ESTree::ReturnStatementNode *RetStmt);
  void genForInStatement(ESTree::ForInStatementNode *ForInStmt);
  void genForOfStatement(ESTree::ForOfStatementNode *forOfStmt);
  void genForOfFastArrayStatement(
      ESTree::ForOfStatementNode *forOfStmt,
      flow::ArrayType *type);
  void genWhileLoop(ESTree::WhileStatementNode *loop);
  void genDoWhileLoop(ESTree::DoWhileStatementNode *loop);

  /// A conservative check of whether the AST tree \p tree captures any
  /// variables. This simply checks whether the expression creates any closures.
  bool treeDoesNotCapture(ESTree::Node *tree);
  /// Compile a for-loop.
  void genForLoop(ESTree::ForStatementNode *loop);
  /// General case for compiling a for loop which may have captures in the
  /// init/test/update expressions.
  void genScopedForLoop(ESTree::ForStatementNode *loop);

  void genSwitchStatement(ESTree::SwitchStatementNode *switchStmt);

  /// Check if all cases values are constant and stores them in
  /// \p casdeLiterals. The default case, if present, is stored as nullptr, to
  ///     maintain the ordering of the cases.
  /// \returns true if all cases are constant.
  bool areAllCasesConstant(
      ESTree::SwitchStatementNode *switchStmt,
      llvh::SmallVectorImpl<Literal *> &caseLiterals);

  /// Generate IR Switch statements when all case tests are constant.
  void genConstSwitchStmt(
      ESTree::SwitchStatementNode *switchStmt,
      llvh::SmallVectorImpl<Literal *> &caseLiterals);

  void genImportDeclaration(ESTree::ImportDeclarationNode *importDecl);

  void genExportNamedDeclaration(
      ESTree::ExportNamedDeclarationNode *exportDecl);
  void genExportDefaultDeclaration(
      ESTree::ExportDefaultDeclarationNode *exportDecl);
  void genExportAllDeclaration(ESTree::ExportAllDeclarationNode *exportDecl);

  void genClassDeclaration(ESTree::ClassDeclarationNode *node);

  // Assumes that the class of the current class context has no
  // explicit constructor.  The \p consName argument is the
  // constructor name for that class.  If the class has a superclass,
  // \p superClass is non-null and is a reference to that class.
  // Generates code for the implicit constructor, and emits and
  // returns the instruction to create a closure object for it.
  CreateFunctionInst *genTypedImplicitConstructor(
      const Identifier &consName,
      Value *superClass);

  /// Generate IR for a legacy class declaration. This just forwards into
  /// genLegacyClassLike.
  void genLegacyClassDeclaration(ESTree::ClassDeclarationNode *node);

  /// Generate IR for a legacy class expression. This just forwards into
  /// genLegacyClassLike.
  Value *genLegacyClassExpression(
      ESTree::ClassExpressionNode *node,
      Identifier nameHint);

  /// Generate IR for legacy classes or declarations. This iterates through the
  /// entire class body and processes each class element.
  CreateClassInst *genLegacyClassLike(
      ESTree::ClassLikeNode *classNode,
      Identifier nameHint);

  /// Generate IR for a direct call to super.
  Value *genLegacyDirectSuper(ESTree::CallExpressionNode *call);

  /// Generate IR for `this` in a derived legacy class constructor.
  Value *genLegacyDerivedThis();

  /// Generate IR for a `return` statement in a derived legacy class
  /// constructor. This makes sure that \p returnValue is undefined or an
  /// object, and performs the checks on `this` when required.
  /// This corresponds mostly closely with ES15 10.2.2 [[Construct]].
  Value *genLegacyDerivedConstructorRet(
      ESTree::ReturnStatementNode *node,
      Value *returnValue);

  /// Generate function code for a function that initializes all instance
  /// elements in a legacy class.
  NormalFunction *genLegacyInstanceElementsInit(
      ESTree::ClassLikeNode *legacyClassNode,
      Variable *homeObjectVar,
      const Identifier &consName);

  /// Emit IR to invoke the instance elements initializer function for a legacy
  /// class.
  void emitLegacyInstanceElementsInitCall();

  /// Generate function code for a function that initializes all static elements
  /// in a legacy class.
  NormalFunction *genStaticElementsInitFunction(
      ESTree::ClassLikeNode *legacyClassNode,
      const Identifier &consName);

  /// Generate the function code for the implicit constructor of a legacy class.
  /// \param superClassNode is non-null for constructors of a derived class.
  NormalFunction *genLegacyImplicitConstructor(
      ESTree::ClassLikeNode *classNode,
      const Identifier &className,
      ESTree::Node *superClassNode);

  /// Emit code to allocate an empty instance of the specified class and return
  /// it.
  /// \param parent the parent object of the newly allocated class, nullptr to
  /// default to the Object prototype.
  Value *emitTypedClassAllocation(flow::ClassType *classType, Value *parent);

  /// Return the default init value for the specified type.
  Value *getDefaultInitValue(flow::Type *type);

  /// Convert a Flow type into an IR type.
  static Type flowTypeToIRType(flow::Type *flowType);

  /// @}

 private:
  /// @name expressions
  /// @{

  /// Optionally emit a checked cast if the IR value type does not match the
  /// Flow type of the specified expression.
  /// This is called automatically by genExpression() but may need to be used
  /// manually in some rare cases.
  Value *enforceExprType(Value *value, ESTree::Node *expr);

  /// Generate IR for the expression \p Expr. Emit a checked cast if the Flow
  /// type of the expression doesn't match the compiled IR type.
  /// \p nameHint is used to provide names for anonymous functions.
  /// We currently provide names for functions that are assigned to a variable,
  /// or functions that are assigned to an object key. These are a subset of
  /// ES6, but not all of it.
  Value *genExpression(ESTree::Node *expr, Identifier nameHint = Identifier{});

  /// A helper called only from \c genExpression. It performs the actual work.
  Value *_genExpressionImpl(ESTree::Node *expr, Identifier nameHint);

  /// Generate an expression and perform a conditional branch depending on
  /// whether it evaluates to true or false (or optionally, nullish).
  /// \param onNullish when not nullptr, the block to branch to if expr
  ///   is nullish (undefined or null). Note that this supersedes `onFalse` when
  ///   provided. If onNullish is nullptr, then `onFalse` will be branched to
  ///   when expr is nullish, by virtue of boolean coercion.
  void genExpressionBranch(
      ESTree::Node *expr,
      BasicBlock *onTrue,
      BasicBlock *onFalse,
      BasicBlock *onNullish);

  /// Generate IR to push \p element onto the end of \p array. If it is a
  /// SpreadElement, append its result to the end of the array.
  void genFastArrayPush(Value *array, ESTree::Node &element);

  /// Convert the given list of elements in \p list into an array, spreading
  /// SpreadElements as needed.
  Value *genFastArrayFromElements(ESTree::NodeList &list);

  /// Convert the given list of elements in \p list into an object with
  /// properties at the element indices given by the tuple type, so PrLoad can
  /// be used to retrieve elements.
  Value *genTupleFromElements(ESTree::NodeList &list);

  /// Convert the \p input into an array, spreading SpreadElements
  /// using for-or iteration semantics.
  /// Allows sharing spread code between genArrayExpr and genCallExpr.
  Value *genArrayFromElements(ESTree::NodeList &list);

  Value *genObjectExpr(ESTree::ObjectExpressionNode *Expr);

  /// Generate IR for the typed object literal \p Expr.
  Value *genTypedObjectExpr(
      ESTree::ObjectExpressionNode *Expr,
      flow::ExactObjectType *type);

  Value *genArrayExpr(ESTree::ArrayExpressionNode *Expr);
  Value *genCallExpr(ESTree::CallExpressionNode *call);
  Value *emitNativeCall(
      ESTree::CallExpressionNode *call,
      flow::NativeFunctionType *natFuncType);

  /// Generates a call expression in an optional chain.
  /// \param shortCircuitBB the block to jump to upon short circuiting,
  ///    when the `?.` operator is used. If null, this is the outermost
  ///    optional expression in the chain, and will create its own
  ///    shortCircuitBB.
  Value *genOptionalCallExpr(
      ESTree::OptionalCallExpressionNode *call,
      BasicBlock *shortCircuitBB);

  /// Generates the $SHBuiltin compiler intrinsics.
  Value *genSHBuiltin(
      ESTree::CallExpressionNode *call,
      ESTree::IdentifierNode *builtin);

  /// $SHBuiltin.call(fn, thisVal, arg1, arg2, ...)
  /// Calls \c fn with \c thisVal and the specified args.
  Value *genSHBuiltinCall(ESTree::CallExpressionNode *call);

  /// $SHBuiltin.externC({}, function fopen(path: c_ptr, mode: c_ptr): c_ptr)
  /// Import an external C function.
  Value *genSHBuiltinExternC(ESTree::CallExpressionNode *call);

  /// $SHBuiltin.moduleFactory(modId, factoryFunction)
  /// Process \p factoryFunction as a module initialization function,
  /// for the module with id \p modId.
  Value *genSHBuiltinModuleFactory(ESTree::CallExpressionNode *call);

  /// $SHBuiltin.export(modId, importNameStr, importExp)
  /// For now, this just evaluates to the third argument of \p call.
  Value *genSHBuiltinImport(ESTree::CallExpressionNode *call);

  /// Emits the actual call for \p call, and is used as a helper function for
  /// genCallExpr and genOptionalCallExpr.
  /// \param newTarget the new.target, undefined if not a constructor call.
  /// \return the result value of the call.
  Value *emitCall(
      ESTree::CallExpressionLikeNode *call,
      Value *callee,
      Value *target,
      bool calleeIsAlwaysClosure,
      Value *thisVal,
      Value *newTarget);

  struct MemberExpressionResult {
    /// Value of the looked up property.
    Value *result;
    /// The IR Function if the result is known to be a specific function.
    Function *resultFn;
    /// Object the property is being looked up on.
    Value *base;
  };

  enum class MemberExpressionOperation {
    Load,
    Delete,
  };

  /// Generate IR for a member expression.
  /// \param op the operation to perform on the property if it exists.
  ///    If Load, the result is set to the value of the property.
  ///    If Delete, the result is the result of DeletePropertyInst.
  /// \return both the result and the base object.
  MemberExpressionResult genMemberExpression(
      ESTree::MemberExpressionNode *Mem,
      MemberExpressionOperation op);

  /// Try to perform a typed member access, if the information is available.
  MemberExpressionResult emitMemberLoad(
      ESTree::MemberExpressionNode *mem,
      Value *baseValue,
      Value *propValue);
  /// \param thisValue is the value of `this` when the member expression was
  /// first evaluated. This is only populated in `super` references.
  void emitMemberStore(
      ESTree::MemberExpressionNode *mem,
      Value *storedValue,
      Value *baseValue,
      Value *propValue,
      Value *thisValue);

  /// emitMemberStore for the specific case where the member is a typed class
  /// field.
  /// \param classType the class containing the field
  /// \param prop the name of the field, assumed to be an IdentifierNode.
  /// \param object to store into
  /// \param value to store.
  void emitTypedFieldStore(
      flow::ClassType *classType,
      ESTree::Node *prop,
      Value *object,
      Value *value);

  /// Load a member property from a super.property expression inside a typed
  /// class method.
  MemberExpressionResult emitTypedSuperLoad(
      ESTree::SuperNode *superNode,
      ESTree::IdentifierNode *property);

  /// \return an instruction that is the currently visible scope described by
  /// \p targetVarScope, emitting a ResolveScopeInst if necessary.
  Instruction *emitResolveScopeInstIfNeeded(VariableScope *targetVarScope);

  /// Generate IR for a member expression in the middle of an optional chain.
  /// \param shortCircuitBB the block to jump to upon short circuiting,
  ///    when the `?.` operator is used. If null, this is the outermost
  ///    optional expression in the chain, and will create its own
  ///    shortCircuitBB.
  /// \param op the operation to perform on the property if it exists.
  ///    If Load, the result is set to the value of the property.
  ///    If Delete, the result is the result of DeletePropertyInst.
  /// \return both the result and the base object.
  MemberExpressionResult genOptionalMemberExpression(
      ESTree::OptionalMemberExpressionNode *mem,
      BasicBlock *shortCircuitBB,
      MemberExpressionOperation op);

  /// Generate IR for a direct call to eval(). This is invoked from
  /// genCallExpr().
  Value *genCallEvalExpr(ESTree::CallExpressionNode *call);

  Value *genNewExpr(ESTree::NewExpressionNode *N);
  Value *genAssignmentExpr(ESTree::AssignmentExpressionNode *AE);

  enum class LogicalAssignmentOp : uint8_t {
    ShortCircuitOrKind, // ||=
    ShortCircuitAndKind, // &&=
    NullishCoalesceKind, // ??=
  };
  Value *genLogicalAssignmentExpr(
      ESTree::AssignmentExpressionNode *AE,
      LogicalAssignmentOp assignmentKind,
      LReference lref,
      Identifier nameHint);
  Value *genConditionalExpr(ESTree::ConditionalExpressionNode *C);
  Value *genSequenceExpr(ESTree::SequenceExpressionNode *Sq);
  Value *genYieldExpr(ESTree::YieldExpressionNode *Y);
  Value *genYieldStarExpr(ESTree::YieldExpressionNode *Y);
  Value *genAwaitExpr(ESTree::AwaitExpressionNode *A);
  Value *genBinaryExpression(ESTree::BinaryExpressionNode *bin);
  Value *genUnaryExpression(ESTree::UnaryExpressionNode *U);
  Value *genUpdateExpr(ESTree::UpdateExpressionNode *updateExpr);
  Value *genLogicalExpression(ESTree::LogicalExpressionNode *logical);
  Value *genThisExpression();

  /// A helper function to unify the largely same IRGen logic of \c genYieldExpr
  /// and \c genAwaitExpr.
  /// \param value the value operand of the will-generate SaveAndYieldInst.
  Value *genYieldOrAwaitExpr(Value *value);

  /// Generate IR for the logical expression \p logical and jump to the
  /// corresponding label depending on its value.
  void genLogicalExpressionBranch(
      ESTree::LogicalExpressionNode *logical,
      BasicBlock *onTrue,
      BasicBlock *onFalse,
      BasicBlock *onNullish);

  /// Generate IR for the identifier expression.
  /// We need to know whether it's after typeof (\p afterTypeOf),
  /// to help decide whether a load can throw.
  Value *genIdentifierExpression(
      ESTree::IdentifierNode *Iden,
      bool afterTypeOf);

  Value *genMetaProperty(ESTree::MetaPropertyNode *MP);

  /// Generate the new.target of the current function.
  Value *genNewTarget();

  /// Generate IR for a template literal expression, which in most cases is
  /// translated to a call to HermesInternal.concat().
  Value *genTemplateLiteralExpr(ESTree::TemplateLiteralNode *Expr);

  /// Generate IR for a tagged template expression, which involves converting
  /// a template literal to a template object, then invoking the tag function
  /// with the template object and substitution expressions.
  Value *genTaggedTemplateExpr(ESTree::TaggedTemplateExpressionNode *Expr);

  /// Whether or not a finally clause is needed in \c genResumeGenerator.
  enum class GenFinally { No, Yes };

  /// Generate IR for handling resuming a suspended generator.
  /// \param genFinally whether or not a finally clause need to be generated.
  ///   It's true when there is a corresponding yield/await to resume from. It's
  ///   not if this is the first resume and there is no yield/await.
  /// \param isReturn the slot indicating whether the user requested a return.
  /// \param nextBB the next BasicBlock to run if the user did not request a
  ///   return.
  /// \param received if non-null, the stack location at which to store the
  ///   value received from the user as the arg to next(), throw(), or return().
  Value *genResumeGenerator(
      GenFinally genFinally,
      AllocStackInst *isReturn,
      BasicBlock *nextBB,
      AllocStackInst *received = nullptr);

  Value *genRegExpLiteral(ESTree::RegExpLiteralNode *RE);

  /// @}

 private:
  /// @name Exception handling.
  /// @{

  /// Generate IR for try/catch/finally statement.
  void genTryStatement(ESTree::TryStatementNode *tryStmt);

  /// Emit the scaffolding for a try/catch block.
  /// \param nextBlock is the block to branch to after the body and the
  ///     finalizer. If nullptr, a new block is allocated. At the end the
  ///     insertion point is set to the that block.
  /// \param emitBody updates \c surroundingTry and emits the body of the block
  ///     guarded by the \c try. Exceptions occurring in the body will be caught
  ///     by the handler.
  /// \param emitNormalCleanup emits the cleanup after exiting the try body
  ///     normally (neither through an exception nor break/return/continue). It
  ///     executes outside the exception scope (exceptions here will not be
  ///     caught by the handler).
  /// \param emitHandler emits the code to execute when
  ///     the exception is caught. It must emit a CatchInst to clear the active
  ///     exception, even if it ignores it. \p nextBlock is passed as a
  ///     parameter.
  /// \return the block used as \c nextBlock
  template <typename EB, typename EF, typename EH>
  BasicBlock *emitTryCatchScaffolding(
      BasicBlock *nextBlock,
      EB emitBody,
      EF emitNormalCleanup,
      EH emitHandler);

  /// When we see a control change such as return, break, continue,
  /// we need to make sure to generate code for finally block if
  /// we are under a try/catch.
  /// \param sourceTry  the try statement surrounding the AST node performing
  ///   the goto (break, continue, return)
  /// \param targetTry  the try statement surrounding the AST node of the label.
  /// \param cfc indicates whether this is "continue" or "break".
  /// \param continueTarget when cfc == Continue, the target of the branch of
  /// the continue.
  void genFinallyBeforeControlChange(
      SurroundingTry *sourceTry,
      SurroundingTry *targetTry,
      ControlFlowChange cfc,
      BasicBlock *continueTarget = nullptr);

  /// @}

 private:
  /// @name functions
  /// @{

  /// Generate IR for function declarations;
  void genFunctionDeclaration(ESTree::FunctionDeclarationNode *func);

  /// Generate IR for FunctionExpression.
  /// \param superClassNode the current superclass, used for generating method
  ///   code. Needed because we need to store this information in
  ///   FunctionContext due to the enqueueCompilation mechanism.
  /// \param functionKind the kind of the function being created.
  ///   ES5Function by default, but may also be ES6Constructor.
  /// \param homeObject will be set as the homeObject in the CapturedState of
  /// the function \p FE we are going to generate.
  /// \param parentNode can optionally be set to the parent AST node of the
  ///  function node.
  Value *genFunctionExpression(
      ESTree::FunctionExpressionNode *FE,
      Identifier nameHint,
      ESTree::Node *superClassNode = nullptr,
      Function::DefinitionKind functionKind =
          Function::DefinitionKind::ES5Function,
      Variable *homeObject = nullptr,
      ESTree::Node *parentNode = nullptr);

  /// Generate IR for ArrowFunctionExpression.
  /// \param parentScope is the VariableScope of the enclosing function, or null
  ///   if there isn't one.
  /// \param parentNode can optionally be set to the parent AST node of the
  ///  function node.
  Value *genArrowFunctionExpression(
      ESTree::ArrowFunctionExpressionNode *AF,
      Identifier nameHint,
      ESTree::Node *parentNode = nullptr);

  /// Generate IR for a function which inherits the current captured state
  /// (this, arguments etc.), which may be lazy.
  //// \param originalName is the original non-unique name specified by the user
  ///   or inferred according to the rules of ES6.
  /// \param functionNode is the ESTree arrow function node.
  /// \param parentScope is the VariableScope of the enclosing function, or null
  ///   if there isn't one.
  /// \param parentNode can optionally be set to the parent AST node of the
  ///  function node.
  /// \param capturedState the relevant captured state.
  NormalFunction *genCapturingFunction(
      Identifier originalName,
      ESTree::FunctionLikeNode *functionNode,
      VariableScope *parentScope,
      const CapturedState &capturedState,
      Function::DefinitionKind functionKind,
      ESTree::Node *parentNode = nullptr);

  /// Generate IR for a function (function declaration or expression) that is
  /// not a generator, async, or arrow function.  Constructors are "basic",
  /// under this definition. The body may optionally be lazy.
  ///
  /// \param originalName is the original non-unique name specified by the user
  ///   or inferred according to the rules of ES6.
  /// \param functionNode is the ESTree function node (declaration, expression,
  ///   object method).
  /// \param parentScope is the VariableScope of the enclosing function, or null
  ///   if there isn't one.
  /// \param superClassNode is the extends clause of the current class, if
  ///   available. Used for super calls.
  /// \param functionKind is the proper kind for this function.
  /// \param ctorClass is the class containing functionNode, if functionNode
  ///  is for a constructor.
  /// \param homeObject will be set as the homeObject in the CapturedState of
  /// the function \p FE we are going to generate.
  /// \param parentNode can optionally be set to the parent AST node of the
  ///  function node.
  /// \returns a new Function.
  NormalFunction *genBasicFunction(
      Identifier originalName,
      ESTree::FunctionLikeNode *functionNode,
      VariableScope *parentScope,
      ESTree::Node *superClassNode = nullptr,
      Function::DefinitionKind functionKind =
          Function::DefinitionKind::ES5Function,
      Variable *homeObject = nullptr,
      ESTree::Node *parentNode = nullptr);

  /// Generate the IR for two functions: an outer GeneratorFunction and an inner
  /// GeneratorInnerFunction. The outer function runs CreateGenerator on the
  /// inner function and returns the result.
  /// \param originalName is the original non-unique name specified by the user
  ///   or inferred according to the rules of ES6.
  /// \param functionNode is the ESTree function node (declaration, expression,
  ///   object method).
  /// \param homeObject will be set as the homeObject in the CapturedState of
  /// the function \p FE we are going to generate.
  /// \param parentNode can optionally be set to the parent AST node of the
  ///  function node.
  /// \return the outer Function.
  Function *genGeneratorFunction(
      Identifier originalName,
      ESTree::FunctionLikeNode *functionNode,
      VariableScope *parentScope,
      Variable *homeObject = nullptr,
      ESTree::Node *parentNode = nullptr);

  /// Generate the IR for an async function: it desugars async function to a
  /// generator function wrapped in a call to the JS builtin `spawnAsync` and
  /// return the result. To visualize it,
  ///   async function<name>(<args>)<body>
  /// is effectively lowered to
  ///   function<name>(){return spawnAsync(function*()<body>, this, arguments)}
  /// \param originalName is the original non-unique name specified by the user
  ///   or inferred according to the rules of ES6.
  /// \param functionNode is the ESTree function node (declaration, expression,
  ///   object method).
  /// \param capturedState the relevant captured state.
  /// \param parentNode can optionally be set to the parent AST node of the
  ///  function node.
  /// \return the async Function.
  Function *genAsyncFunction(
      Identifier originalName,
      ESTree::FunctionLikeNode *functionNode,
      VariableScope *parentScope,
      const CapturedState &capturedState,
      ESTree::Node *parentNode = nullptr);

  /// In the beginning of an ES5 function, initialize the special captured
  /// variables needed by arrow functions, constructors and methods.
  /// This is used only by \c genES5Function() and the global scope.
  /// This is called internally by emitFunctionPrologue().
  void initCaptureStateInES5FunctionHelper();

  enum class InitES5CaptureState { No, Yes };

  enum class DoEmitDeclarations { No, Yes };

  /// Emit the function prologue for the current function, consisting of the
  /// following things:
  /// - a next block, so we can append instructions to it to generate the actual
  ///   code for the function
  /// - declare all hoisted es5 variables and global properties
  /// - initialize all hoisted es5 variables to undefined
  /// - declare all hoisted es5 functions and initialize them (recursively
  ///   generating their functions)
  /// - create "this" parameter
  /// - create all explicit parameters and store them in variables
  /// \param entry the unpopulated entry block for the function
  /// \param doInitES5CaptureState initialize the capture state for ES5
  ///     functions.
  /// \param doEmitDeclarations run code to initialize declarations in the
  /// function.
  ///     When "No", only set the .length of the resultant function.
  ///     Used for the outer function of generator functions, e.g.
  /// \param parentScope the VariableScope of the enclosing function, or null if
  /// emitting the top level function.
  void emitFunctionPrologue(
      ESTree::FunctionLikeNode *funcNode,
      BasicBlock *entry,
      InitES5CaptureState doInitES5CaptureState,
      DoEmitDeclarations doEmitDeclarations,
      VariableScope *parentScope);

  /// Declare all variables in the scope, except parameters (which are handled
  /// separately). Variables that obey TDZ are initialized to empty.
  /// Hoisted closures are recursively compiled and initialized. Imports are
  /// code generated.
  /// \param scope The lexical scope, nullabe. If null, there is no scope, so do
  ///     nothing.
  void emitScopeDeclarations(sema::LexicalScope *scope);

  /// Emit any lazy declarations in the global scope which haven't been emitted
  /// yet because they were found as the result of lazy compilation.
  /// \pre all undeclared declarations are at the end of the scope's list.
  void emitLazyGlobalDeclarations(sema::LexicalScope *globalScope);

  /// Emit the function declaration for a hoisted function.
  /// Eliminate dead stores of undefined when necessary.
  void emitHoistedFunctionDeclaration(
      sema::LexicalScope *scope,
      ESTree::FunctionDeclarationNode *funcDecl);

  /// Emit the loading and initialization of parameters in the function
  /// prologue.
  void emitParameters(ESTree::FunctionLikeNode *funcNode);

  /// Count number of expected arguments, including "this".
  /// Only parameters up to the rest parameter or the first parameter with an
  /// initializer are counted.
  /// See ES2025 15.1.5 for details.
  uint32_t countExpectedArgumentsIncludingThis(
      ESTree::FunctionLikeNode *funcNode);

  /// Optionally emit a return value and perform cleanup after emission of the
  /// current function is finished. Specifically it attempts to merge the entry
  /// and the next block in order to create less "noise".
  /// \param returnValue if non-nullptr, a return instruction for it is emitted
  ///   with debug location at the end of the source range.
  ///   If nullptr, there is no implicit return and UnreachableInst is emitted
  ///   instead.
  void emitFunctionEpilogue(Value *returnValue);

  /// Generates (if necessary) the field init function for the class
  /// of the current class context.  This function performs any
  /// necessary field initializations for that class.  (This should
  /// only be called by emitCreateFieldInitFunction, below.)
  ///
  /// \return the generated function, or else nullptr if the class has no field
  /// initializers.
  Function *genFieldInitFunction();

  /// If there is a field init function associated with the class of
  /// the current class context, emits the instruction for creating
  /// the closure for that function, stores the result of that a
  /// variable in the scope containing the class declaration, and
  /// associates the variable with the class type in a table within
  /// the ESTreeIRGen object.
  void emitCreateTypedFieldInitFunction();

  /// Emit a call to the field init function for in \p classType.
  void emitTypedFieldInitCall(flow::ClassType *classType);

  /// Generate a body for a dummy function so that it doesn't crash the
  /// backend when encountered.
  static void genDummyFunction(Function *dummy);

  /// Add a LazyCompilationDataInst stub to \p F.
  /// \param parentNode can optionally be set to the parent AST node of the
  ///  function node.
  void setupLazyFunction(
      Function *F,
      ESTree::FunctionLikeNode *functionNode,
      ESTree::Node *parentNode,
      ESTree::BlockStatementNode *bodyBlock,
      VariableScope *parentVarScope,
      ExtraKey extraKey,
      const CapturedState &capturedState = {});

  /// Invoked after IR for a function has been emitted to perform any needed
  /// adjustments. Specifically, it updates all catch targets.
  void onCompiledFunction(Function *F);

  /// @}

 private:
  inline FunctionContext *curFunction() {
    assert(functionContext_ && "No active function context");
    return functionContext_;
  }

  /// Resolve the identifier node to the corresponding variable or global prop
  /// in expression context.
  Value *resolveIdentifier(ESTree::IdentifierNode *id) {
    return getDeclData(getIDDecl(id));
  }

  /// Resolve the identifier node associated with the promoted function
  /// to the corresponding global variable in expression context.
  Value *resolveIdentifierPromoted(ESTree::IdentifierNode *id) {
    auto *declPromoted = getIDDeclPromoted(id);
    return declPromoted ? getDeclData(declPromoted) : nullptr;
  }

  /// Cast the node to ESTree::IdentifierNode and resolve it to an expression
  /// decl.
  Value *resolveIdentifierFromID(ESTree::Node *id) {
    return resolveIdentifier(llvh::cast<ESTree::IdentifierNode>(id));
  }

  /// Generate the IR for the property field of the MemberExpressionNode.
  /// The property field may be a string literal or some computed expression.
  Value *genMemberExpressionProperty(ESTree::MemberExpressionLikeNode *Mem);

  /// Check whether we know that an LReference to the specified AST node can
  /// be created without any side effects, including throwing. This is not
  /// supposed to perform a sophisticated analysis, just catch the obvious
  /// cases. For now it only recognizes local non-const variables.
  bool canCreateLRefWithoutSideEffects(ESTree::Node *target);

  /// Generates a left hand side reference from valid estree nodes
  /// that can be translated to lref.
  /// \param declInit indicates whether this reference is a declaration
  ///     initialization, in which case the TDZ check on store should be
  ///     omitted.
  LReference createLRef(ESTree::Node *node, bool declInit);

  /// Generate a call to a method of HermesInternal with the specified name \p
  /// name.
  Value *genHermesInternalCall(
      llvh::StringRef name,
      Value *thisValue,
      ArrayRef<Value *> args);

  /// Generate a builtin call.
  Value *genBuiltinCall(
      BuiltinMethod::Enum builtinIndex,
      ArrayRef<Value *> args);

  /// Generate code to ensure that \p value is an object and it it isn't, throw
  /// a type error with the specified message.
  void emitEnsureObject(Value *value, llvh::StringRef message);

  /// \return the internal value @@iterator
  Value *emitIteratorSymbol();

  /// IteratorRecord as defined in ES2018 7.4.1 GetIterator
  struct IteratorRecordSlow {
    Value *iterator;
    Value *nextMethod;
  };

  /// ES2018 7.4.1 GetIterator
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-getiterator
  ///
  /// Call obj[@@iterator], which should return an iterator, and return the
  /// iterator itself and its \c next() method.
  ///
  /// NOTE: This API is slow and should only be used if it is necessary to
  /// provide a value to the `next()` method on the iterator.
  ///
  /// \return (iterator, nextMethod)
  IteratorRecordSlow emitGetIteratorSlow(Value *obj);

  /// ES2018 7.4.2 IteratorNext
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-iteratornext
  ///
  /// Call the next method and throw an exception if the result is not an
  /// object.
  ///
  /// \return the result object.
  Value *emitIteratorNextSlow(IteratorRecordSlow iteratorRecord);

  /// ES2018 7.4.3 IteratorComplete
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-iteratorcomplete
  ///
  /// \return \c iterResult.done
  Value *emitIteratorCompleteSlow(Value *iterResult);

  /// ES2018 7.4.4 IteratorValue
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-iteratorvalue
  ///
  /// \return \c iterResult.value
  Value *emitIteratorValueSlow(Value *iterResult);

  /// ES2018 7.4.6 IteratorClose
  /// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-iteratorclose
  ///
  /// \param ignoreInnerException if set, exceptions thrown by the \c
  ///     iterator.return() method will be ignored and its result will not be
  ///     checked whether it is an object.
  void emitIteratorCloseSlow(
      IteratorRecordSlow iteratorRecord,
      bool ignoreInnerException);

  /// Used as the stack storages needed for fast iteration.
  struct IteratorRecord {
    /// Storage for the iterator or the index.
    /// Set to undefined if iteration is complete.
    AllocStackInst *iterStorage;
    /// If this is an array being iterated by index, stores the source.
    /// Otherwise, stores the `iterator.next` method.
    AllocStackInst *sourceOrNext;
  };

  /// Emit the IteratorBeginInst for \p obj, which allows us to iterate arrays
  /// efficiently.
  IteratorRecord emitGetIterator(Value *obj);

  /// Emit the IteratorNextInst for \p iteratorRecord, which may alter the
  /// iteratorRecord to indicate completion.
  Value *emitIteratorNext(IteratorRecord iteratorRecord) {
    auto *LSI = Builder.createLoadStackInst(iteratorRecord.sourceOrNext);
    return Builder.createIteratorNextInst(iteratorRecord.iterStorage, LSI);
  }

  /// If iteratorRecord.iterStorage is undefined, then iteration is complete
  /// and the user of the iterator may want to branch to an exit code path.
  /// \return iteratorRecord.iterStorage === undefined
  Value *emitIteratorComplete(IteratorRecord iteratorRecord) {
    return Builder.createBinaryOperatorInst(
        Builder.createLoadStackInst(iteratorRecord.iterStorage),
        Builder.getLiteralUndefined(),
        ValueKind::BinaryStrictlyEqualInstKind);
  }

  /// Emit the IteratorCloseInst, which will close the iterator if it was
  /// opened by IteratorBeginInst.
  /// \param ignoreInnerException ignore any exceptions thrown by `.return()`.
  Value *emitIteratorClose(
      IteratorRecord iteratorRecord,
      bool ignoreInnerException) {
    auto *LSI = Builder.createLoadStackInst(iteratorRecord.iterStorage);
    return Builder.createIteratorCloseInst(LSI, ignoreInnerException);
  }

  /// Generate code for destructuring assignment to ArrayPattern or
  /// ObjectPattern.
  void emitDestructuringAssignment(
      bool declInit,
      ESTree::PatternNode *target,
      Value *source);

  /// Generate code for destructuring assignment to ArrayPattern.
  void emitDestructuringArray(
      bool declInit,
      ESTree::ArrayPatternNode *targetPat,
      Value *source);

  /// Generate code for destructuring assignment to ArrayPattern from a typed
  /// tuple.
  void emitDestructuringTypedTuple(
      bool declInit,
      ESTree::ArrayPatternNode *targetPat,
      flow::TupleType *type,
      Value *source);

  /// A record used by to describe a shared exception handler. Every individual
  /// stores the caught exception in the specified location and branches to the
  /// specified block.
  struct SharedExceptionHandler {
    /// Stack location to store the exception if it occurs.
    AllocStackInst *exc = nullptr;
    /// Branch to this block if an exception occurs.
    BasicBlock *exceptionBlock = nullptr;
    /// Set to true if at least one try/catch block was needed and emitted.
    bool emittedTry = false;
  };

  void emitRestElement(
      bool declInit,
      ESTree::RestElementNode *rest,
      IteratorRecord iteratorRecord,
      AllocStackInst *iteratorDone,
      SharedExceptionHandler *handler);

  /// Emit an operation guarded by try with a shared exception handler.
  template <typename EB>
  void emitTryWithSharedHandler(SharedExceptionHandler *handler, EB emitBody);

  /// Generate code for destructuring assignment to ObjectPattern.
  void emitDestructuringObject(
      bool declInit,
      ESTree::ObjectPatternNode *target,
      Value *source);

  /// Generate code for assigning to the "rest" property in an object
  /// destructuring pattern. \p excludedItems is a list of the keys that have
  /// been destructured so far, so they can be excluded from the rest property.
  void emitRestProperty(
      bool declInit,
      ESTree::RestElementNode *rest,
      const llvh::SmallVectorImpl<Value *> &excludedItems,
      Value *source);

  /// If the initializer \p init is nullptr, just return \p value.
  /// Otherwise emit code to check whether \p value equals \c undefined, and
  /// evaluate and return the initializer in that case.
  /// \param nameHint used to provide names for anonymous functions.
  Value *emitOptionalInitialization(
      Value *value,
      ESTree::Node *init,
      Identifier nameHint);

  /// Emit an instruction to load a value from a specified location.
  /// \param from location to load from, either a Variable or
  ///     GlobalObjectProperty.
  /// \param inhibitThrow  if true, do not throw when loading from missing
  ///     global properties.
  /// \return the instruction performing the load.
  Instruction *emitLoad(Value *from, bool inhibitThrow);

  /// Emit an instruction to a store a value into the specified location.
  /// \param storedValue value to store
  /// \param ptr location to store into, either a Variable or
  ///     GlobalObjectProperty.
  /// \param declInit whether this is a declaration initializer, so the TDZ
  /// check
  ///     should be skipped.
  /// \return the instruction performing the store.
  Instruction *emitStore(Value *storedValue, Value *ptr, bool declInit);

 private:
  /// Search for the specified AST node in \c compiledEntities_ and return the
  /// associated IR value, or nullptr if not found.
  Value *findCompiledEntity(
      ESTree::Node *node,
      ExtraKey extraKey = ExtraKey::Normal) {
    auto compiledIt =
        compiledEntities_.find(CompiledMapKey(node, (unsigned)extraKey));
    return compiledIt != compiledEntities_.end() ? compiledIt->second : nullptr;
  }

  /// Associate the forward declared value \p value with the specified AST node
  /// \p node, and enque the compilation worker that will eventually compile the
  /// body of the specified AST value.
  ///
  /// \param node the AST node that will be compiled
  /// \param extraKey an "additional" key to distinguish between different
  ///     compilation callabacks that may be associated with the same AST node.
  /// \param value the forward declared Value that will be compiled (a Function)
  /// \param f the callback that will compile the body of the AST when  invoked.
  template <typename F>
  void enqueueCompilation(
      ESTree::Node *node,
      ExtraKey extraKey,
      Value *value,
      F &&f) {
    CompiledMapKey key(node, (unsigned)extraKey);
    assert(compiledEntities_.count(key) == 0 && "Overwriting compiled entity");
    compiledEntities_[key] = value;
    compilationQueue_.emplace_back(f);
  }

  /// Run all tasks in the compilation queue until it is empty.
  void drainCompilationQueue();

  /// Return the non-null expression sema::Decl associated with the identifier.
  sema::Decl *getIDDecl(ESTree::IdentifierNode *id) {
    assert(id && "IdentifierNode cannot be null");
    sema::Decl *decl = semCtx_.getExpressionDecl(id);
    assert(decl && "identifier must be resolved");
    return decl;
  }

  /// Return the global Decl associated with the promoted function identifier.
  sema::Decl *getIDDeclPromoted(ESTree::IdentifierNode *id) {
    assert(id && "IdentifierNode cannot be null");
    sema::Decl *decl = semCtx_.getPromotedDecl(id);
    return decl;
  }

  /// Set the customData field of the declaration with the specified value.
  static void setDeclData(sema::Decl *decl, Value *value) {
    assert(decl);
    decl->customData = value;
  }
  /// Extract the decl's custom data field, which must be a value.
  static Value *getDeclData(sema::Decl *decl) {
    assert(decl);
    assert(decl->customData);
    return static_cast<Value *>(decl->customData);
  }
};

template <typename EB, typename EF, typename EH>
BasicBlock *ESTreeIRGen::emitTryCatchScaffolding(
    BasicBlock *nextBlock,
    EB emitBody,
    EF emitNormalCleanup,
    EH emitHandler) {
  auto *function = Builder.getFunction();

  auto *catchBlock = Builder.createBasicBlock(function);
  if (!nextBlock)
    nextBlock = Builder.createBasicBlock(function);
  auto *tryBodyBlock = Builder.createBasicBlock(function);

  // Start with a TryStartInst, and transition to try body.
  Builder.createTryStartInst(tryBodyBlock, catchBlock);
  Builder.setInsertionBlock(tryBodyBlock);

  // Generate IR for the body of Try
  emitBody(catchBlock);

  // Emit TryEnd.
  auto *cleanupBlock = Builder.createBasicBlock(function);
  Builder.createTryEndInst(catchBlock, cleanupBlock);
  Builder.setInsertionBlock(cleanupBlock);

  emitNormalCleanup();

  Builder.createBranchInst(nextBlock);

  // Generate the catch/finally block.
  Builder.setInsertionBlock(catchBlock);

  emitHandler(nextBlock);

  return nextBlock;
}

template <typename EB>
void ESTreeIRGen::emitTryWithSharedHandler(
    hermes::irgen::ESTreeIRGen::SharedExceptionHandler *handler,
    EB emitBody) {
  emitTryCatchScaffolding(
      nullptr,
      // emitBody.
      emitBody,
      // emitNormalCleanup.
      []() {},
      // emitHandler.
      [this, handler](BasicBlock *nextBlock) {
        Builder.createStoreStackInst(Builder.createCatchInst(), handler->exc);
        Builder.createBranchInst(handler->exceptionBlock);
        Builder.setInsertionBlock(nextBlock);
      });
  handler->emittedTry = true;
}

} // namespace irgen
} // namespace hermes

#endif // HERMES_IRGEN_ESTREEIRGEN_H
