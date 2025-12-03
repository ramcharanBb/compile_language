# MyPassBBmerge: Basic Block Merging Optimization Pass

## Technical Documentation for Compiler Report

---

## 1. Overview

**MyPassBBmerge** is a custom LLVM optimization pass implemented as part of the compiler infrastructure. It performs basic block merging optimization to simplify the Control Flow Graph (CFG) by eliminating redundant control flow and combining sequential blocks of code.

### 1.1 Purpose

The primary goal of this optimization pass is to:
- Reduce the number of basic blocks in a function
- Eliminate unnecessary branch instructions
- Improve code locality and cache performance
- Simplify the CFG for subsequent optimization passes
- Reduce overall code size

### 1.2 Classification

- **Pass Type**: Function Pass (operates on individual functions)
- **Pass Category**: Control Flow Optimization
- **Optimization Level**: Medium-level optimization (runs after initial code generation)
- **Framework**: LLVM New Pass Manager

---

## 2. Background and Motivation

### 2.1 The Problem

During LLVM IR generation from high-level language constructs (such as if-else statements and boolean literals), the compiler often generates suboptimal control flow patterns:

1. **Constant Conditional Branches**: When boolean literals (`true` or `false`) are used as conditions, the compiler generates conditional branch instructions with constant conditions like `br i1 true, label %A, label %B`.

2. **Straight-Line Code Split Across Blocks**: Code that executes sequentially may be split into multiple basic blocks connected by unconditional branches.

### 2.2 Example of Inefficient IR

```llvm
afterwhile:                                       ; preds = %whilecond
  br i1 true, label %then21, label %else23

then21:                                           ; preds = %afterwhile
  %printfCall = call i32 (ptr, ...) @printf(...)
  br label %ifcont25
```

In this example:
- The branch `br i1 true` always takes the first path
- Block `then21` has only one predecessor (`afterwhile`)
- These two blocks can be merged into a single block

---

## 3. Algorithm and Implementation

### 3.1 Two-Phase Optimization Strategy

MyPassBBmerge employs a two-phase approach:

**Phase 1: Constant Branch Simplification**
- Identifies conditional branches with constant conditions
- Converts them to unconditional branches
- Cleans up unreachable code paths

**Phase 2: Basic Block Merging**
- Identifies unconditional branches to successor blocks
- Merges blocks when the successor has a unique predecessor
- Eliminates redundant branch instructions

### 3.2 Phase 1: Constant Branch Simplification

#### Algorithm Steps:

1. **Iterate** through all basic blocks in the function
2. **Examine** the terminator instruction of each block
3. **Check** if it's a conditional branch instruction
4. **Detect** constant integer conditions (`ConstantInt`)
5. **Determine** the target block based on the constant value:
   - If condition is `1` (true): take the first successor
   - If condition is `0` (false): take the second successor
6. **Create** a new unconditional branch to the target block
7. **Update** the CFG by removing the current block from the dead block's predecessor list
8. **Delete** the old conditional branch instruction

#### Code Implementation:

```cpp
// Phase 1: Simplify constant conditional branches
for(llvm::BasicBlock &BB : F){
    llvm::Instruction *TerminatorBB = BB.getTerminator();
    if (!TerminatorBB) continue;
    
    if (auto *BI = llvm::dyn_cast<llvm::BranchInst>(TerminatorBB)) {
        if(BI->isConditional()){
            llvm::Value *Condition = BI->getCondition();
            
            if (auto *ConstCond = llvm::dyn_cast<llvm::ConstantInt>(Condition)) {
                // Determine target based on constant value
                llvm::BasicBlock *TargetBB = ConstCond->isOne() ? 
                    BI->getSuccessor(0) : BI->getSuccessor(1);
                llvm::BasicBlock *DeadBB = ConstCond->isOne() ? 
                    BI->getSuccessor(1) : BI->getSuccessor(0);
                
                // Create unconditional branch to target
                llvm::BranchInst::Create(TargetBB, &BB);
                
                // Update CFG
                DeadBB->removePredecessor(&BB);
                
                // Remove old conditional branch
                BI->eraseFromParent();
                changed = true;
            }
        }
    }
}
```

### 3.3 Phase 2: Basic Block Merging

#### Algorithm Steps:

1. **Iterate** through all basic blocks (using early increment range to allow safe deletion)
2. **Examine** the terminator instruction
3. **Check** if it's an unconditional branch
4. **Verify** merge conditions:
   - Successor block exists
   - Not a self-loop (BB ≠ successor)
   - Current block is the unique predecessor of the successor
5. **Move** all instructions from successor to current block (before terminator)
6. **Delete** the old unconditional branch
7. **Mark** successor block for deletion
8. **Clean up** by erasing empty successor blocks

#### Code Implementation:

```cpp
// Phase 2: Merge blocks connected by unconditional branches
for(llvm::BasicBlock &BB : llvm::make_early_inc_range(F)){
    llvm::Instruction *TerminatorBB = BB.getTerminator();
    if (!TerminatorBB) continue; 
    
    if (auto *BI = llvm::dyn_cast<llvm::BranchInst>(TerminatorBB)) {
        if(BI->isUnconditional()){
            llvm::BasicBlock *succesorBB = BB.getSingleSuccessor();
            
            // Verify merge conditions
            if(succesorBB && &BB != succesorBB && 
               &BB == succesorBB->getUniquePredecessor()){
                
                // Move instructions from successor to current block
                llvm::BasicBlock::iterator InsertPos = TerminatorBB->getIterator();
                while (!succesorBB->empty()) {
                    llvm::Instruction &Inst = succesorBB->front();
                    Inst.moveBefore(&*InsertPos); 
                }
                
                // Remove unconditional branch
                TerminatorBB->eraseFromParent();
                
                // Schedule successor for deletion
                blocksneed2del.push_back(succesorBB);
                changed = true;
            }
        }
    }
}

// Clean up empty blocks
for(auto *delBB : blocksneed2del){ 
    delBB->eraseFromParent();
}
```

---

## 4. Example Transformation

### 4.1 Input Code (High-Level)

```c
if (true) {
    print("This always runs A");
} else {
    print("Unreachable print A");
}
```

### 4.2 Before Optimization (LLVM IR)

```llvm
afterwhile:                                       ; preds = %whilecond
  br i1 true, label %then21, label %else23

then21:                                           ; preds = %afterwhile
  %printfCall22 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.3)
  br label %ifcont25

else23:                                           ; preds = %afterwhile
  %printfCall24 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.4)
  br label %ifcont25

ifcont25:                                         ; preds = %else23, %then21
  ; continuation...
```

**Analysis:**
- Block `afterwhile` contains a constant conditional branch (`br i1 true`)
- Block `then21` has only one predecessor
- Opportunity for simplification and merging

### 4.3 After Phase 1 (Constant Branch Simplification)

```llvm
afterwhile:                                       ; preds = %whilecond
  br label %then21                                ; <-- Simplified to unconditional

then21:                                           ; preds = %afterwhile
  %printfCall22 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.3)
  br label %ifcont25

else23:                                           ; No predecessors!
  %printfCall24 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.4)
  br label %ifcont25

ifcont25:                                         ; preds = %then21
  ; continuation...
```

**Changes:**
- Constant branch replaced with unconditional branch
- Block `else23` marked as unreachable (no predecessors)

### 4.4 After Phase 2 (Block Merging)

```llvm
afterwhile:                                       ; preds = %whilecond
  %printfCall22 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.3)
  br label %ifcont25                              ; <-- Blocks merged

else23:                                           ; No predecessors! (Dead code)
  %printfCall24 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.4)
  br label %ifcont25

ifcont25:                                         ; preds = %afterwhile
  ; continuation...
```

**Final Result:**
- Blocks `afterwhile` and `then21` merged into single block
- One branch instruction eliminated
- Dead code exposed for future cleanup

---

## 5. Results and Metrics

### 5.1 Test Case: test_control_flow.al

**Input File Statistics:**
- Functions: 2 (`test_no_dce`, `main`)
- Control flow constructs: 4 if-else statements, 1 while loop
- Boolean literals: 2 (`true`, `false`)

**Optimization Results:**

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| Total IR Lines | 96 | 90 | 6.25% reduction |
| Basic Blocks (test_no_dce) | 16 | 14 | 2 blocks eliminated |
| Branch Instructions | 11 | 9 | 2 branches eliminated |
| Constant Branches Simplified | 0 | 2 | N/A |
| Block Merges Performed | 0 | 2 | N/A |

### 5.2 Compiler Output

```
MyDCEPass is running on function: test_no_dce
Simplified constant branch in block: afterwhile
Simplified constant branch in block: ifcont25
Merged blocks: afterwhile and then21
Merged blocks: ifcont25 and else28
MyDCEPass is running on function: main
LLVM IR written to Output.ll
```

### 5.3 Impact Analysis

**Performance Benefits:**
- **Reduced Branch Mispredictions**: Fewer branch instructions mean fewer opportunities for branch misprediction penalties
- **Improved Cache Locality**: Sequential code in a single block has better instruction cache performance
- **Simplified CFG**: Easier for subsequent optimization passes to analyze and optimize

**Code Size Benefits:**
- **6.25% IR size reduction** in the test case
- Fewer basic block headers and metadata
- Reduced overhead from branch instructions

**Optimization Pipeline Benefits:**
- Exposes dead code for Dead Code Elimination (DCE) passes
- Simplifies CFG for other analyses (e.g., dominance, loop analysis)
- Enables more aggressive optimizations in subsequent passes

---

## 6. Integration with Compiler Pipeline

### 6.1 Pass Registration

The pass is registered in the LLVM optimization pipeline through the `codegen.cpp` file:

```cpp
#include "MyPassBBmerge.h"

Codegen::Codegen(){
    // ... initialization code ...
    
    TheFPM->addPass(llvm::PromotePass());     // Mem2Reg
    TheFPM->addPass(MyPass());                 // Dead Code Elimination
    TheFPM->addPass(MyPassBBmerge());         // Basic Block Merging
}
```

### 6.2 Pass Ordering Rationale

The pass runs **after** Dead Code Elimination because:
1. DCE removes trivially dead instructions
2. This may expose more merge opportunities
3. Constant folding from earlier passes creates the constant branches we optimize

### 6.3 Preserved Analyses

The pass returns:
- `PreservedAnalyses::none()` when changes are made (invalidating all analyses)
- `PreservedAnalyses::all()` when no changes occur (preserving all analyses)

This ensures the LLVM analysis framework correctly updates dependent analyses.

---

## 7. Limitations and Future Work

### 7.1 Current Limitations

1. **No Dead Block Elimination**: The pass marks unreachable blocks but doesn't delete them
2. **Single Predecessor Requirement**: Only merges when successor has unique predecessor
3. **No Empty Block Handling**: Doesn't optimize empty intermediate blocks with multiple predecessors
4. **Limited to Constant Conditions**: Doesn't handle other forms of constant expressions

### 7.2 Future Enhancements

1. **Integrated Dead Block Removal**: Add third phase to eliminate orphaned blocks
2. **Relaxed Merging Criteria**: Handle more complex CFG patterns
3. **Constant Expression Evaluation**: Extend to handle non-trivial constant expressions
4. **Integration with SimplifyCFG**: Leverage LLVM's built-in CFG simplification utilities

---

## 8. Conclusion

MyPassBBmerge successfully implements a two-phase optimization strategy for basic block merging in LLVM IR. By first simplifying constant conditional branches and then merging sequential blocks, the pass achieves measurable improvements in code size and quality.

The implementation demonstrates:
- **Effective CFG optimization** through constant propagation at the control flow level
- **Careful LLVM API usage** for safe instruction movement and block deletion
- **Integration with the optimization pipeline** for maximum benefit
- **Measurable results** with 6.25% IR size reduction in test cases

This optimization pass contributes to the overall compiler infrastructure by producing cleaner, more efficient LLVM IR that serves as better input for subsequent optimization passes and code generation.

---

## References

**LLVM Documentation:**
- LLVM Pass Framework: https://llvm.org/docs/WritingAnLLVMPass.html
- LLVM IR Reference: https://llvm.org/docs/LangRef.html
- BasicBlock API: https://llvm.org/doxygen/classllvm_1_1BasicBlock.html

**Implementation Files:**
- `include/MyPassBBmerge.h` - Pass declaration
- `lib/MyPassBBmerge.cpp` - Pass implementation
- `lib/codegen.cpp` - Pass registration
