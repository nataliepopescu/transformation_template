#include "llvm/Pass.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;

namespace {

    struct CAT : public FunctionPass {
        static char ID; 

        CAT() : FunctionPass(ID) {}
    
        // This function is invoked once at the initialization phase of the compiler
        // The LLVM IR of functions isn't ready at this point
        bool doInitialization (Module &M) override {
            //errs() << "Hello LLVM World at \"doInitialization\"\n";
            return false;
        }
    

        /*static bool inspectOneLoop(Loop *L, SmallVectorImpl<Loop *> &Worklist, LoopInfo *LI) {
            bool Changed = false;
        
            SmallVector<BasicBlock*, 8> ExitingBlocks;
            L->getExitingBlocks(ExitingBlocks);
        
            for (BasicBlock *ExitingBlock : ExitingBlocks) {
                if (BranchInst *BI = dyn_cast<BranchInst>(ExitingBlock->getTerminator())) {
                    if (BI->isConditional()) {
                        errs() << "Here is an EXITING block with a conditional TERMINATOR: ";
                        errs().write_escaped(ExitingBlock->getName()) << '\n';
                    }
                }
            }
        
            BasicBlock *LoopLatch = L->getLoopLatch();
            errs() << "Here is a LATCH block: ";
            errs().write_escaped(LoopLatch->getName()) << '\n';
        
            return Changed;
        }

        bool inspectLoops(Loop *L, LoopInfo *LI) {
            bool Changed = false;
        
            SmallVector<Loop *, 4> Worklist;
            Worklist.push_back(L);
        
            for (unsigned Idx = 0; Idx != Worklist.size(); ++Idx) {
                Loop *L2 = Worklist[Idx];
                Worklist.append(L2->begin(), L2->end());
            }
        
            while (!Worklist.empty()) {
                Changed |= inspectOneLoop(Worklist.pop_back_val(), Worklist, LI);
            }
        
            return Changed;
        }

        // This function is invoked once per function compiled
        // The LLVM IR of the input functions is ready and it can be analyzed and/or transformed
        bool runOnFunction (Function &F) override {
            errs() << "Hello LLVM World at \"runOnFunction\"\n";
            bool Changed = false;
            errs() << "In function: ";
            errs().write_escaped(F.getName()) << '\n';
        
            LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        
            for (LoopInfo::iterator I = LI->begin(), E = LI->end(); I != E; ++I) {
                Changed |= inspectLoops(*I, LI);
            }
        
            return Changed;
        }*/

        static StringRef getFunctionName(CallBase *C) {

            Function *F = C->getCalledFunction();

            if (F) {
                return F->getName();
            } else {
                return StringRef("indirect call");
            }

        }

        bool runOnFunction (Function &F) override {

            for (BasicBlock &BB: F) {
                for (Instruction &I: BB) {
                    // Assuming this is the call instruction
                    if (CallBase* C = dyn_cast<CallBase>(&I)) {
                        if (getFunctionName(C).startswith("_ZN4core9panicking18panic_bounds_check")) {
                            // Get the predecessor of this basic block (which is the bounds-checking
                            // basic block) and print its terminator instruction
                            for (BasicBlock* P: predecessors(&BB)) {
                                Instruction *T = P->getTerminator();
                                if (BranchInst *BI = dyn_cast<BranchInst>(T)) {
                                    if (BI->isConditional()) {
                                        Value *V = BI->getCondition();
                                        errs() << "\nFunction: ";
                                        errs().write_escaped(F.getName()) << '\n';
                                        errs() << "Basic block: \n";
                                        errs().write_escaped(P->getName()) << '\n';
                                        errs() << "Terminator: \n";
                                        errs() << *T << '\n';
                                        errs() << "Condition: \n";
                                        errs() << *V << '\n';
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return false;

        }

        // We don't modify the program, so we preserve all analyses.
        // The LLVM IR of functions isn't ready at this point
        void getAnalysisUsage(AnalysisUsage &AU) const override {
            //errs() << "Hello LLVM World at \"getAnalysisUsage\"\n";
            AU.addRequired<LoopInfoWrapperPass>();
            AU.setPreservesAll();
        }
    };
}

// Next there is code to register your pass to "opt"
char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "Template CAT Pass");

// Next there is code to register your pass to "clang"
static CAT * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CAT()); }}); // ** for -O0
